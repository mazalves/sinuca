//==============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
// Eduardo Henrique Molina da Cruz
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
#include <stdint.h>

/// ============================================================================
/// Memory Package
/// ============================================================================
 /*! Memory Package class to transport information about memory requests
  */
class memory_package_t {
    public:
        uint32_t id_owner;                      /// if (read / write) PROCESSOR.ID   else if (copy-back / prefetch) CACHE_MEMORY.ID
        uint64_t opcode_number;                 /// initial opcode number
        uint64_t opcode_address;                /// initial opcode address
        uint64_t uop_number;                    /// initial uop number (Instruction == 0)
        uint64_t memory_address;                /// memory address
        uint32_t memory_size;                   /// operation size after offset

        package_state_t state;                  /// package state
        uint64_t ready_cycle;                   /// package latency
        uint64_t born_cycle;                    /// package create time

        memory_operation_t memory_operation;    /// memory operation
        bool is_answer;                         /// is answer or request

        bool *sub_blocks;

        /// Router Control
        uint32_t id_src;                        /// id src component
        uint32_t id_dst;                        /// id dst component

        uint32_t *hops;                         /// route information
        int32_t hop_count;                     /// route information

        /// ====================================================================
        /// Methods
        /// ====================================================================
        memory_package_t();
        ~memory_package_t();

        std::string memory_to_string();

        memory_package_t & operator=(const memory_package_t &package);
        void package_clean();
        void package_set_src_dst(uint32_t id_src, uint32_t id_dst);
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_ready(uint32_t wait_time);
        void package_transmit(uint32_t stall_time);
        void packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                    uint64_t memory_address, uint32_t memory_size,
                    package_state_t state, uint32_t stall_time,
                    memory_operation_t memory_operation, bool is_answer,
                    uint32_t id_src, uint32_t id_dst, uint32_t *hops, uint32_t hop_count);

        static int32_t find_free(memory_package_t *input_array, uint32_t size_array);
        static uint32_t count_free(memory_package_t *input_array, uint32_t size_array);

        static void find_old_rqst_ans_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state, int32_t &position_rqst, int32_t &position_ans);
        static int32_t find_old_request_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state);
        static int32_t find_old_answer_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state);
        static int32_t find_state_mem_address(memory_package_t *input_array, uint32_t size_array, package_state_t state, uint64_t address);

        static std::string print(memory_package_t input_package);
        static std::string print_all(memory_package_t *input_array, uint32_t size_array);
        static std::string print_all(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);

        static bool check_age(memory_package_t *input_array, uint32_t size_array);
        static bool check_age(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);

        INSTANTIATE_GET_SET(uint32_t, id_owner);
};

