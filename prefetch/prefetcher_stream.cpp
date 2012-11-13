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


/// ============================================================================ DONE
prefetch_stream_t::prefetch_stream_t() {
    this->set_prefetcher_type(PREFETCHER_STREAM);
    
    this->stream_table_size = 0;

    this->prefetch_distance = 0;  
    this->search_distance = 0;
    this->prefetch_degree = 0;
    
    this->lifetime_cycles = 0;

    this->stream_table = NULL;

};

/// ============================================================================ DONE
prefetch_stream_t::~prefetch_stream_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<stream_table_line_t>(stream_table);
};

/// ============================================================================ DONE
void prefetch_stream_t::allocate() {
    prefetch_t::allocate();
    
    ERROR_ASSERT_PRINTF(this->prefetch_degree != 0, "Prefetch degree should be at least 1.\n")
    ERROR_ASSERT_PRINTF(this->prefetch_distance >= 2, "Address distance should be at least 2.\n")
    ERROR_ASSERT_PRINTF(this->search_distance != 0, "Search distance should be at least 1.\n")
    ERROR_ASSERT_PRINTF(this->lifetime_cycles >= 100, "Lifetime cycles should be reasonably large, recommended 10000.\n")
    this->stream_table = utils_t::template_allocate_array<stream_table_line_t>(this->get_stream_table_size());

};

/// ============================================================================ working on it NOW
void prefetch_stream_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");
};

//====================================================================== working on it...
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


    ///first check for a monitor and request entry available to match
    for (slot = 0; slot < this->stream_table_size; slot++){
        if (this->stream_table[slot].state == PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST){
            if ((this->stream_table[slot].direction == 1) &&  ///if direction is upstream,  (this is a bit of short circuiting with &&)
                ( (package->memory_address >= this->stream_table[slot].starting_address) &&  ///if this package is between starting address
                    (package->memory_address <= this->stream_table[slot].ending_address))){   ///and ending address
                    /// detected valid prefetching;
                    found = 1;
                    this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle(); 
                    
                    if ((this->stream_table[slot].ending_address - 
                        package->memory_address + (sinuca_engine.get_global_line_size()*this->prefetch_degree)) <= (sinuca_engine.get_global_line_size()*this->prefetch_distance)) 
                    {
                        for (uint32_t index=1; index <= this->prefetch_degree; index++){
                                    
                                int32_t position = this->request_buffer_insert();
                                if (position != POSITION_FAIL) {

                                    /// Statistics
                                    this->add_stat_created_prefetches();
                                    this->add_stat_upstride_prefetches();

                                    uint64_t opcode_address = package->opcode_address ;
                                    uint64_t memory_address = (this->stream_table[slot].ending_address & this->not_offset_bits_mask) + (sinuca_engine.get_global_line_size()*index);

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
                    this->stream_table[slot].ending_address += sinuca_engine.get_global_line_size()*this->prefetch_degree;
                    this->stream_table[slot].starting_address = package->memory_address;       
                    break;
                }
            }
            else if ((this->stream_table[slot].direction == 0) &&  ///if direction is downstream,  (this is a bit of short circuiting with &&)
                ( (package->memory_address <= this->stream_table[slot].starting_address) &&  ///if this package is between starting address
                    (package->memory_address >= this->stream_table[slot].ending_address))){   ///and ending address
                    ///detected valid prefetching;
                    found = 1;
                    this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle(); 
                    
                    if ((package->memory_address - this->stream_table[slot].ending_address + 
                        (sinuca_engine.get_global_line_size()*this->prefetch_degree)) <= (sinuca_engine.get_global_line_size()*this->prefetch_distance)) 
                    {
                        for (uint32_t index=1; index <= this->prefetch_degree; index++){
                                    
                                int32_t position = this->request_buffer_insert();
                                if (position != POSITION_FAIL) {

                                    /// Statistics
                                    this->add_stat_created_prefetches();
                                    this->add_stat_downstride_prefetches();

                                    uint64_t opcode_address = package->opcode_address ;
                                    uint64_t memory_address = (this->stream_table[slot].ending_address & this->not_offset_bits_mask) - (sinuca_engine.get_global_line_size()*index);

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
                    this->stream_table[slot].ending_address -= sinuca_engine.get_global_line_size()*this->prefetch_degree;
                    this->stream_table[slot].starting_address = package->memory_address;
                    break;
                }
            }
        }
    }
    
    
    if ((!found) && (package->is_answer == false)){     ///did not match this address to any monitor and request, and it is a MISS, so it can be used for allocating and training
        for (slot = 0; slot < this->stream_table_size; slot++){
            switch (this->stream_table[slot].state){
                case PREFETCHER_STREAM_STATE_INVALID: ///invalid/empty
                        available_slot = slot;
                break;
                
                case PREFETCHER_STREAM_STATE_ALLOCATED: ///allocated
                        if ((package->memory_address > this->stream_table[slot].starting_address) && 
                            (package->memory_address <= (this->stream_table[slot].starting_address + this->search_distance*sinuca_engine.get_global_line_size()))){ ///checking for upwards miss
                            
                            this->stream_table[slot].ending_address = package->memory_address;
                            this->stream_table[slot].direction = 1;
                            this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            this->stream_table[slot].state = PREFETCHER_STREAM_STATE_TRAINING;
                            found2 = 1;
                            
                        }
                        else if ((package->memory_address < this->stream_table[slot].starting_address) && 
                            (package->memory_address >= (this->stream_table[slot].starting_address - this->search_distance*sinuca_engine.get_global_line_size()))){ ///checking for downwards miss
                            
                            this->stream_table[slot].ending_address = package->memory_address;
                            this->stream_table[slot].direction = 0;
                            this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            this->stream_table[slot].state = PREFETCHER_STREAM_STATE_TRAINING;
                            found2 = 1;
                            
                        }
                break;
                
                case PREFETCHER_STREAM_STATE_TRAINING: ///training
                        if ((package->memory_address > this->stream_table[slot].ending_address) && 
                            (package->memory_address <= (this->stream_table[slot].ending_address + (this->search_distance*sinuca_engine.get_global_line_size())))){ ///checking for confirmation of direction to enter monitor and request, upwards
                            found2 = 1;
                            if (this->stream_table[slot].direction == 1){
                                this->stream_table[slot].ending_address = package->memory_address  + (sinuca_engine.get_global_line_size()*(this->prefetch_distance/2)); ///ATTENTION: using prefetch_degree as initial startup value.
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST;
                                
                            }
                            else {
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;    ///failure to obtain a direction pattern, returning to "allocated" state considering this last access
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();

                            }
                        }
                        else if ((package->memory_address < this->stream_table[slot].ending_address) && 
                            (package->memory_address >= (this->stream_table[slot].ending_address - (this->search_distance*sinuca_engine.get_global_line_size())))){///checking for confirmation of direction to enter monitor and request, downwards
                            found2 = 1;
                            if (this->stream_table[slot].direction == 0){
                                this->stream_table[slot].ending_address = package->memory_address - (sinuca_engine.get_global_line_size()*(this->prefetch_distance/2)); ///ATTENTION: using prefetch_degree as initial startup value.
                                this->stream_table[slot].starting_address = package->memory_address;
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST;
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            }
                            else {
                                this->stream_table[slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
                                this->stream_table[slot].starting_address = package->memory_address;///failure to obtain a direction pattern, returning to "allocated" state considering this last access
                                this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();

                            }
                        }
                break;
                        
                case PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST:    ///should only get here on M&R, which we already checked for and got no matches.
                break;
                
            }
        }   
    }
    
    if (((!found) && (!found2)) && ((package->is_answer == false))){ ///did not detect the pattern for any other stream, create a new stream if there is an available slot
        if (available_slot != POSITION_FAIL){
            this->stream_table[available_slot].state = PREFETCHER_STREAM_STATE_ALLOCATED;
            this->stream_table[available_slot].starting_address = package->memory_address;
            this->stream_table[available_slot].cycle_last_activation = sinuca_engine.get_global_cycle();
        }
        else {
            for (slot = 0; slot < this->stream_table_size; slot++){
                if (sinuca_engine.get_global_cycle() > (this->stream_table[slot].cycle_last_activation + this->lifetime_cycles)){
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

/// ============================================================================ DONE 
void prefetch_stream_t::print_structures() {
    prefetch_t::print_structures();

    SINUCA_PRINTF("%s TABLE:\n%s", this->get_label(), stream_table_line_t::print_all(this->stream_table, this->stream_table_size).c_str());
};

/// ============================================================================ DONE 
void prefetch_stream_t::panic() {
    prefetch_t::panic();

    this->print_structures();
};

/// ============================================================================ DONE 
void prefetch_stream_t::periodic_check(){
    prefetch_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================ DONE 
void prefetch_stream_t::reset_statistics() {
    prefetch_t::reset_statistics();
};

/// ============================================================================ DONE 
void prefetch_stream_t::print_statistics() {
    prefetch_t::print_statistics();
};

/// ============================================================================ DONE 
void prefetch_stream_t::print_configuration() {
    prefetch_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_table_size", stream_table_size);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_distance", prefetch_distance);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetch_degree", prefetch_degree);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "search_distance", search_distance);
    
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "lifetime_cycles", lifetime_cycles);
    
};
