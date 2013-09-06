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
pht_line_t::pht_line_t() {
    this->opcode_address = 0;
    this->offset = 0;
    this->last_access = 0;
    this->pointer = 0;
    this->access_counter = NULL;
    this->overflow = NULL;
};

/// ============================================================================
pht_line_t::~pht_line_t() {
    if (this->access_counter) delete [] access_counter;
    if (this->overflow) delete [] overflow;
};

/// ============================================================================
void pht_line_t::clean() {
    ERROR_ASSERT_PRINTF(this->access_counter != NULL, "Cleanning a not allocated line.\n")
    // ~ ERROR_ASSERT_PRINTF(this->overflow != NULL, "Cleanning a not allocated line.\n")

    this->opcode_address = 0;
    this->offset = 0;
    this->last_access = 0;
    this->pointer = false;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->access_counter[i] = 0;
        this->overflow[i] = false;
    }
}

/// ============================================================================
std::string pht_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "pht_line -";
    content_string = content_string + " Opcode_Address:" + utils_t::uint64_to_string(this->opcode_address);
    content_string = content_string + " Offset:" + utils_t::uint32_to_string(this->offset);
    content_string = content_string + " Last_Access:" + utils_t::uint64_to_string(this->last_access);
    content_string = content_string + " Pointer:" + utils_t::uint32_to_string(this->pointer);
    content_string = content_string + "\n";

    /// access_counter
    content_string = content_string + "\t access_counter [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint64_to_string(this->access_counter[i]);
    }
    content_string = content_string + "]\n";

    /// overflow
    content_string = content_string + "\t overflow      [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::bool_to_string(this->overflow[i]);
    }
    content_string = content_string + "]\n";

    return content_string;
};


