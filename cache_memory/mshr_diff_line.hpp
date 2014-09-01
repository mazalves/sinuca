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

class mshr_diff_line_t {
    public:
        bool valid;
        uint64_t memory_address;
        uint32_t usage_counter;

        // ====================================================================
        /// Methods
        // ====================================================================
        mshr_diff_line_t();
        ~mshr_diff_line_t();

        void clean();

        std::string content_to_string();
        static std::string print_all(mshr_diff_line_t *input_array, uint32_t size_array);
};
