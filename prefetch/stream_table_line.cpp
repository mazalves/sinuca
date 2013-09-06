/// ============================================================================
//
// Copyright (C) 2010, 2011
// Marco Antonio Zanata Alves
//
// Modified by Francis Birck Moreira 2012
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
#include "./../sinuca.hpp"
#include <string>

/// ============================================================================
stream_table_line_t::stream_table_line_t() {
    this->clean();
};

/// ============================================================================
stream_table_line_t::~stream_table_line_t() {
};

/// ============================================================================
void stream_table_line_t::clean() {
    this->first_address = 0;             /// First address that generated this stream (line_usage_predictor information)

     this->starting_address = 0;
     this->ending_address = 0;
     this->direction = 0;

     this->cycle_last_activation = 0;         /// Last time a Memory Request matched into this stream
     this->cycle_last_request = 0;            /// Last prefetch done
     this->last_memory_address = 0;

     this->state = PREFETCHER_STREAM_STATE_INVALID;
}

/// ============================================================================
std::string stream_table_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PREFETCHER_STREAM_STATE_INVALID && this->starting_address == 0) {
            return content_string;
        }
    #endif


    content_string = content_string + " STREAM: First Address:" + utils_t::uint64_to_string(this->first_address); /// line_usage_predictor information
    content_string = content_string + " Starting Address:" + utils_t::uint64_to_string(this->starting_address);
    content_string = content_string + " Ending Address:" + utils_t::uint64_to_string(this->ending_address);
    content_string = content_string + " Stream Direction:" + utils_t::uint32_to_string(this->direction);

    content_string = content_string + " Stream Last Activation:" + utils_t::int64_to_string(this->cycle_last_activation);
    content_string = content_string + " Stream Last Request:" + utils_t::uint64_to_string(this->cycle_last_request);
    content_string = content_string + " Stream Last Address:" + utils_t::uint64_to_string(this->last_memory_address);

    content_string = content_string + " Stream State:" + get_enum_prefetch_stream_state_char(this->state);
    return content_string;
};

/// ============================================================================
std::string stream_table_line_t::print_all(stream_table_line_t *input_array, uint32_t size_array) {
    std::string content_string;
    std::string final_string;

    final_string = "";
    for (uint32_t i = 0; i < size_array ; i++) {
        content_string = "";
        content_string = input_array[i].content_to_string();
        if (content_string.size() > 1) {
            final_string = final_string + "[" + utils_t::uint32_to_string(i) + "] " + content_string + "\n";
        }
    }
    return final_string;
};

