//==============================================================================
//
// Copyright (C) 2010, 2011, 2012
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
//==============================================================================

/// ============================================================================
/// Opcode Package (High Level Instruction)
/// ============================================================================

class opcode_package_t {
    public:
        /// TRACE Variables
        char opcode_assembly[20];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        int32_t read_regs[MAX_REGISTERS];
        int32_t write_regs[MAX_REGISTERS];

        uint32_t base_reg;
        uint32_t index_reg;

        bool is_read;
        uint64_t read_address;
        uint32_t read_size;

        bool is_read2;
        uint64_t read2_address;
        uint32_t read2_size;

        bool is_write;
        uint64_t write_address;
        uint32_t write_size;

        bool is_branch;
        bool is_predicated;
        bool is_prefetch;

        /// SINUCA Control Variables
        uint64_t opcode_number;
        package_state_t state;
        uint64_t ready_cycle;
        uint64_t born_cycle;
        sync_t sync_type;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        opcode_package_t();
        ~opcode_package_t();

        std::string content_to_string();
        void opcode_to_trace_char(char *trace_line);

        void opcode_to_trace_string(char *trace_string);

        void trace_string_to_opcode(char * input_string);
        void trace_string_to_read(char *input_string, uint32_t actual_bbl);
        void trace_string_to_read2(char *input_string, uint32_t actual_bbl);
        void trace_string_to_write(char *input_string, uint32_t actual_bbl);

        opcode_package_t & operator=(const opcode_package_t &package);
        bool operator==(const opcode_package_t &package);

        void package_clean();
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_stall(uint32_t wait_time);
        void package_ready(uint32_t wait_time);

        static int32_t find_free(opcode_package_t *input_array, uint32_t size_array);
        static int32_t find_opcode_number(opcode_package_t *input_array, uint32_t size_array, uint64_t opcode_number);

        static bool check_age(opcode_package_t *input_array, uint32_t size_array);
        static bool check_age(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
        static std::string print_all(opcode_package_t *input_array, uint32_t size_array);
        static std::string print_all(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
};
