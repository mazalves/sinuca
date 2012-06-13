//==============================================================================
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
//==============================================================================
#include "../sinuca.hpp"

#ifdef PREFETCHER_DEBUG
    #define PREFETCHER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PREFETCHER_DEBUG_PRINTF(...)
#endif


//==============================================================================
prefetch_t::prefetch_t() {
    this->prefetcher_type = PREFETCHER_DISABLE;
    this->stream_table_size = 0;
    this->stream_address_range = 0;  /// double of line size
    this->stream_window = 0;
    this->stream_threshold_activate = 0;
    this->stream_distance = 0;
    this->stream_wait_between_requests = 0;

    this->request_buffer = NULL;
    this->request_buffer_position_start = 0;
    this->request_buffer_position_end = 0;
    this->request_buffer_position_used = 0;
    this->stream_table = NULL;
};

//==============================================================================
prefetch_t::~prefetch_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<memory_package_t>(request_buffer);
    utils_t::template_delete_array<cache_prefetch_stream_table_t>(stream_table);
};

//==============================================================================
void prefetch_t::allocate() {
    this->request_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_request_buffer_size());
    switch (this->get_prefetcher_type()) {
        case PREFETCHER_STREAM:
            this->stream_table = utils_t::template_allocate_array<cache_prefetch_stream_table_t>(this->get_stream_table_size());
        break;

        case PREFETCHER_DISABLE:
        break;

        default:
            ERROR_PRINTF("Invalid prefetch strategy %u.\n", this->get_prefetcher_type());
        break;
    }
};

//==============================================================================
void prefetch_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");

    switch (this->get_prefetcher_type()) {
        case PREFETCHER_STREAM:
            for (uint32_t slot = 0 ; slot < this->stream_table_size ; slot++) {
                if (this->stream_table[slot].relevance_count > 0) {

                    /// Useful Stream, but Long time since last activation
                    if (this->stream_table[slot].cycle_last_activation <= sinuca_engine.get_global_cycle() - this->stream_window) {
                        /// FREE the position
                        this->stream_table_line_clean(slot);
                    }

                    /// Something interesting to be requested. Check if can do a new request
                    else if (
                    /// Never requested stream
                    (this->stream_table[slot].relevance_count > this->stream_threshold_activate &&
                    this->stream_table[slot].prefetch_ahead == 0 )||
                    /// Already requested stream
                    (this->stream_table[slot].relevance_count > this->stream_threshold_activate &&
                    this->stream_table[slot].prefetch_ahead < this->stream_distance &&
                    this->stream_table[slot].cycle_last_request <= sinuca_engine.get_global_cycle() - this->stream_wait_between_requests)) {

                        PREFETCHER_DEBUG_PRINTF("New Prefetch found STREAM_BUFFER[%u] %s\n", slot, this->stream_table_line_to_string(slot).c_str());
                        int32_t position = this->request_buffer_insert();
                        if (position != POSITION_FAIL) {
                            this->stream_table[slot].prefetch_ahead++;
                            this->stream_table[slot].cycle_last_request = sinuca_engine.get_global_cycle();

                            uint64_t memory_address = this->stream_table[slot].last_memory_address +
                                                    ( this->stream_table[slot].prefetch_ahead * this->stream_table[slot].memory_address_difference);
                            this->request_buffer[position].packager(
                                                                0,                                          /// Request Owner
                                                                0,                                          /// Opcode. Number
                                                                0,                                          /// Opcode. Address
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

                            PREFETCHER_DEBUG_PRINTF("INSERTED on PREFETCHER_BUFFER[%d]", position);
                            PREFETCHER_DEBUG_PRINTF("%s", this->request_buffer[position].memory_to_string().c_str());
                            break;
                        }
                    }
                }
            }
        break;

        case PREFETCHER_DISABLE:
        break;

        default:
            ERROR_PRINTF("Invalid prefetch strategy %u.\n", this->get_prefetcher_type());
        break;
    }
};


//==============================================================================
bool prefetch_t::receive_package(memory_package_t *package, uint32_t input_port) {
    ERROR_PRINTF("Received package %s into the input_port %u.\n", package->memory_to_string().c_str(), input_port);
    return FAIL;
};

//======================================================================
void prefetch_t::treat_prefetch(memory_package_t *s) {
    uint32_t slot;

    if (s->memory_operation != MEMORY_OPERATION_READ || s->is_answer == true) {
        return;
    }
    else {
        switch (this->get_prefetcher_type()) {
            case PREFETCHER_STREAM:
                /// Try to match the request with some Stream
                for (slot = 0; slot < this->stream_table_size; slot++) {
                    if (this->stream_table[slot].last_memory_address == s->memory_address) {
                        PREFETCHER_DEBUG_PRINTF("Prefetcher: Found one stream... No Difference on Address - Ignoring.\n");
                        break;
                    }

                    /// Compute once the address diference, reused multiple times.
                    int64_t address_difference = s->memory_address - this->stream_table[slot].last_memory_address;

                    if ((this->stream_table[slot].memory_address_difference == address_difference || this->stream_table[slot].memory_address_difference == 0) &&
                    abs(address_difference) < stream_address_range) {
                        PREFETCHER_DEBUG_PRINTF("Prefetcher: Found one Stream ... %s\n", address_difference > 0 ? "Increasing" : "Decreasing");
                        this->stream_table[slot].memory_address_difference = address_difference;
                        this->stream_table[slot].last_memory_address = s->memory_address;
                        this->stream_table[slot].relevance_count++;
                        this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                        if (this->stream_table[slot].prefetch_ahead > 0) {
                            this->stream_table[slot].prefetch_ahead--;
                        }
                        break;
                    }
                }

                /// Could not find a STREAM, Create a new
                if (slot == this->stream_table_size) {
                    for (slot = 0 ; slot < this->stream_table_size ; slot++) {
                        /// Free slot
                        if (this->stream_table[slot].relevance_count == 0) {
                            PREFETCHER_DEBUG_PRINTF("Prefetcher: No stream found... Allocating it.\n");
                            this->stream_table[slot].memory_address_difference = 0;
                            this->stream_table[slot].last_memory_address = s->memory_address;
                            this->stream_table[slot].relevance_count = 1;
                            this->stream_table[slot].prefetch_ahead = 0;
                            this->stream_table[slot].cycle_last_activation = sinuca_engine.get_global_cycle();
                            this->stream_table[slot].cycle_last_request = sinuca_engine.get_global_cycle();
                            break;
                        }
                    }
                }

                /// Could not insert the STREAM
                if (slot == this->stream_table_size) {
                    PREFETCHER_DEBUG_PRINTF("Prefetcher: Cannot insert this stream on the stream_table.\n");
                }
            break;

            case PREFETCHER_DISABLE:
            break;

            default:
                ERROR_PRINTF("Invalid prefetch strategy %u.\n", this->get_prefetcher_type());
            break;
        }
    }
};

//==============================================================================
void prefetch_t::print_structures() {
    SINUCA_PRINTF("%s REQUEST_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->request_buffer_position_start, this->request_buffer_position_end, this->request_buffer_position_used);
    SINUCA_PRINTF("%s REQUEST_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->request_buffer, this->request_buffer_size).c_str() )
    SINUCA_PRINTF("%s STREAM_TABLE:\n%s", this->get_label(), this->stream_table_print_all().c_str() )
};

// =============================================================================
void prefetch_t::panic() {
    this->print_structures();
};

//==============================================================================
void prefetch_t::periodic_check(){
    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
    if (memory_package_t::check_age(this->request_buffer, this->request_buffer_size) != OK) {
        WARNING_PRINTF("Check_age failed.\n");
    }
    // ~ ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->request_buffer, this->request_buffer_size) == OK, "Check_age failed.\n");
};

// =============================================================================
// STATISTICS
// =============================================================================
void prefetch_t::reset_statistics() {
};

// =============================================================================
void prefetch_t::print_statistics() {
     char title[50] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};

// =============================================================================
void prefetch_t::print_configuration() {
    char title[50] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetcher_type", get_enum_prefetch_policy_char(prefetcher_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "full_buffer_type", get_enum_full_buffer_char(full_buffer_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "request_buffer_size", request_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_table_size", stream_table_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_address_range", stream_address_range);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_window", stream_window);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_threshold_activate", stream_threshold_activate);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_distance", stream_distance);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stream_wait_between_requests", stream_wait_between_requests);
};

//==============================================================================
// Request Buffer Methods
//==============================================================================
/*! Should make all the verifications before call this method, because it will
 * update the position_end and position_used for the fetch_buffer
 */
int32_t prefetch_t::request_buffer_insert() {
    int32_t valid_position = POSITION_FAIL;
    /// There is free space.
    if (this->request_buffer_position_used < this->request_buffer_size) {
        valid_position = this->request_buffer_position_end;
        this->request_buffer_position_used++;
        this->request_buffer_position_end++;
        if (this->request_buffer_position_end >= this->request_buffer_size) {
            this->request_buffer_position_end = 0;
        }
    }
    else {
        switch (this->full_buffer_type) {
            case FULL_BUFFER_OVERRIDE:
                valid_position = this->request_buffer_position_end;
                this->request_buffer_position_end++;
                if (this->request_buffer_position_end >= this->request_buffer_size) {
                    this->request_buffer_position_end = 0;
                }
                this->request_buffer_position_start++;
                if (this->request_buffer_position_start >= this->request_buffer_size) {
                    this->request_buffer_position_start = 0;
                }
            break;

            case FULL_BUFFER_STOP:
                valid_position = POSITION_FAIL;
            break;
        }
    }
    return valid_position;
};

// =============================================================================
memory_package_t* prefetch_t::request_buffer_get_older() {
    if (this->request_buffer_position_used > 0) {
        return (&this->request_buffer[this->request_buffer_position_start]);
    }
    else {
        return NULL;
    }
};


// =============================================================================
/*! Make sure that you want to remove the first element before call this method
 * because it will remove the buffer[position_start] and update the controls
 */
void prefetch_t::request_buffer_remove() {
    ERROR_ASSERT_PRINTF(this->request_buffer_position_used > 0, "Trying to remove from request_buffer with no used position.\n");

    this->request_buffer[this->request_buffer_position_start].package_clean();

    this->request_buffer_position_used--;
    this->request_buffer_position_start++;
    if (this->request_buffer_position_start >= this->request_buffer_size) {
        this->request_buffer_position_start = 0;
    }
};


//==============================================================================
// Stream Table Methods
//==============================================================================
void prefetch_t::stream_table_line_clean(uint32_t stream_buffer_line) {
    this->stream_table[stream_buffer_line].last_memory_address = 0;
    this->stream_table[stream_buffer_line].memory_address_difference = 0;
    this->stream_table[stream_buffer_line].relevance_count = 0;
    this->stream_table[stream_buffer_line].prefetch_ahead = 0;
    this->stream_table[stream_buffer_line].cycle_last_activation = 0;
    this->stream_table[stream_buffer_line].cycle_last_request = 0;
};

// =============================================================================
std::string prefetch_t::stream_table_line_to_string(uint32_t stream_buffer_line) {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->stream_table[stream_buffer_line].relevance_count == 0) {
            return PackageString;
        }
    #endif
    PackageString = PackageString + " STREAM: Last Address:" + utils_t::uint64_to_char(this->stream_table[stream_buffer_line].last_memory_address);
    PackageString = PackageString + " Address Difference:" + utils_t::int64_to_char(this->stream_table[stream_buffer_line].memory_address_difference);
    PackageString = PackageString + " Relevance:" + utils_t::uint32_to_char(this->stream_table[stream_buffer_line].relevance_count);
    PackageString = PackageString + " Prefetch Ahead:" + utils_t::uint32_to_char(this->stream_table[stream_buffer_line].prefetch_ahead);
    PackageString = PackageString + " Cycle Last Activation:" + utils_t::uint64_to_char(this->stream_table[stream_buffer_line].cycle_last_activation);
    PackageString = PackageString + " Cycle Last Request:" + utils_t::uint64_to_char(this->stream_table[stream_buffer_line].cycle_last_request);
    return PackageString;
};

// =============================================================================
std::string prefetch_t::stream_table_print_all() {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < this->stream_table_size ; i++) {
        PackageString = this->stream_table_line_to_string(i);
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::uint32_to_char(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};
