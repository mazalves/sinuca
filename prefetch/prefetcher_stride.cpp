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
    this->prefetch_degree = 0;
    this->wait_between_requests = 0;
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

    ERROR_ASSERT_PRINTF(this->prefetch_degree != 0, "Prefetch degree should be at least 1.\n")
    this->reference_prediction_table = utils_t::template_allocate_array<reference_prediction_line_t>(this->get_reference_prediction_table_size());
};

/// ============================================================================
void prefetch_stride_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");

    for (uint32_t slot = 0 ; slot < this->reference_prediction_table_size ; slot++) {
        /// Something interesting to be requested. Check if can do a new request
        if (this->reference_prediction_table[slot].stride_state == PREFETCHER_STRIDE_STATE_STEADY &&
        this->reference_prediction_table[slot].prefetch_ahead < this->prefetch_degree &&
        this->reference_prediction_table[slot].cycle_last_request <= sinuca_engine.get_global_cycle() - this->wait_between_requests) {

            /// Check if it will request different cache line (already requested)
            uint64_t last_memory_address = this->reference_prediction_table[slot].last_memory_address +
                                    ( this->reference_prediction_table[slot].prefetch_ahead * this->reference_prediction_table[slot].memory_address_difference);

            uint64_t memory_address = this->reference_prediction_table[slot].last_memory_address +
                                    ( (this->reference_prediction_table[slot].prefetch_ahead + 1) * this->reference_prediction_table[slot].memory_address_difference);

            PREFETCHER_DEBUG_PRINTF("New Prefetch found BUFFER[%u] %s\n", slot, this->reference_prediction_table[slot].content_to_string().c_str());
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
                }
                break;
            }
            /// Request was in the same cache line (already requested)
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

    /// Try to match the request with some Stream
    for (slot = 0; slot < this->reference_prediction_table_size; slot++) {
        if (this->reference_prediction_table[slot].last_opcode_address == package->opcode_address) {
            uint64_t next_address = this->reference_prediction_table[slot].last_memory_address + this->reference_prediction_table[slot].memory_address_difference;
            bool is_correct = (next_address == package->memory_address);

            switch (this->reference_prediction_table[slot].stride_state) {
                /// Set at first entry in the RPT or after the entry experienced an incorrect prediction from STATE_STEADY.
                case PREFETCHER_STRIDE_STATE_INIT:
                    if (is_correct) {
                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;

                        /// Statistics
                        this->add_stat_steady_state();
                    }
                    else {
                        uint64_t address_difference = package->memory_address - this->reference_prediction_table[slot].last_memory_address;
                        this->reference_prediction_table[slot].memory_address_difference = address_difference;

                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_TRANSIENT;
                    }
                break;

                /// Corresponds to the case when the system is not sure whether the previous prediction was good or not.
                /// The new stride will be obtained by subtracting the previous address from the currently referenced address.
                case PREFETCHER_STRIDE_STATE_TRANSIENT:
                    if (is_correct) {
                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;

                        /// Statistics
                        this->add_stat_steady_state();
                    }
                    else {
                        uint64_t address_difference = package->memory_address - this->reference_prediction_table[slot].last_memory_address;
                        this->reference_prediction_table[slot].memory_address_difference = address_difference;

                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_NO_PRED;
                    }
                break;

                /// Indicates that the prediction should be stable for a while.
                case PREFETCHER_STRIDE_STATE_STEADY:
                    if (is_correct) {
                        if (this->reference_prediction_table[slot].prefetch_ahead > 0) {
                            this->reference_prediction_table[slot].prefetch_ahead--;
                        }
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;

                        /// Statistics
                        this->add_stat_steady_state();
                    }
                    else {
                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_INIT;
                    }
                break;

                /// Disables the prefetching for this entry for the time being.
                case PREFETCHER_STRIDE_STATE_NO_PRED:
                    if (is_correct) {
                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_TRANSIENT;
                    }
                    else {
                        uint64_t address_difference = package->memory_address - this->reference_prediction_table[slot].last_memory_address;
                        this->reference_prediction_table[slot].memory_address_difference = address_difference;

                        this->reference_prediction_table[slot].prefetch_ahead = 0;
                        this->reference_prediction_table[slot].last_memory_address = package->memory_address;
                        this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
                        this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_NO_PRED;
                    }
                break;
            }
            /// Statistics
            this->add_stat_request_matches();
            return;
        }
    }

    /// Could not find a STRIDE, Evict the LRU position to create a new stride.
    uint64_t old_position = this->reference_prediction_table_size;
    uint64_t old_cycle = sinuca_engine.get_global_cycle();
    for (slot = 0 ; slot < this->reference_prediction_table_size ; slot++) {
        /// Free slot
        if (old_cycle > this->reference_prediction_table[slot].cycle_last_activation) {
            old_position = slot;
            old_cycle = this->reference_prediction_table[slot].cycle_last_activation;
        }
    }

    slot = old_position;
    this->add_stat_allocate_stride_ok();
    PREFETCHER_DEBUG_PRINTF("Prefetcher: No stride found... Allocating it.\n");

    this->reference_prediction_table[slot].clean();
    this->reference_prediction_table[slot].last_opcode_address = package->opcode_address;
    this->reference_prediction_table[slot].last_memory_address = package->memory_address;
    this->reference_prediction_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();                        
    this->reference_prediction_table[slot].stride_state = PREFETCHER_STRIDE_STATE_INIT;


    /// Could not insert the STRIDE
    if (slot >= this->reference_prediction_table_size) {
        this->add_stat_allocate_stride_fail();
        PREFETCHER_DEBUG_PRINTF("Prefetcher: Cannot insert this stride on the reference_prediction_table.\n");
    }

};

/// ============================================================================
void prefetch_stride_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s TABLE:\n%s", this->get_label(), reference_prediction_line_t::print_all(this->reference_prediction_table, this->reference_prediction_table_size).c_str())
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
/// STATISTICS
/// ============================================================================
void prefetch_stride_t::reset_statistics() {
    prefetch_t::reset_statistics();

    this->stat_steady_state = 0;
    this->stat_allocate_stride_ok = 0;
    this->stat_allocate_stride_fail = 0;
};

/// ============================================================================
void prefetch_stride_t::print_statistics() {
    prefetch_t::print_statistics();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_steady_state", stat_steady_state);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_allocate_stride_ok", stat_allocate_stride_ok);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_allocate_stride_fail", stat_allocate_stride_fail);
};

/// ============================================================================
void prefetch_stride_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "reference_prediction_table_size", reference_prediction_table_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_degree", prefetch_degree);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_requests", wait_between_requests);
};
