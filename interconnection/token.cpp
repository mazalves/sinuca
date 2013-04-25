/// ============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
// Eduardo Henrique Molina da Cruz
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
token_t::token_t(){
    this->id_owner = 0;
    this->opcode_number = 0;
    this->opcode_address = 0;
    this->uop_number = 0;
    this->memory_address = 0;
    this->memory_operation = MEMORY_OPERATION_INST;
    this->is_coming = false;
};

/// ============================================================================
std::string token_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + " TOKEN: Owner:" + utils_t::uint32_to_char(this->id_owner);
    content_string = content_string + " OPCode#" + utils_t::uint64_to_char(this->opcode_number);
    content_string = content_string + " 0x" + utils_t::uint64_to_char(this->opcode_address);
    content_string = content_string + " UOP#" + utils_t::uint64_to_char(this->uop_number);

    content_string = content_string + " | " + get_enum_memory_operation_char(this->memory_operation);
    content_string = content_string + " 0x" + utils_t::uint64_to_char(this->memory_address);
    if (this->is_coming) {
        content_string = content_string + " COMING ";
    }
    else {
        content_string = content_string + " NOT_COMING";
    }
    return content_string;
};
