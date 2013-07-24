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
dewp_metadata_line_t::dewp_metadata_line_t() {
    this->clean();
    this->reset_statistics();
};

/// ============================================================================
dewp_metadata_line_t::~dewp_metadata_line_t() {
};

/// ============================================================================
void dewp_metadata_line_t::clean() {
    this->line_status = LINE_SUB_BLOCK_DISABLE;

    this->real_access_counter_read = 0;
    this->real_access_counter_writeback = 0;

    this->access_counter_read = 0;
    this->access_counter_writeback = 0;

    this->overflow_read = false;
    this->overflow_writeback = false;

    this->learn_mode = false;
    this->aht_pointer = NULL;

    /// Special Flags
    this->is_dead_read = true;
    this->is_dead_writeback = true;
    this->is_dirty = false;

    /// Static Energy
    this->clock_first_access = 0;
};

/// ============================================================================
void dewp_metadata_line_t::reset_statistics() {

};

/// ============================================================================
std::string dewp_metadata_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Metadata -";
    content_string = content_string + " " + get_enum_line_sub_block_t_char(this->line_status);

    content_string = content_string + " READ: RealAccess:" + utils_t::uint32_to_char(this->real_access_counter_read);
    content_string = content_string + " Predicted:" + utils_t::uint32_to_char(this->access_counter_read);
    content_string = content_string + " Overflow:" + utils_t::bool_to_char(this->overflow_read);

    content_string = content_string + " WRITE: RealAccess:" + utils_t::uint32_to_char(this->real_access_counter_writeback);
    content_string = content_string + " Predicted:" + utils_t::uint32_to_char(this->access_counter_writeback);
    content_string = content_string + " Overflow:" + utils_t::bool_to_char(this->overflow_writeback);


    content_string = content_string + " learn_mode:" + utils_t::bool_to_char(this->learn_mode);
    content_string = content_string + " aht_ptr:" + utils_t::bool_to_char(this->aht_pointer != NULL);

    content_string = content_string + " is_dead_read:" + utils_t::bool_to_char(this->is_dead_read);
    content_string = content_string + " is_dead_write:" + utils_t::bool_to_char(this->is_dead_writeback);
    content_string = content_string + " is_dirty:" + utils_t::bool_to_char(this->is_dirty);

    content_string = content_string + " clock_first_access:" + utils_t::uint64_to_char(this->clock_first_access);

    content_string = content_string + "\n";
    return content_string;
};



