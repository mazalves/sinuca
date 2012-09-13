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
class dsbp_metadata_line_t {
    public:
        line_sub_block_t *valid_sub_blocks;
        uint64_t *real_usage_counter;
        uint64_t *usage_counter;
        bool *overflow;
        bool learn_mode;
        pht_line_t *pht_pointer;

        /// Static Energy
        uint64_t *clock_become_alive;
        uint64_t *clock_become_dead;

        /// Copyback
        bool is_dirty;
        uint64_t *written_sub_blocks;

        /// Dead Flag
        uint32_t active_sub_blocks;
        bool is_dead;

        /// Dead Statistics
        uint64_t stat_total_dead_cycles;

        dsbp_metadata_line_t() {
            this->valid_sub_blocks = NULL;
            this->real_usage_counter = NULL;
            this->usage_counter = NULL;
            this->overflow = NULL;
            this->learn_mode = 0;
            this->pht_pointer = NULL;
            this->clock_become_alive = NULL;
            this->clock_become_dead = NULL;
            this->is_dirty = false;
            this->written_sub_blocks = NULL;
            this->active_sub_blocks = 0;
            this->is_dead = true;
            this->stat_total_dead_cycles = 0;
        };
        ~dsbp_metadata_line_t() {
            if (this->valid_sub_blocks) delete [] valid_sub_blocks;
            if (this->real_usage_counter) delete [] real_usage_counter;
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_total_dead_cycles);
};
