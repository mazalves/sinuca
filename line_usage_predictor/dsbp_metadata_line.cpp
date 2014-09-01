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
dsbp_metadata_line_t::dsbp_metadata_line_t() {
    this->line_status = LINE_SUB_BLOCK_DISABLE;

    this->sub_blocks = NULL;
    this->real_access_counter_read = NULL;
    this->access_counter_read = NULL;
    this->overflow_read = NULL;

    this->learn_mode = false;
    this->pht_pointer = NULL;

    /// Special Flags
    this->is_dead_read = true;
    this->is_dirty = false;

    /// Static Energy
    this->clock_last_access = 0;
    this->active_sub_blocks = 0;

    this->clock_last_access_subblock = NULL;
};

// ============================================================================
dsbp_metadata_line_t::~dsbp_metadata_line_t() {
    utils_t::template_delete_array<bool>(this->sub_blocks);

    utils_t::template_delete_array<uint64_t>(this->real_access_counter_read);
    utils_t::template_delete_array<uint64_t>(this->access_counter_read);
    utils_t::template_delete_array<bool>(this->overflow_read);

    utils_t::template_delete_array<uint64_t>(this->clock_last_access_subblock);
};

// ============================================================================
void dsbp_metadata_line_t::clean() {
    ERROR_ASSERT_PRINTF(this->sub_blocks != NULL, "Cleanning a not allocated line.\n")

    this->line_status = LINE_SUB_BLOCK_DISABLE;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->sub_blocks[i] = false;
        this->real_access_counter_read[i] = 0;
        this->access_counter_read[i] = 0;
        this->overflow_read[i] = false;
    }

    this->learn_mode = false;
    this->pht_pointer = NULL;

    /// Special Flags
    this->is_dead_read = true;
    this->is_dirty = false;

    /// Static Energy
    this->clock_last_access = 0;
    this->active_sub_blocks = 0;

    if (this->clock_last_access_subblock != NULL) {
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->clock_last_access_subblock[i] = 0;
        }
    }

    this->cycle_access_list.clear();
};

// ============================================================================
std::string dsbp_metadata_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    content_string = content_string + "Metadata -";
    content_string = content_string + " " + get_enum_line_sub_block_t_char(this->line_status);

    /// line_status
    content_string = content_string + "\n SubBlocks:\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + "  " + utils_t::bool_to_string(this->sub_blocks[i]);
    }
    content_string = content_string + "|\n";


    content_string = content_string + " RealAccess:\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_string(this->real_access_counter_read[i]);
    }
    content_string = content_string + "|\n";


    content_string = content_string + " Predicted:\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + " " + utils_t::uint32_to_string(this->access_counter_read[i]);
    }
    content_string = content_string + "|\n";


    content_string = content_string + " Overflow:\t";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % 4 == 0) {
            content_string = content_string + "|";
        }
        content_string = content_string + "  " + utils_t::bool_to_string(this->overflow_read[i]);
    }
    content_string = content_string + "|\n";


    content_string = content_string + " learn_mode:" + utils_t::bool_to_string(this->learn_mode);
    content_string = content_string + " pht_ptr:" + utils_t::bool_to_string(this->pht_pointer != NULL);

    content_string = content_string + " is_dead_read:" + utils_t::bool_to_string(this->is_dead_read);
    content_string = content_string + " is_dirty:" + utils_t::bool_to_string(this->is_dirty);

    content_string = content_string + " clock_last_access:" + utils_t::uint64_to_string(this->clock_last_access);
    content_string = content_string + " active:" + utils_t::uint32_to_string(this->active_sub_blocks);

    content_string = content_string + "\n";
    return content_string;
};



