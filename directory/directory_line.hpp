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
class directory_line_t {
    public:
        uint32_t id_owner;
        uint64_t opcode_number;
        uint64_t opcode_address;
        uint64_t uop_number;

        lock_t lock_type;

        uint32_t *cache_request_order;
        uint32_t cache_requested;

        memory_operation_t initial_memory_operation;
        uint64_t initial_memory_address;
        uint32_t initial_memory_size;

        uint64_t born_cycle;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        directory_line_t();
        ~directory_line_t();
        directory_line_t & operator=(const directory_line_t &line);

        void packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                        lock_t lock_type,
                        memory_operation_t initial_memory_operaton, uint64_t initial_memory_address, uint32_t initial_memory_size);
        std::string directory_line_to_string();
        static bool check_age(container_ptr_directory_line_t *input_array, uint32_t size_array);
};

