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
#include "./../sinuca.hpp"
#include <string>

/// ============================================================================
stride_table_line_t::stride_table_line_t() {
    this->clean();
};

/// ============================================================================
stride_table_line_t::~stride_table_line_t() {
};

/// ============================================================================
void stride_table_line_t::clean() {
    this->last_opcode_address = 0;
    this->last_memory_address = 0;
    this->memory_address_difference = 0;
    this->prefetch_ahead = 0;
    this->cycle_last_activation = 0;
    this->stride_state = PREFETCHER_STRIDE_STATE_NO_PRED;
}

/// ============================================================================
std::string stride_table_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + " STRIDE:";
    content_string = content_string + " Last Opcode Address:" + utils_t::uint64_to_char(this->last_opcode_address);
    content_string = content_string + " Last Memory Address:" + utils_t::uint64_to_char(this->last_memory_address);
    content_string = content_string + " Address Difference:" + utils_t::int64_to_char(this->memory_address_difference);
    content_string = content_string + " Prefetch Ahead:" + utils_t::uint32_to_char(this->prefetch_ahead);
    content_string = content_string + " Cycle Last Activation:" + utils_t::uint64_to_char(this->cycle_last_activation);
    content_string = content_string + " Stride State:" + get_enum_prefetch_stride_state_char(this->stride_state);
    return content_string;
};

/// ============================================================================
std::string stride_table_line_t::print_all(stride_table_line_t *input_array, uint32_t size_array) {
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

