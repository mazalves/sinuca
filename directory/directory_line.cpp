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
#include "../sinuca.hpp"
#include <string>

#ifdef DIRECTORY_CTRL_DEBUG
    #define DIRECTORY_CTRL_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define DIRECTORY_CTRL_DEBUG_PRINTF(...)
#endif

/// ============================================================================
directory_line_t::directory_line_t() {
    this->id_owner = 0;
    this->opcode_number = 0;
    this->opcode_address = 0;
    this->uop_number = 0;

    this->lock_type = LOCK_FREE;
    this->cache_request_order = utils_t::template_allocate_initialize_array<uint32_t>(sinuca_engine.cache_memory_array_size, 0);
    this->cache_requested = 0;

    this->initial_memory_operation = MEMORY_OPERATION_INST;
    this->initial_memory_address = 0;
    this->initial_memory_size = 0;

    this->born_cycle = sinuca_engine.get_global_cycle();
};

/// ============================================================================
directory_line_t::~directory_line_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint32_t>(cache_request_order);
};

/// ============================================================================
void directory_line_t::packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                                            lock_t lock_type,
                                            memory_operation_t initial_memory_operation, uint64_t initial_memory_address, uint32_t initial_memory_size) {
    this->id_owner = id_owner;
    this->opcode_number = opcode_number;
    this->opcode_address = opcode_address;
    this->uop_number = uop_number;

    this->lock_type = lock_type;

    this->initial_memory_operation = initial_memory_operation;
    this->initial_memory_address = initial_memory_address;
    this->initial_memory_size = initial_memory_size;

    this->born_cycle = sinuca_engine.get_global_cycle();
};

/// ============================================================================
std::string directory_line_t::directory_line_to_string() {
    std::string PackageString;
    PackageString = "";

    PackageString = PackageString + " DIR: Owner:" + utils_t::uint32_to_char(this->id_owner);
    PackageString = PackageString + " OPCode#:" + utils_t::uint64_to_char(this->opcode_number);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->opcode_address);
    PackageString = PackageString + " UOP#:" + utils_t::uint64_to_char(this->uop_number);

    PackageString = PackageString + " | LOCK:" + get_enum_lock_char(this->lock_type);

    PackageString = PackageString + " | " + get_enum_memory_operation_char(this->initial_memory_operation);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->initial_memory_address);
    PackageString = PackageString + " Size:" + utils_t::uint64_to_char(this->initial_memory_size);

    PackageString = PackageString + " | BORN:" + utils_t::uint64_to_char(this->born_cycle);

    PackageString = PackageString + " | RQST:" + utils_t::uint32_to_char(this->cache_requested);
    for (uint32_t i = 0; i < sinuca_engine.get_cache_memory_array_size(); i++) {
        PackageString = PackageString + "[" + utils_t::uint32_to_char(this->cache_request_order[i]) + "]";
    }
    return PackageString;
};

/// ============================================================================
bool directory_line_t::check_age(container_ptr_directory_line_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[0][i]->born_cycle < min_cycle) {
            return FAIL;
        }
    }
    return OK;
};
