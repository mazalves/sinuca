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
skewed_metadata_line_t::skewed_metadata_line_t() {
    this->clean();
};

// ============================================================================
skewed_metadata_line_t::~skewed_metadata_line_t() {
};

// ============================================================================
void skewed_metadata_line_t::clean() {
    this->line_status = LINE_PREDICTION_TURNOFF;

    this->real_access_counter_read = 0;
    this->real_access_counter_writeback = 0;

    this->signature = 0;

    /// Special Flags
    this->is_dead_read = true;
    this->is_dead_writeback = true;
    this->is_dirty = false;

    /// Static Energy
    this->clock_last_access = 0;
};


// ============================================================================
std::string skewed_metadata_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Metadata -";
    content_string = content_string + " " + get_enum_line_prediction_t_char(this->line_status);

    content_string = content_string + " READ: RealAccess:" + utils_t::uint32_to_string(this->real_access_counter_read);
    content_string = content_string + " WRITE: RealAccess:" + utils_t::uint32_to_string(this->real_access_counter_writeback);

    content_string = content_string + " signature:" + utils_t::bool_to_string(this->signature);

    content_string = content_string + " is_dead_read:" + utils_t::bool_to_string(this->is_dead_read);
    content_string = content_string + " is_dead_write:" + utils_t::bool_to_string(this->is_dead_writeback);
    content_string = content_string + " is_dirty:" + utils_t::bool_to_string(this->is_dirty);

    content_string = content_string + " clock_last_access:" + utils_t::uint64_to_string(this->clock_last_access);

    content_string = content_string + "\n";
    return content_string;
};



