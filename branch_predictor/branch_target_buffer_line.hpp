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

class branch_target_buffer_line_t {
    public:
        uint64_t tag;
        uint64_t last_access;
        uint64_t usage_counter;

        // ====================================================================
        /// Methods
        // ====================================================================
        branch_target_buffer_line_t() {
            this->tag = 0;
            this->last_access = 0;
            this->usage_counter = 0;
        };
        ~branch_target_buffer_line_t() {
        };
};
