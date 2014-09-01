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

#include "../sinuca.hpp"

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

// ============================================================================
pht_line_t::pht_line_t() {
    this->opcode_address = 0;
    this->offset = 0;
    this->last_access = 0;
    this->pointer = false;

    this->access_counter_read = 0;
    this->overflow_read = 0;
};

// ============================================================================
pht_line_t::~pht_line_t() {
    if (this->access_counter_read) delete [] access_counter_read;
    if (this->overflow_read) delete [] overflow_read;
};

// ============================================================================
void pht_line_t::clean() {
    ERROR_ASSERT_PRINTF(this->access_counter_read != NULL, "Cleanning a not allocated line.\n")
    ERROR_ASSERT_PRINTF(this->overflow_read != NULL, "Cleanning a not allocated line.\n")

    this->opcode_address = 0;
    this->offset = 0;
    this->last_access = 0;
    this->pointer = false;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->access_counter_read[i] = 0;
        this->overflow_read[i] = false;
    }
}

// ============================================================================
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
    content_string = content_string + "Predicted\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_string(this->access_counter_read[i]);
    }
    content_string = content_string + "|\n";

    /// overflow
    content_string = content_string + "Overflow\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + "  " + utils_t::bool_to_string(this->overflow_read[i]);
    }
    content_string = content_string + "|\n";

    return content_string;
};


