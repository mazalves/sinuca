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
dsbp_metadata_line_t::dsbp_metadata_line_t() {
    this->valid_sub_blocks = NULL;
    
    this->learn_mode = false;
    this->is_dirty = false;
    this->is_dead = false;

    this->real_usage_counter = NULL;
    this->usage_counter = NULL;
    this->overflow = NULL;

    this->pht_pointer = NULL;
    this->clock_become_alive = NULL;
    this->clock_become_dead = NULL;

    this->written_sub_blocks = NULL;
    this->active_sub_blocks = 0;

    this->stat_total_dead_cycles = 0;
};

/// ============================================================================
dsbp_metadata_line_t::~dsbp_metadata_line_t() {
    
    if (this->valid_sub_blocks) delete [] valid_sub_blocks;
    if (this->real_usage_counter) delete [] real_usage_counter;
    if (this->usage_counter) delete [] usage_counter;
    if (this->overflow) delete [] overflow;
};

/// ============================================================================
void dsbp_metadata_line_t::clean() {
    ERROR_ASSERT_PRINTF(this->valid_sub_blocks != NULL, "Cleanning a not allocated line.\n")

    // ~ ERROR_ASSERT_PRINTF(this->real_usage_counter != NULL, "Cleanning a not allocated line.\n")
    // ~ ERROR_ASSERT_PRINTF(this->usage_counter != NULL, "Cleanning a not allocated line.\n")
    // ~ ERROR_ASSERT_PRINTF(this->overflow != NULL, "Cleanning a not allocated line.\n")
// ~ 
    // ~ ERROR_ASSERT_PRINTF(this->clock_become_alive != NULL, "Cleanning a not allocated line.\n")
    // ~ ERROR_ASSERT_PRINTF(this->clock_become_dead != NULL, "Cleanning a not allocated line.\n")
// ~ 
    // ~ ERROR_ASSERT_PRINTF(this->written_sub_blocks != NULL, "Cleanning a not allocated line.\n")
  
    this->learn_mode = false;
    this->is_dirty = false;
    this->is_dead = false;
    this->pht_pointer = NULL;
    
    this->active_sub_blocks = 0;
    this->stat_total_dead_cycles = 0;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;

        this->real_usage_counter[i] = 0;
        this->usage_counter[i] = 0;
        this->overflow[i] = false;

        this->clock_become_alive[i] = 0;
        this->clock_become_dead[i] = 0;

        this->written_sub_blocks[i] = 0;
    }
};

/// ============================================================================
std::string dsbp_metadata_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Metadata -";
    content_string = content_string + " Learn:" + utils_t::uint32_to_char(this->learn_mode);    
    content_string = content_string + " Dirty:" + utils_t::uint32_to_char(this->is_dirty);
    content_string = content_string + " Dead:" + utils_t::uint32_to_char(this->is_dead);
    
    content_string = content_string + " pht_ptr:" + utils_t::uint32_to_char(this->pht_pointer != NULL);
    content_string = content_string + " Active:" + utils_t::uint32_to_char(this->active_sub_blocks);
    content_string = content_string + "\n";
    
    /// valid_sub_blocks 
    content_string = content_string + "\t valid_sub_blocks      [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_char(this->valid_sub_blocks[i]);
    }
    content_string = content_string + "]\n";

    /// real_usage_counter
    content_string = content_string + "\t real_usage_counter    [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_char(this->real_usage_counter[i]);
    }
    content_string = content_string + "]\n";

    /// usage_counter
    content_string = content_string + "\t usage_counter         [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_char(this->usage_counter[i]);
    }
    content_string = content_string + "]\n";

    /// overflow
    content_string = content_string + "\t overflow              [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_char(this->overflow[i]);
    }
    content_string = content_string + "]\n";

    return content_string;
};



