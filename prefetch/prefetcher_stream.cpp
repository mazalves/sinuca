/// ============================================================================
//
// Copyright (C) 2010, 2011
// Marco Antonio Zanata Alves
//
//
// Modified by Francis Birck Moreira
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
prefetch_stream_t::prefetch_stream_t() {
    this->set_prefetcher_type(PREFETCHER_STREAM);

    this->stream_table_size = 0;

    this->prefetch_distance = 0;
    this->search_distance = 0;
    this->prefetch_degree = 0;

    this->lifetime_cycles = 0;

    this->stream_table = NULL;

};

/// ============================================================================
prefetch_stream_t::~prefetch_stream_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<stream_table_line_t>(stream_table);
};

/// ============================================================================
void prefetch_stream_t::allocate() {
    prefetch_t::allocate();

    ERROR_ASSERT_PRINTF(this->prefetch_degree > 0, "Prefetch degree should be at least 1.\n")
    ERROR_ASSERT_PRINTF(this->prefetch_distance >= 2, "Address distance should be at least 2.\n")
    ERROR_ASSERT_PRINTF(this->search_distance > 0, "Search distance should be at least 1.\n")
    ERROR_ASSERT_PRINTF(this->lifetime_cycles >= 100, "Lifetime cycles should be reasonably large, recommended 10000.\n")
    this->stream_table = utils_t::template_allocate_array<stream_table_line_t>(this->get_stream_table_size());

};

/// ============================================================================
void prefetch_stream_t::treat_prefetch(memory_package_t *package) {
    uint32_t slot;
    bool found = 0;
    bool found2 = 0;
    int32_t available_slot = POSITION_FAIL;

    ///1.gets any L2 requests --
    ///2. Checks if the request belongs to a tracking entry. --
    ///3. If it does: --
        //Check entry state. if Monitor & Request:
            ///prefetch ending_address+1...ending_address+P
            /// ending_address += P --
            /// starting_address = ending_address - prefetch_distance (aka prefetch distance) --
        // else if allocated:
            /// if address > starting_address && address < (starting_address + 16)
                // ending_address = address, direction = 1; state = training;
            ///else if address < starting_address && address > (starting_address - 16)
                // ending_address = address, direction = 0; state = training;

        //else if training:
                ///if address > ending_address && address < (ending_address + 16)
                    //if direction == 1
                        ///ending_address = address + prefetch_degree (OR initial startup distance... maybe "wait between requests" ?)
                        ///state = monitor and request
                    //else
                        ///state = allocated;
                ///else if address < ending_address && > (ending_address - 16)
                    //if direction == 0
                        ///ending_address = address - prefetch_degree
                        ///state = monitor and request
                    //else
                        ///state = allocated;

    /// ========================================================================
    /// Check for a MONITOR AND REQUEST entry available to match
    /// ========================================================================
    for (slot = 0; slot < this->stream_table_size; slot++){
        if (this->stream_table[slot].state == PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST) {
            if (this->stream_table[slot].direction == 1 &&                            /// direction is upstream &&
            package->memory_address >= this->stream_table[slot].starting_address &&   /// this package is between starting address &&
            package->memory_address <= this->stream_table[slot].ending_address) {     /// and ending address
                /// detected valid prefetching;
                found = 1;
                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();

                if ((this->stream_table[slot].ending_address - package->memory_address +
                (sinuca_engine.get_global_line_size() * this->prefetch_degree)) <= (sinuca_engine.get_global_line_size() * this->prefetch_distance)) {
                    for (uint32_t index = 1; index <= this->prefetch_degree; index++) {

                        int32_t position = this->request_buffer_insert();
                        if (position != POSITION_FAIL) {

                            /// Statistics
                            this->add_stat_created_prefetches();
                            this->add_stat_upstride_prefetches();

                            uint64_t opcode_address = package->opcode_address ;
                            uint64_t memory_address = (this->stream_table[slot].ending_address & this->not_offset_bits_mask) + (sinuca_engine.get_global_line_size() * index);

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

                            PREFETCHER_DEBUG_PRINTF("\t %s", this->request_buffer[position].content_to_string().c_str());
                            PREFETCHER_DEBUG_PRINTF("\t INSERTED on PREFETCHER_BUFFER[%d]\n", position);
                        }
                    }
                    /// The following update is to avoid the START become greater than END
                    /// If the stride of the access is bigger than the prefetch degree ... update using the stride to update the ending address
                    if (package->memory_address - this->stream_table[slot].starting_address >  sinuca_engine.get_global_line_size() * this->prefetch_degree) {
                        this->stream_table[slot].ending_address += package->memory_address - this->stream_table[slot].starting_address;
                    }
                    /// Otherwise, use the prefetch degree to update the ending address
                    else {
                        this->stream_table[slot].ending_address += sinuca_engine.get_global_line_size() * this->prefetch_degree;
                    }
                    // ~ this->stream_table[slot].ending_address += sinuca_engine.get_global_line_size() * this->prefetch_degree;
                    this->stream_table[slot].starting_address = package->memory_address;
                    break;
                }
            }
            else if (this->stream_table[slot].direction == 0 &&                       /// direction is downstream &&
            package->memory_address <= this->stream_table[slot].starting_address &&   /// this package is between starting address &&
            package->memory_address >= this->stream_table[slot].ending_address){      /// and ending address
                ///detected valid prefetching;
                found = 1;
                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();

                if ((package->memory_address - this->stream_table[slot].ending_address +
                (sinuca_engine.get_global_line_size() * this->prefetch_degree)) <= (sinuca_engine.get_global_line_size() * this->prefetch_distance)) {
                    for (uint32_t index = 1; index <= this->prefetch_degree; index++) {

                        int32_t position = this->request_buffer_insert();
                        if (position != POSITION_FAIL) {

                            /// Statistics
                            this->add_stat_created_prefetches();
                            this->add_stat_downstride_prefetches();

                            uint64_t opcode_address = package->opcode_address ;
                            uint64_t memory_address = (this->stream_table[slot].ending_address & this->not_offset_bits_mask) - (sinuca_engine.get_global_line_size() * index);

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
                    }
                    /// The following update is to avoid the START become greater than END
                    /// If the stride of the access is bigger than the prefetch degree ... update using the stride to update the ending address
                    if (this->stream_table[slot].starting_address - package->memory_address > sinuca_engine.get_global_line_size() * this->prefetch_degree) {
                        this->stream_table[slot].ending_address += package->memory_address - this->stream_table[slot].starting_address;
                    }
                    /// Otherwise, use the prefetch degree to update the ending address
                    else {
                        this->stream_table[slot].ending_address -= sinuca_engine.get_global_line_size() * this->prefetch_degree;
                    }
                    // ~ this->stream_table[slot].ending_address -= sinuca_engine.get_global_line_size() * this->prefetch_degree;
                    this->stream_table[slot].starting_address = package->memory_address;
                    break;
                }
            }
        }
    }


    /// ========================================================================
    /// Did not match this address to any MONITOR AND REQUEST,
    /// and it is a MISS, so it can be used for allocating and training
    /// ========================================================================
    if (!found && package->is_answer == false) {
        for (slot = 0; slot < this->stream_table_size; slot++) {
            switch (this->stream_table[slot].state) {
                case PREFETCHER_STREAM_STATE_INVALID:
                        available_slot = slot;
                break;

                case PREFETCHER_STREAM_STATE_ALLOCATED:
                        /// Checking for upwards miss
                        if (package->memory_address > this->stream_table[slot].starting_address &&
                        package->memory_address <= (this->stream_table[slot].starting_address + (this->search_distance * sinuca_engine.get_global_line_size()))) {

                            this->stream_table[slot].ending_address = package->memory_address;
                            this->stream_table[slot].direction = 1;
                            this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            this->stream_table[slot].state = PREFETCHER_STREAM_STATE_TRAINING;
                            found2 = 1;
                        }
                        /// Checking for downwards miss
                        else if (package->memory_address < this->stream_table[slot].starting_address &&
                        package->memory_address >= (this->stream_table[slot].starting_address - (this->search_distance * sinuca_engine.get_global_line_size()))) {

                            this->stream_table[slot].ending_address = package->memory_address;
                            this->stream_table[slot].direction = 0;
                            this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            this->stream_table[slot].state = PREFETCHER_STREAM_STATE_TRAINING;
                            found2 = 1;
                        }
                break;

                case PREFETCHER_STREAM_STATE_TRAINING:
                        /// Checking for confirmation of direction to enter monitor and request, upwards
                        if (package->memory_address > this->stream_table[slot].ending_address &&
                        package->memory_address <= (this->stream_table[slot].ending_address + (this->search_distance * sinuca_engine.get_global_line_size()))){
                            found2 = 1;
                            /// Success to obtain a direction pattern
                            if (this->stream_table[slot].direction == 1) {
                                this->stream_table[slot].ending_address = package->memory_address  + (sinuca_engine.get_global_line_size() * this->prefetch_distance);
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST;
                            }
                            /// Failure to obtain a direction pattern, returning to STATE_ALLOCATED considering this last access
                            else {
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            }
                        }
                        /// Checking for confirmation of direction to enter monitor and request, downwards
                        else if (package->memory_address < this->stream_table[slot].ending_address &&
                        package->memory_address >= (this->stream_table[slot].ending_address - (this->search_distance * sinuca_engine.get_global_line_size()))){
                            found2 = 1;
                            /// Success to obtain a direction pattern
                            if (this->stream_table[slot].direction == 0) {
                                this->stream_table[slot].ending_address = package->memory_address - (sinuca_engine.get_global_line_size() * this->prefetch_distance);
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            }
                            /// Failure to obtain a direction pattern, returning to STATE_ALLOCATED considering this last access
                            else {
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            }
                        }
                break;

                case PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST:    ///should only get here on M&R, which we already checked for and got no matches.
                break;
            }
        }
    }

    /// ========================================================================
    /// Did not detect the pattern for ANY other stream,
    /// create a new stream if there is an available slot
    /// ========================================================================
    if (!found && !found2 && package->is_answer == false) {
        if (available_slot != POSITION_FAIL) {
            this->stream_table[available_slot].clean();
            this->stream_table[available_slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
            this->stream_table[available_slot].starting_address = package->memory_address;
            this->stream_table[available_slot].cycle_last_activation = sinuca_engine.get_global_cycle();
        }
        else {
            for (slot = 0; slot < this->stream_table_size; slot++) {
                if ((this->stream_table[slot].cycle_last_activation + this->lifetime_cycles) < sinuca_engine.get_global_cycle()) {
                    this->stream_table[slot].clean();
                    this->stream_table[slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
                    this->stream_table[slot].starting_address = package->memory_address;
                    this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                    break;
                }
            }
        }
    }
};

/// ============================================================================
void prefetch_stream_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s TABLE:\n%s", this->get_label(), stream_table_line_t::print_all(this->stream_table, this->stream_table_size).c_str());
};

/// ============================================================================
void prefetch_stream_t::panic() {
    prefetch_t::panic();

    this->print_structures();
};

/// ============================================================================
void prefetch_stream_t::periodic_check(){
    prefetch_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void prefetch_stream_t::reset_statistics() {
    prefetch_t::reset_statistics();
};

/// ============================================================================
void prefetch_stream_t::print_statistics() {
    prefetch_t::print_statistics();
};

/// ============================================================================
void prefetch_stream_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_table_size", stream_table_size);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_distance", prefetch_distance);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_degree", prefetch_degree);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "search_distance", search_distance);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "lifetime_cycles", lifetime_cycles);
};
