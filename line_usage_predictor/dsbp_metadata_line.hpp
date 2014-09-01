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

class dsbp_metadata_line_t {
    public:
        line_sub_block_t line_status;
        bool *sub_blocks;

        uint64_t *real_access_counter_read;
        uint64_t *access_counter_read;
        bool *overflow_read;

        bool learn_mode;
        pht_line_t *pht_pointer;

        /// Special Flags
        bool is_dead_read;
        bool is_dirty;

        /// Static Energy
        uint64_t clock_last_access;
        uint32_t active_sub_blocks;

        container_uint64_t cycle_access_list;
        uint64_t *clock_last_access_subblock; // ORACLE
        // ====================================================================
        /// Methods
        // ====================================================================
        dsbp_metadata_line_t();
        ~dsbp_metadata_line_t();

        void clean();
        std::string content_to_string();
};
