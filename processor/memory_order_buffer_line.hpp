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

class memory_order_buffer_line_t {
    public:
        memory_package_t memory_request;                /// memory_request stored
        reorder_buffer_line_t* rob_ptr;                 /// rob pointer

        /// Memory Dependencies Control
        bool uop_executed;
        uint32_t wait_mem_deps_number;                  /// Must wait BEFORE execution
        memory_order_buffer_line_t* *mem_deps_ptr_array;     /// Elements to wake-up AFTER execution

        // ====================================================================
        /// Methods
        // ====================================================================
        memory_order_buffer_line_t();
        ~memory_order_buffer_line_t();

        void package_clean();
        std::string content_to_string();

        static int32_t find_free(memory_order_buffer_line_t *input_array, uint32_t size_array);
        static int32_t find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state);
        static int32_t find_uop_number(memory_order_buffer_line_t *input_array, uint32_t size_array, uint64_t uop_number);
        static std::string print_all(memory_order_buffer_line_t *input_array, uint32_t size_array);
        static bool check_age(memory_order_buffer_line_t *input_array, uint32_t size_array);
};
