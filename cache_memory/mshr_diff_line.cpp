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

#include "./../sinuca.hpp"
#include <string>

// ============================================================================
mshr_diff_line_t::mshr_diff_line_t() {
    this->clean();
};

// ============================================================================
mshr_diff_line_t::~mshr_diff_line_t() {
};

// ============================================================================
void mshr_diff_line_t::clean(){
    this->valid = false;
    this->memory_address = 0;
    this->usage_counter = 0;
};

// ============================================================================
std::string mshr_diff_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->valid == false) {
            return content_string;
        }
    #endif

    content_string = content_string + " MSHR_DIFF_LINE: ";
    if (this->valid) {
        content_string = content_string + " VALID ";
    }
    else {
        content_string = content_string + " INVALID";
    }
    content_string = content_string + " 0x" + utils_t::uint64_to_string(this->memory_address);
    content_string = content_string + "  #" + utils_t::uint32_to_string(this->usage_counter);
    return content_string;
};


// ============================================================================
std::string mshr_diff_line_t::print_all(mshr_diff_line_t *input_array, uint32_t size_array) {
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

