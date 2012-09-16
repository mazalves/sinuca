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

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

/// ============================================================================
aht_line_t::aht_line_t() {
    this->clean();
};

/// ============================================================================
aht_line_t::~aht_line_t() {
};

/// ============================================================================
void aht_line_t::clean() {
    this->opcode_address = 0;
    this->offset = 0;
    this->last_access = 0;
    this->pointer = false;
    this->access_counter = 0;
    this->overflow = 0;
    this->is_last_write = false;
};

/// ============================================================================
std::string aht_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "aht_line -";
    content_string = content_string + " Opcode_Address:" + utils_t::uint64_to_char(this->opcode_address);
    content_string = content_string + " Offset:" + utils_t::uint32_to_char(this->offset);
    content_string = content_string + " Last_Access:" + utils_t::uint64_to_char(this->last_access);
    content_string = content_string + " Pointer:" + utils_t::uint32_to_char(this->pointer);

    content_string = content_string + " Usage_Counter:" + utils_t::uint32_to_char(this->access_counter);
    content_string = content_string + " Overflow:" + utils_t::uint32_to_char(this->overflow);
    content_string = content_string + " Is_Last_Write:" + utils_t::uint32_to_char(this->is_last_write);
    content_string = content_string + "\n";
    return content_string;
};


