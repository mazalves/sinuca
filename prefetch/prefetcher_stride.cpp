/*
 * Copyright (C) 2010~2014  Marco Antonio Zanata Alves
 *                          (mazalves at inf.ufrgs.br)
 *                          GPPD - Parallel and Distributed Processing Group
 *                          Universidade Federal do Rio Grande do Sul
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../sinuca.hpp"

#ifdef PREFETCHER_DEBUG
    #define PREFETCHER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PREFETCHER_DEBUG_PRINTF(...)
#endif


// ============================================================================
prefetch_stride_t::prefetch_stride_t() {
    this->set_prefetcher_type(PREFETCHER_STRIDE);

    this->stride_table_size = 0;
    this->prefetch_degree = 0;
    this->search_distance = 0;
    this->next_lines_prefetch = 0;

    this->last_request_address = 0;
    this->last_prefetch_address = 0;

    this->stride_table = NULL;
};

// ============================================================================
prefetch_stride_t::~prefetch_stride_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<stride_table_line_t>(stride_table);
};

// ============================================================================
void prefetch_stride_t::allocate() {
    prefetch_t::allocate();

    ERROR_ASSERT_PRINTF(this->prefetch_degree != 0, "Prefetch degree should be at least 1.\n")
    this->stride_table = utils_t::template_allocate_array<stride_table_line_t>(this->get_stride_table_size());
};


// ============================================================================
void prefetch_stride_t::treat_prefetch(memory_package_t *package) {
    ERROR_ASSERT_PRINTF(package->is_answer == false, "Should never receive an answer.\n")
    uint32_t slot;
    uint64_t new_request_address = 0;
    bool found_stride = false;

    /// Try to match the request with some Stream
    for (slot = 0; slot < this->stride_table_size; slot++) {
        if (this->stride_table[slot].last_opcode_address == package->opcode_address) {

            int64_t address_difference = package->memory_address - this->stride_table[slot].last_memory_address;

            bool is_stride_match;
            if (address_difference >= 0)
                is_stride_match = address_difference == this->stride_table[slot].memory_address_difference &&
                                   address_difference != 0 &&
                                  address_difference <= (search_distance * sinuca_engine.get_global_line_size());
            else
                is_stride_match = address_difference == this->stride_table[slot].memory_address_difference &&
                                   address_difference != 0 &&
                                  address_difference >= -1 * (search_distance * sinuca_engine.get_global_line_size());

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
                        this->stride_table[slot].last_memory_address = package->memory_address;
                        this->stride_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        this->stride_table[slot].stride_state = PREFETCHER_STRIDE_STATE_STEADY;
                        this->add_stat_steady_state();
                        this->add_stat_request_matches();

                        /// Generate the prefetches
                        for (uint32_t index = 1; index <= this->prefetch_degree; index++) {
                            /// Check if can generate one more prefetch
                            if (this->stride_table[slot].prefetch_ahead >= this->prefetch_degree) {
                                break;
                            }

                            /// Check if it will request different cache line (already requested)
                            uint64_t last_request_address = package->memory_address +
                                                    ( this->stride_table[slot].prefetch_ahead * this->stride_table[slot].memory_address_difference);

                            new_request_address = last_request_address + this->stride_table[slot].memory_address_difference;


                            PREFETCHER_DEBUG_PRINTF("New Prefetch found BUFFER[%u] %s\n", slot, this->stride_table[slot].content_to_string().c_str());
                            if (!this->cmp_index_tag(package->memory_address, new_request_address) &&
                            !this->cmp_index_tag(last_request_address, new_request_address)) {

                                /// Find free position into the request buffer
                                if (!this->request_buffer.is_full()) {
                                    /// Statistics
                                    this->add_stat_created_prefetches();
                                    if (this->stride_table[slot].memory_address_difference > 0) {
                                        this->add_stat_upstride_prefetches();
                                    }
                                    else {
                                        this->add_stat_downstride_prefetches();
                                    }

                                    uint64_t opcode_address = this->stride_table[slot].last_opcode_address;

                                    memory_package_t new_request;
                                    new_request.packager(
                                                        0,                                      /// Request Owner
                                                        0,                                      /// Opcode. Number
                                                        opcode_address,                         /// Opcode. Address
                                                        0,                                      /// Uop. Number

                                                        new_request_address,                    /// Mem. Address
                                                        sinuca_engine.get_global_line_size(),   /// Block Size

                                                        PACKAGE_STATE_UNTREATED,                /// Pack. State
                                                        0,                                      /// Ready Cycle

                                                        MEMORY_OPERATION_PREFETCH,              /// Mem. Operation
                                                        false,                                  /// Is Answer

                                                        0,                                      /// Src ID
                                                        0,                                      /// Dst ID
                                                        NULL,                                   /// *Hops
                                                        0);                                     /// Hop Counter

                                    PREFETCHER_DEBUG_PRINTF("\t NEW PREFETCH %s", new_request.content_to_string().c_str());
                                    this->request_buffer.push_back(new_request);
                                }
                                else {
                                    this->add_stat_full_buffer();
                                }
                            }
                            /// Request was in the same cache line (already requested)
                            else {
                                /// Statistics
                                this->add_stat_dropped_prefetches();
                                PREFETCHER_DEBUG_PRINTF("\t DROPPED PREFETCH\n");
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
            found_stride = true;
            break;
        }
    }

    // =========================================================================
    /// Next line prefetch
    if (this->next_lines_prefetch > 0 &&    /// If it is set to generate next line prefetch
        (package->memory_address - this->last_request_address) <= sinuca_engine.get_global_line_size() &&  /// Check if lines are consecutives
        new_request_address != package->memory_address + sinuca_engine.get_global_line_size()) {  /// Do not generate same address twice

        /// Generate the prefetches
        for (uint32_t index = 1; index <= this->next_lines_prefetch; index++) {
            /// Generate a next lines prefetch
            new_request_address = package->memory_address + (index * sinuca_engine.get_global_line_size());

            /// Avoid prefetching same line multiple times
            if (new_request_address < this->last_prefetch_address) {
                break;
            }
            this->last_prefetch_address = new_request_address;

            /// Find free position into the request buffer
            if (!this->request_buffer.is_full()) {
                /// Statistics
                this->add_stat_created_prefetches();
                this->add_stat_upstride_prefetches();
                this->add_stat_next_line_prefetches();

                memory_package_t new_request;
                new_request.packager(
                                    0,                                      /// Request Owner
                                    0,                                      /// Opcode. Number
                                    package->opcode_address,                         /// Opcode. Address
                                    0,                                      /// Uop. Number

                                    new_request_address,                    /// Mem. Address
                                    sinuca_engine.get_global_line_size(),   /// Block Size

                                    PACKAGE_STATE_UNTREATED,                /// Pack. State
                                    0,                                      /// Ready Cycle

                                    MEMORY_OPERATION_PREFETCH,              /// Mem. Operation
                                    false,                                  /// Is Answer

                                    0,                                      /// Src ID
                                    0,                                      /// Dst ID
                                    NULL,                                   /// *Hops
                                    0);                                     /// Hop Counter

                PREFETCHER_DEBUG_PRINTF("\t NEW PREFETCH %s", new_request.content_to_string().c_str());
                this->request_buffer.push_back(new_request);
            }
            else {
                this->add_stat_full_buffer();
            }
        }
    }
    this->last_request_address = package->opcode_address;


    // =========================================================================
    /// Could not find a STRIDE, Evict the LRU position to create a new stride.
    if (!found_stride) {
        uint64_t old_position = this->stride_table_size;
        uint64_t old_cycle = std::numeric_limits<uint64_t>::max();
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
    }
};

// ============================================================================
void prefetch_stride_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s TABLE:\n%s", this->get_label(), stride_table_line_t::print_all(this->stride_table, this->stride_table_size).c_str())
};

// ============================================================================
void prefetch_stride_t::panic() {
    prefetch_t::panic();

    this->print_structures();
};

// ============================================================================
void prefetch_stride_t::periodic_check(){
    prefetch_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

// ============================================================================
/// STATISTICS
// ============================================================================
void prefetch_stride_t::reset_statistics() {
    prefetch_t::reset_statistics();

    this->stat_init_state = 0;
    this->stat_transient_state = 0;
    this->stat_steady_state = 0;
    this->stat_no_pred_state = 0;


    this->stat_allocate_stride_ok = 0;
    this->stat_allocate_stride_fail = 0;

    this->stat_next_line_prefetches = 0;
};

// ============================================================================
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

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_next_line_prefetches", stat_next_line_prefetches);
};

// ============================================================================
void prefetch_stride_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stride_table_size", stride_table_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_degree", prefetch_degree);
};
