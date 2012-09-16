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
        aht_line_t *ahtm_pointer;
        aht_line_t *ahtc_pointer;

        /// Static Energy
        uint64_t clock_become_alive;
        uint64_t clock_become_dead;

        /// Copyback Flag
        bool is_dirty;
        bool is_last_write;

        /// Dead Flag
        bool is_dead;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        dlec_metadata_line_t();
        ~dlec_metadata_line_t();

        void clean();
        std::string content_to_string();


};
