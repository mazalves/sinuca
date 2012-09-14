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
class dlec_metadata_line_t {
    public:
        line_sub_block_t valid_sub_blocks;
        uint64_t real_usage_counter;
        uint64_t usage_counter;
        bool overflow;
        bool learn_mode;
        aht_line_t *aht_pointer;

        /// Static Energy
        uint64_t clock_become_alive;
        uint64_t clock_become_dead;

        /// Copyback Flag
        bool is_dirty;

        /// Dead Flag
        bool is_dead;

        dlec_metadata_line_t() {
            this->valid_sub_blocks = LINE_SUB_BLOCK_DISABLE;
            this->real_usage_counter = 0;
            this->usage_counter = 0;
            this->overflow = 0;
            this->learn_mode = false;
            this->aht_pointer = NULL;
            this->clock_become_alive = 0;
            this->clock_become_dead = 0;
            this->is_dirty = false;
            this->is_dead = true;
        };
        ~dlec_metadata_line_t() {
        };


};
