/// ============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
// Eduardo Henrique Molina da Cruz
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
/// ============================================================================
/// Token for Token Controller (Inside interconnection_interface_t)
/// ============================================================================
class token_t {
    public:
        uint32_t id_owner;
        uint64_t opcode_number;
        uint64_t opcode_address;
        uint64_t uop_number;
        uint64_t memory_address;
        memory_operation_t memory_operation;
        bool is_comming;

        token_t(){
            this->id_owner = 0;
            this->opcode_number = 0;
            this->opcode_address = 0;
            this->uop_number = 0;
            this->memory_address = 0;
            this->memory_operation = MEMORY_OPERATION_INST;
            this->is_comming = false;
        };
};
