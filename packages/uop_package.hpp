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
/// Microcode Package (Low Level Instruction)
// ============================================================================
class uop_package_t {
    public:
        /// TRACE Variables
        char opcode_assembly[MAX_ASSEMBLY_SIZE];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        int32_t read_regs[MAX_REGISTERS];
        int32_t write_regs[MAX_REGISTERS];

        instruction_operation_t uop_operation;
        uint64_t memory_address;
        uint32_t memory_size;

        /// SINUCA Control Variables
        uint64_t opcode_number;
        uint64_t uop_number;
        package_state_t state;
        uint64_t ready_cycle;
        uint64_t born_cycle;

        // ====================================================================
        /// Methods
        // ====================================================================
        uop_package_t();
        uop_package_t(const uop_package_t &package);
        ~uop_package_t();

        std::string content_to_string();

        uop_package_t& operator=(const uop_package_t &package);
        bool operator==(const uop_package_t &package);

        void opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode);
        void package_clean();
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_stall(uint32_t wait_time);
        void package_ready(uint32_t wait_time);

        static int32_t find_free(uop_package_t *input_array, uint32_t size_array);

        static bool check_age(circular_buffer_t<uop_package_t> *input_array, uint32_t size_array);
        static bool check_age(uop_package_t *input_array, uint32_t size_array);
        static bool check_age(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);

        static std::string print_all(circular_buffer_t<uop_package_t> *input_array, uint32_t size_array);
        static std::string print_all(uop_package_t *input_array, uint32_t size_array);
        static std::string print_all(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
};
