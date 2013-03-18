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
reorder_buffer_line_t::reorder_buffer_line_t() {
    this->package_clean();
    this->reg_deps_ptr_array = NULL;
    this->mem_deps_ptr_array = NULL;
};

/// ============================================================================
reorder_buffer_line_t::~reorder_buffer_line_t() {
    utils_t::template_delete_array<reorder_buffer_line_t*>(reg_deps_ptr_array);
    utils_t::template_delete_array<reorder_buffer_line_t*>(mem_deps_ptr_array);
};

/// ============================================================================
void reorder_buffer_line_t::package_clean() {
    this->uop.package_clean();
    this->stage = PROCESSOR_STAGE_DECODE;
    this->mob_ptr = NULL;
    this->wait_reg_deps_number = 0;
    this->wait_mem_deps_number = 0;
};

/// ============================================================================
std::string reorder_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->uop.state == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif

    content_string = this->uop.content_to_string();
    content_string = content_string + " | STAGE:" + get_enum_processor_stage_char(this->stage);
    content_string = content_string + " | RegWAIT:" + utils_t::uint32_to_char(this->wait_reg_deps_number);
    content_string = content_string + " | MemWAIT:" + utils_t::uint32_to_char(this->wait_mem_deps_number);
    return content_string;
};

/// ============================================================================
/// STATIC METHODS
/// ============================================================================

std::string reorder_buffer_line_t::print_all(reorder_buffer_line_t *input_array, uint32_t size_array) {
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
bool reorder_buffer_line_t::check_age(reorder_buffer_line_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].uop.state != PACKAGE_STATE_FREE) {
            if (input_array[i].uop.state != PACKAGE_STATE_FREE && input_array[i].uop.born_cycle < min_cycle) {
                WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_array[i].content_to_string().c_str())
                return FAIL;
            }
        }
    }
    return OK;
};
