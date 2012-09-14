/// ============================================================================
//
// Copyright (C) 2010, 2011
// Marco Antonio Zanata Alves
//
// GPPD - Parallel and Distributed Processing Group
// Universidade Federal do Rio Grande do Sul
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
/// ============================================================================
#include "../sinuca.hpp"

#ifdef PREFETCHER_DEBUG
    #define PREFETCHER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PREFETCHER_DEBUG_PRINTF(...)
#endif


/// ============================================================================
prefetch_stride_t::prefetch_stride_t() {
    this->set_prefetcher_type(PREFETCHER_STRIDE);

    this->reference_prediction_table_size = 0;
    this->stride_address_distance = 0;  /// double of line size
    this->stride_window = 0;
    this->stride_threshold_activate = 0;
    this->stride_prefetch_degree = 0;
    this->stride_wait_between_requests = 0;
    this->reference_prediction_table = NULL;
};

/// ============================================================================
prefetch_stride_t::~prefetch_stride_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<reference_prediction_line_t>(reference_prediction_table);
};

/// ============================================================================
void prefetch_stride_t::allocate() {
    prefetch_t::allocate();

    this->reference_prediction_table = utils_t::template_allocate_array<reference_prediction_line_t>(this->get_reference_prediction_table_size());
};

/// ============================================================================
void prefetch_stride_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");


    for (uint32_t slot = 0 ; slot < this->reference_prediction_table_size ; slot++) {
        /// Useful Stream, but Long time since last activation
        if (this->reference_prediction_table[slot].relevance_count > 0 &&
        this->reference_prediction_table[slot].cycle_last_activation <= sinuca_engine.get_global_cycle() - this->stride_window) {
            /// FREE the position
            this->reference_prediction_table[slot].clean();
        }

        /// Something interesting to be requested. Check if can do a new request
        else if (this->reference_prediction_table[slot].relevance_count > this->stride_threshold_activate &&
        this->reference_prediction_table[slot].prefetch_ahead < this->stride_prefetch_degree &&
        this->reference_prediction_table[slot].cycle_last_request <= sinuca_engine.get_global_cycle() - this->stride_wait_between_requests) {

            uint64_t last_memory_address = this->reference_prediction_table[slot].last_memory_address +
                                    ( this->reference_prediction_table[slot].prefetch_ahead * this->reference_prediction_table[slot].memory_address_difference);

            uint64_t memory_address = this->reference_prediction_table[slot].last_memory_address +
                                    ( (this->reference_prediction_table[slot].prefetch_ahead + 1) * this->reference_prediction_table[slot].memory_address_difference);

            PREFETCHER_DEBUG_PRINTF("New Prefetch found STRIDE_BUFFER[%u] %s\n", slot, this->reference_prediction_table[slot].content_to_string().c_str());
            if (!this->cmp_index_tag(last_memory_address, memory_address)) {
                int32_t position = this->request_buffer_insert();
                if (position != POSITION_FAIL) {

                    this->reference_prediction_table[slot].prefetch_ahead++;
                    this->reference_prediction_table[slot].cycle_last_request = sinuca_engine.get_global_cycle();

                    /// Statistics
                    this->add_stat_created_prefetches();
                    if (this->reference_prediction_table[slot].memory_address_difference > 0) {
                        this->add_stat_upstride_prefetches();
                    }
                    else {
                        this->add_stat_downstride_prefetches();
                    }

                    uint64_t opcode_address = this->reference_prediction_table[slot].last_opcode_address;

                    this->request_buffer[position].packager(
                                                        0,                                          /// Request Owner
                                                        0,                                          /// Opcode. Number
                                                        opcode_address,                             /// Opcode. Address
                                                        0,                                          /// Uop. Number

                                                        memory_address,                             /// Mem. Address
                                                        sinuca_engine.get_global_line_size(),       /// Block Size

                                                        PACKAGE_STATE_UNTREATED,                    /// Pack. State
                                                        0,                                          /// Ready Cycle

                                                        MEMORY_OPERATION_PREFETCH,                  /// Mem. Operation
                                                        false,                                      /// Is Answer

                                                        0,                                          /// Src ID
                                                        0,                                          /// Dst ID
                                                        NULL,                                       /// *Hops
                                                        0                                           /// Hop Counter
                                                        );
                    PREFETCHER_DEBUG_PRINTF("\t INSERTED on PREFETCHER_BUFFER[%d]\n", position);
                    PREFETCHER_DEBUG_PRINTF("\t %s", this->request_buffer[position].content_to_string().c_str());
                    break;
                }
            }
            else {
                this->reference_prediction_table[slot].prefetch_ahead++;
                this->reference_prediction_table[slot].cycle_last_request = sinuca_engine.get_global_cycle();

                /// Statistics
                this->add_stat_dropped_prefetches();
                PREFETCHER_DEBUG_PRINTF("\t DROPPED PREFETCH\n");
                break;
            }
        }
    }
};

//======================================================================
void prefetch_stride_t::treat_prefetch(memory_package_t *package) {
    uint32_t slot;

    switch (this->get_prefetcher_type()) {
        case PREFETCHER_STRIDE:
            /// Try to match the request with some Stream
            for (slot = 0; slot < this->reference_prediction_table_size; slot++) {
                /// Found one stride with no difference on MemoryAddress - Ignoring
                if (this->reference_prediction_table[slot].last_memory_address == package->memory_address) {
                    PREFETCHER_DEBUG_PRINTF("Prefetcher: Found one stride... No Difference on Address - Ignoring.\n");
                    break;
                }

                /// Compute once the address diference, reused multiple times.
                int64_t address_difference = package->memory_address - this->reference_prediction_table[slot].last_memory_address;

                /// Found one stride with same MemoryAddressDifference - Match
                if ((this->reference_prediction_table[slot].memory_address_difference == address_difference || this->reference_prediction_table[slot].memory_address_difference == 0) &&
                abs(address_difference) <= stride_address_distance) {
                    PREFETCHER_DEBUG_PRINTF("Prefetcher: Found one Stream ... %s\n", address_difference > 0 ? "Increasing" : "Decreasing");
                    this->reference_prediction_table[slot].memory_address_difference = address_difference;
                    this->reference_prediction_table[slot].last_opcode_address = package->opcode_address;
                    this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                    this->reference_prediction_table[slot].relevance_count++;
                    this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();

                    /// Statistics
                    this->add_stat_request_matches();

                    if (this->reference_prediction_table[slot].prefetch_ahead > 0) {
                        this->reference_prediction_table[slot].prefetch_ahead--;
                    }
                    break;
                }
            }

            /// Could not find a STRIDE, Create a new
            if (slot == this->reference_prediction_table_size) {
                for (slot = 0 ; slot < this->reference_prediction_table_size ; slot++) {
                    /// Free slot
                    if (this->reference_prediction_table[slot].relevance_count == 0) {
                        PREFETCHER_DEBUG_PRINTF("Prefetcher: No stride found... Allocating it.\n");
                        this->reference_prediction_table[slot].memory_address_difference = 0;
                        this->reference_prediction_table[slot].first_opcode_address = package->opcode_address;
                        this->reference_prediction_table[slot].last_opcode_address = package->opcode_address;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].relevance_count = 1;
                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->reference_prediction_table[slot].cycle_last_request = sinuca_engine.get_global_cycle();
                        break;
                    }
                }
            }

            /// Could not insert the STRIDE
            if (slot == this->reference_prediction_table_size) {
                PREFETCHER_DEBUG_PRINTF("Prefetcher: Cannot insert this stride on the reference_prediction_table.\n");
            }
        break;

        case PREFETCHER_DISABLE:
        break;

        default:
            ERROR_PRINTF("Invalid prefetch strategy %u.\n", this->get_prefetcher_type());
        break;
    }
};

/// ============================================================================
void prefetch_stride_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s STRIDE_TABLE:\n%s", this->get_label(), reference_prediction_line_t::print_all(this->reference_prediction_table, this->reference_prediction_table_size).c_str())
};

/// ============================================================================
void prefetch_stride_t::panic() {
    prefetch_t::panic();

    this->print_structures();
};

/// ============================================================================
void prefetch_stride_t::periodic_check(){
    prefetch_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
// STATISTICS
/// ============================================================================
void prefetch_stride_t::reset_statistics() {
    prefetch_t::reset_statistics();
};

/// ============================================================================
void prefetch_stride_t::print_statistics() {
    prefetch_t::print_statistics();
};

/// ============================================================================
void prefetch_stride_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "reference_prediction_table_size", reference_prediction_table_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_address_distance", stride_address_distance);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_window", stride_window);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_threshold_activate", stride_threshold_activate);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_prefetch_degree", stride_prefetch_degree);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_wait_between_requests", stride_wait_between_requests);
};
