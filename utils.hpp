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

// ============================================================================
/// Utils
// ============================================================================
 /*! Provides useful generic methods used in multiple components like
  * constructors and destructors, number conversors, pretty print outs.
  */
class utils_t {
    public:
        // ====================================================================
        template <class TYPE>
        static void template_delete_variable(TYPE *variable) {
            /// Deallocate
            if (variable) {
                delete variable;
            }
        };
        // ====================================================================
        template <class TYPE>
        static void template_delete_array(TYPE *array) {
            /// Deallocate
            if (array != NULL) {
                delete[] array;
            }
        };
        // ====================================================================
        template <class TYPE>
        static void template_delete_matrix(TYPE **matrix, uint32_t count) {
            /// Deallocate
            if (count != 0 && matrix != NULL ) {
                if (matrix[0] != NULL) {
                    delete[] matrix[0];
                }
                delete[] matrix;
            }
        };


        // ====================================================================
        template <class TYPE>
        static TYPE* template_allocate_array(uint32_t count) {
            ERROR_ASSERT_PRINTF(count > 0, "Allocating array of %u positions\n", count);
            /// Allocate
            TYPE *var = new TYPE[count];
            return var;
        };

        // ====================================================================
        template <class TYPE>
        static TYPE* template_allocate_initialize_array(uint32_t count, TYPE init) {
            ERROR_ASSERT_PRINTF(count > 0, "Allocating array of %u positions\n", count);
            /// Allocate
            TYPE *var = new TYPE[count];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Initialize
            for (uint32_t position = 0; position < count; position++) {
                var[position] = init;
            };
            return var;
        };

        // ====================================================================
        template <class TYPE>
        static TYPE** template_allocate_matrix(uint32_t count_x, uint32_t count_y) {
            ERROR_ASSERT_PRINTF(count_x > 0, "Allocating matrix of %u(x) positions\n", count_x);
            ERROR_ASSERT_PRINTF(count_y > 0, "Allocating matrix of %u(y) positions\n", count_y);
            /// Allocate the pointers
            TYPE **var = new TYPE*[count_x];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Allocate all contiguously
            var[0] = new TYPE[count_x*count_y];
            ERROR_ASSERT_PRINTF(var[0] != NULL, "Could not allocate memory\n");
            /// Distribute over the positions
            for (uint32_t line = 1; line < count_x; line++) {
                var[line] = var[0] + (count_y * line);
            };
            return var;
        };

        // ====================================================================
        template <class TYPE>
        static TYPE** template_allocate_initialize_matrix(uint32_t count_x, uint32_t count_y, TYPE init) {
            ERROR_ASSERT_PRINTF(count_x > 0, "Allocating matrix of %u(x) positions\n", count_x);
            ERROR_ASSERT_PRINTF(count_y > 0, "Allocating matrix of %u(y) positions\n", count_y);
            /// Allocate the pointers
            TYPE **var = new TYPE*[count_x];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Allocate all contiguously
            var[0] = new TYPE[count_x*count_y];
            ERROR_ASSERT_PRINTF(var[0] != NULL, "Could not allocate memory\n");
            /// Initialize
            for (uint32_t position = 0; position < count_x*count_y; position++) {
                var[0][position] = init;
            };
            /// Distribute over the positions
            for (uint32_t line = 1; line < count_x; line++) {
                var[line] = var[0] + (count_y * line);
            };
            return var;
        };

        static void get_path(char *path, char *file_path);

        static uint64_t get_power_of_two(uint64_t n);
        static uint32_t check_if_power_of_two(uint64_t n);
        static uint64_t hash_function(hash_function_t type, uint64_t input1, uint64_t input2, uint64_t bit_size);
        static uint64_t fill_bit(uint32_t start, uint32_t end);

        static void bool_to_char(char *string, bool input_int);
        static void int32_to_char(char *string, int32_t input_int);
        static void uint32_to_char(char *string, uint32_t input_int);
        static void uint64_to_char(char *string, uint64_t input_int);
        static void int64_to_char(char *string, int64_t input_int);

        static uint64_t string_to_uint64(char *string);

        static std::string print_mask_of_bits(uint32_t line_size, uint32_t line_number, uint32_t assoc);
        static std::string progress_pretty(uint64_t actual, uint64_t total);
        static std::string connections_pretty(cache_memory_t *cache_memory, uint32_t level);
        static std::string address_to_binary(uint64_t address);

        static std::string bool_to_string(bool input_int);
        static std::string uint32_to_string(uint32_t input_int);
        static std::string int32_to_string(int32_t input_int);
        static std::string big_uint64_to_string(uint64_t input_int);
        static std::string uint64_to_string(uint64_t input_int);
        static std::string int64_to_string(int64_t input_int);

        static void process_mem_usage(double *stat_vm_usage, double *resident_set);
};
