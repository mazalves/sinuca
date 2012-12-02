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

    this->stride_table_size = 0;
    this->prefetch_degree = 0;
    this->stride_table = NULL;
};

/// ============================================================================
prefetch_stride_t::~prefetch_stride_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<stride_table_line_t>(stride_table);
};

/// ============================================================================
void prefetch_stride_t::allocate() {
    prefetch_t::allocate();

    ERROR_ASSERT_PRINTF(this->prefetch_degree != 0, "Prefetch degree should be at least 1.\n")
    this->stride_table = utils_t::template_allocate_array<stride_table_line_t>(this->get_stride_table_size());
};

/// ============================================================================
void prefetch_stride_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");
};

//======================================================================
void prefetch_stride_t::treat_prefetch(memory_package_t *package) {
    uint32_t slot;

    /// Try to match the request with some Stream
    for (slot = 0; slot < this->stride_table_size; slot++) {
        if (this->stride_table[slot].last_opcode_address == package->opcode_address) {

            int64_t address_difference = package->memory_address - this->stride_table[slot].last_memory_address;
            bool is_stride_match = (address_difference == this->stride_table[slot].memory_address_difference && address_difference != 0);

            /// Statistics
            if (is_stride_match) {
                this->add_stat_request_matches();
            }

            switch (this->stride_table[slot].stride_state) {
                /// Set at first entry in the RPT or after the entry experienced an incorrect prediction from STATE_STEADY.
                case PREFETCHER_STRIDE_STATE_INIT:
                    if (is_stride_match) {
                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;
                        this->add_stat_steady_state();
                    }
                    else {
                        this->stride_table[slot].memory_address_difference = address_difference;

                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_TRANSIENT;
                        this->add_stat_transient_state();
                    }
                break;

                /// Corresponds to the case when the system is not sure whether the previous prediction was good or not.
                /// The new stride will be obtained by subtracting the previous address from the currently referenced address.
                case PREFETCHER_STRIDE_STATE_TRANSIENT:
                    if (is_stride_match) {
                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;
                        this->add_stat_steady_state();
                    }
                    else {
                        this->stride_table[slot].memory_address_difference = address_difference;

                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_NO_PRED;
                        this->add_stat_no_pred_state();
                    }
                break;

                /// Indicates that the prediction should be stable for a while.
                case PREFETCHER_STRIDE_STATE_STEADY:
                    if (is_stride_match) {
                        /// Reduce the number of prefetchs ahead
                        if (this->stride_table[slot].prefetch_ahead > 0) {
                            this->stride_table[slot].prefetch_ahead--;
                        }
                        /////////////////////////////////////////////////////
                        // ~ else {
                            // ~ printf("\n--\n");
                        // ~ }
                        /////////////////////////////////////////////////////
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;
                        this->add_stat_steady_state();

                        /// Generate the prefetches
                        for (uint32_t index = 1; index <= this->prefetch_degree; index++) {
                            /// Check if can generate one more prefetch
                            if (this->stride_table[slot].prefetch_ahead >= this->prefetch_degree) {
                                break;
                            }

                            /// Check if it will request different cache line (already requested)
                            uint64_t last_request_address = package->memory_address +
                                                    ( this->stride_table[slot].prefetch_ahead * this->stride_table[slot].memory_address_difference);

                            uint64_t new_request_address = last_request_address + this->stride_table[slot].memory_address_difference;

                            /////////////////////////////////////////////////////
                            // ~ printf("Slot:%d Opcode:%"PRIu64" Diff:%"PRIu64" PackAddr:%"PRIu64" New:%"PRIu64" \n",
                                    // ~ slot,
                                    // ~ this->stride_table[slot].last_opcode_address,
                                    // ~ this->stride_table[slot].memory_address_difference,
                                    // ~ package->memory_address,
                                    // ~ new_request_address);
                            /////////////////////////////////////////////////////

                            PREFETCHER_DEBUG_PRINTF("New Prefetch found BUFFER[%u] %s\n", slot, this->stride_table[slot].content_to_string().c_str());
                            if (!this->cmp_index_tag(package->memory_address, new_request_address) &&
                            !this->cmp_index_tag(last_request_address, new_request_address)) {

                                /// Find free position into the request buffer
                                int32_t position = this->request_buffer_insert();
                                if (position != POSITION_FAIL) {

                                    /// Statistics
                                    this->add_stat_created_prefetches();
                                    if (this->stride_table[slot].memory_address_difference > 0) {
                                        this->add_stat_upstride_prefetches();
                                    }
                                    else {
                                        this->add_stat_downstride_prefetches();
                                    }

                                    uint64_t opcode_address = this->stride_table[slot].last_opcode_address;

                                    this->request_buffer[position].packager(
                                                                        0,                                          /// Request Owner
                                                                        0,                                          /// Opcode. Number
                                                                        opcode_address,                             /// Opcode. Address
                                                                        0,                                          /// Uop. Number

                                                                        new_request_address,                        /// Mem. Address
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
                                /// Cannot insert a new request
                                else {
                                    break;
                                }
                            }
                            /// Request was in the same cache line (already requested)
                            else {
                                /// Statistics
                                this->add_stat_dropped_prefetches();
                                PREFETCHER_DEBUG_PRINTF("\t DROPPED PREFETCH\n");
                                ////////////////////////////////////////////////
                                // ~ printf("\t DROPPED PREFETCH\n");
                                ////////////////////////////////////////////////
                            }
                            this->stride_table[slot].prefetch_ahead++;
                        }
                    }
                    else {
                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_INIT;
                        this->add_stat_init_state();
                    }
                break;

                /// Disables the prefetching for this entry for the time being.
                case PREFETCHER_STRIDE_STATE_NO_PRED:
                    if (is_stride_match) {
                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_TRANSIENT;
                        this->add_stat_transient_state();
                    }
                    else {
                        this->stride_table[slot].memory_address_difference = address_difference;

                        this->stride_table[slot].prefetch_ahead = 0;
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_NO_PRED;
                        this->add_stat_no_pred_state();
                    }
                break;
            }
            return;
        }
    }

    /// Could not find a STRIDE, Evict the LRU position to create a new stride.
    uint64_t old_position = this->stride_table_size;
    uint64_t old_cycle = sinuca_engine.get_global_cycle() + 1;
    for (slot = 0 ; slot < this->stride_table_size ; slot++) {
        /// Free slot
        if (old_cycle > this->stride_table[slot].cycle_last_activation) {
            old_position = slot;
            old_cycle = this->stride_table[slot].cycle_last_activation;
        }
    }

    slot = old_position;
    ERROR_ASSERT_PRINTF(slot < this->stride_table_size, "Could not insert this stride on the stride_table.\n");

    this->add_stat_allocate_stride_ok();
    PREFETCHER_DEBUG_PRINTF("Prefetcher: No stride found... Allocating it.\n");

    this->stride_table[slot].clean();
    this->stride_table[slot].last_opcode_address = package->opcode_address;
    this->stride_table[slot].last_memory_address = package->memory_address;
    this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
    this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_INIT;
    this->add_stat_init_state();
};

/// ============================================================================
void prefetch_stride_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s TABLE:\n%s", this->get_label(), stride_table_line_t::print_all(this->stride_table, this->stride_table_size).c_str())
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

    this->stat_init_state = 0;
    this->stat_transient_state = 0;
    this->stat_steady_state = 0;
    this->stat_no_pred_state = 0;


    this->stat_allocate_stride_ok = 0;
    this->stat_allocate_stride_fail = 0;
};

/// ============================================================================
void prefetch_stride_t::print_statistics() {
    prefetch_t::print_statistics();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_init_state", stat_init_state);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_transient_state", stat_transient_state);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_steady_state", stat_steady_state);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_no_pred_state", stat_no_pred_state);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_allocate_stride_ok", stat_allocate_stride_ok);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_allocate_stride_fail", stat_allocate_stride_fail);
};

/// ============================================================================
void prefetch_stride_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_table_size", stride_table_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_degree", prefetch_degree);
};
