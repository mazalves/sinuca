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
dlec_metadata_line_t::dlec_metadata_line_t() {
    this->clean();
};

/// ============================================================================
dlec_metadata_line_t::~dlec_metadata_line_t() {
};

/// ============================================================================
void dlec_metadata_line_t::clean() {
    this->valid_sub_blocks = LINE_SUB_BLOCK_DISABLE;
    this->learn_mode = false;
    this->is_dirty = false;
    this->is_dead = false;
    this->is_last_write = false;
    
    this->real_access_counter = 0;
    this->real_write_counter = 0;
    
    this->access_counter = 0;
    this->overflow = false;

    this->ahtm_pointer = NULL;
    this->ahtc_pointer = NULL;
    this->clock_become_alive = 0;
    this->clock_become_dead = 0;

    this->clock_first_write = 0;
    this->clock_last_write = 0;        
};

/// ============================================================================
std::string dlec_metadata_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Metadata -";
    content_string = content_string + " " + utils_t::uint32_to_char(this->valid_sub_blocks);

    content_string = content_string + " Learn:" + utils_t::uint32_to_char(this->learn_mode);    
    content_string = content_string + " Dirty:" + utils_t::uint32_to_char(this->is_dirty); 
    content_string = content_string + " Dead:" + utils_t::uint32_to_char(this->is_dead);
    content_string = content_string + " Last_Write:" + utils_t::uint32_to_char(this->is_last_write);

    content_string = content_string + " RealAccess:" + utils_t::uint32_to_char(this->real_access_counter);
    content_string = content_string + " RealWrite:" + utils_t::uint32_to_char(this->real_write_counter);
        
    content_string = content_string + " Usage:" + utils_t::uint32_to_char(this->access_counter);
    content_string = content_string + " Overflow:" + utils_t::uint32_to_char(this->overflow);
    
    content_string = content_string + " ahtm_ptr:" + utils_t::uint32_to_char(this->ahtm_pointer != NULL);
    content_string = content_string + " ahtc_ptr:" + utils_t::uint32_to_char(this->ahtc_pointer != NULL);    

    content_string = content_string + " Alive:" + utils_t::uint64_to_char(this->clock_become_alive);
    content_string = content_string + " Dead:" + utils_t::uint64_to_char(this->clock_become_dead);

    content_string = content_string + " First_Write:" + utils_t::uint64_to_char(this->clock_first_write);
    content_string = content_string + " Last_Write:" + utils_t::uint64_to_char(this->clock_last_write);


    content_string = content_string + "\n";
    return content_string;
};



