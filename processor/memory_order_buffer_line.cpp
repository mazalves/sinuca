/// ============================================================================
//
// Copyright (C) 2010, 2011, 2012
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
memory_order_buffer_line_t::memory_order_buffer_line_t() {
    this->package_clean();
};

/// ============================================================================
memory_order_buffer_line_t::~memory_order_buffer_line_t() {
};


/// ============================================================================
void memory_order_buffer_line_t::package_clean() {
    this->memory_request.package_clean();
    this->rob_ptr = NULL;
};

/// ============================================================================
std::string memory_order_buffer_line_t::content_to_string() {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->rob_ptr == NULL) {
            return PackageString;
        }
    #endif

    PackageString = this->memory_request.content_to_string();
    PackageString = PackageString + " | UOP#" + utils_t::uint64_to_char(this->rob_ptr == NULL ? 0 : this->rob_ptr->uop.uop_number);
    PackageString = PackageString + " | MemWait:" + utils_t::uint32_to_char(this->rob_ptr == NULL ? 0 : this->rob_ptr->wait_mem_deps_number);
    return PackageString;
};

/// ============================================================================
/// STATIC METHODS
/// ============================================================================
int32_t memory_order_buffer_line_t::find_free(memory_order_buffer_line_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].rob_ptr == NULL) {
            ERROR_ASSERT_PRINTF(input_array[i].memory_request.state == PACKAGE_STATE_FREE, "Found a mob without rob_ptr but with memory_request.\n")
            return i;
        }
    }
    return POSITION_FAIL;
};

/// ============================================================================
int32_t memory_order_buffer_line_t::find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    uint64_t old_uop_number = std::numeric_limits<uint64_t>::max();

    /// Find the oldest UOP inside the MOB.... and it have 0 deps.
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].rob_ptr != NULL &&
        input_array[i].rob_ptr->wait_mem_deps_number == 0 &&
        input_array[i].memory_request.state == state &&
        input_array[i].memory_request.is_answer == false &&
        old_uop_number > input_array[i].memory_request.uop_number &&
        input_array[i].memory_request.ready_cycle <= sinuca_engine.get_global_cycle()) {
            old_uop_number = input_array[i].memory_request.uop_number;
            old_pos = i;
        }
    }
    return old_pos;
};

/// ============================================================================
int32_t memory_order_buffer_line_t::find_uop_number(memory_order_buffer_line_t *input_array, uint32_t size_array, uint64_t uop_number) {

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].rob_ptr != NULL &&
        input_array[i].memory_request.state != PACKAGE_STATE_FREE &&
        input_array[i].memory_request.uop_number == uop_number) {
            return i;
        }
    }

    return POSITION_FAIL;
};

/// ============================================================================
std::string memory_order_buffer_line_t::print_all(memory_order_buffer_line_t *input_array, uint32_t size_array) {
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

/// ============================================================================
bool memory_order_buffer_line_t::check_age(memory_order_buffer_line_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].memory_request.state != PACKAGE_STATE_FREE) {
            if (input_array[i].memory_request.born_cycle < min_cycle) {
                WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_array[i].content_to_string().c_str())
                return FAIL;
            }
        }
    }
    return OK;
};
