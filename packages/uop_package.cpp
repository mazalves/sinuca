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
#include "./../sinuca.hpp"
#include <string>

//==============================================================================
uop_package_t::uop_package_t() {
    this->package_clean();
};
//==============================================================================
uop_package_t::~uop_package_t(){
};

//==============================================================================
uop_package_t &uop_package_t::operator=(const uop_package_t &package) {
    /// TRACE Variables
    strncpy(this->opcode_assembly, package.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = package.opcode_operation;
    this->opcode_address = package.opcode_address;
    this->opcode_size = package.opcode_size;


    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->read_regs[i] = package.read_regs[i];
    }


    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->write_regs[i] = package.write_regs[i];
    }

    this->uop_operation = package.uop_operation;
    this->memory_address = package.memory_address;
    this->memory_size = package.memory_size;

    /// SINUCA Control Variables
    this->opcode_number = package.opcode_number;
    this->uop_number = package.uop_number;
    this->state = package.state;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
    return *this;
};

//==============================================================================
bool uop_package_t::operator==(const uop_package_t &package) {
    /// TRACE Variables
    if (strcmp(this->opcode_assembly, package.opcode_assembly) != 0) return FAIL;
    if (this->opcode_operation != package.opcode_operation) return FAIL;
    if (this->opcode_address != package.opcode_address) return FAIL;
    if (this->opcode_size != package.opcode_size) return FAIL;

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] != package.read_regs[i]) return FAIL;
    }

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] != package.write_regs[i]) return FAIL;
    }

    if (this->uop_operation != package.uop_operation) return FAIL;
    if (this->memory_address != package.memory_address) return FAIL;
    if (this->memory_size != package.memory_size) return FAIL;

    /// SINUCA Control Variables
    if (this->opcode_number != package.opcode_number) return FAIL;
    if (this->uop_number != package.uop_number) return FAIL;
    if (this->ready_cycle != package.ready_cycle) return FAIL;
    if (this->state != package.state) return FAIL;
    if (this->born_cycle != package.born_cycle) return FAIL;

    return OK;
};

//==============================================================================
void uop_package_t::opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode) {
    ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE,
                        "Trying to decode to uop in a non-free location\n");

    /// TRACE Variables
    strncpy(this->opcode_assembly, opcode.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = opcode.opcode_operation;
    this->opcode_address = opcode.opcode_address;
    this->opcode_size = opcode.opcode_size;

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->read_regs[i] = opcode.read_regs[i];
    }

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->write_regs[i] = opcode.write_regs[i];
    }

    this->uop_operation = uop_operation;
    this->memory_address = memory_address;
    this->memory_size = memory_size;

    /// SINUCA Control Variables
    this->opcode_number = opcode.opcode_number;
    this->uop_number = uop_number;
    this->state = PACKAGE_STATE_UNTREATED;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
};

//==============================================================================
void uop_package_t::package_clean() {
    /// TRACE Variables
    strcpy(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->read_regs[i] = POSITION_FAIL;
    }

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->write_regs[i] = POSITION_FAIL;
    }

    this->uop_operation = INSTRUCTION_OPERATION_NOP;
    this->memory_address = 0;
    this->memory_size = 0;

    /// SINUCA Control Variables
    this->opcode_number = 0;
    this->uop_number = 0;
    this->state = PACKAGE_STATE_FREE;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
};

//==============================================================================
void uop_package_t::package_ready(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_READY;
};


//==============================================================================
void uop_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_UNTREATED;
};


//==============================================================================
void uop_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_WAIT;
};


//==============================================================================
/// Convert Instruction variables into String
std::string uop_package_t::content_to_string() {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PACKAGE_STATE_FREE) {
            return PackageString;
        }
    #endif
    PackageString = PackageString + " UOP: OPCODE#" + utils_t::uint64_to_char(this->opcode_number);
    PackageString = PackageString + " | UOP#" + utils_t::uint64_to_char(this->uop_number);
    PackageString = PackageString + " | " + get_enum_instruction_operation_char(this->opcode_operation);
    PackageString = PackageString + " | 0x" + utils_t::uint64_to_char(this->opcode_address);
    PackageString = PackageString + " SIZE:" + utils_t::utils_t::uint32_to_char(this->opcode_size);

    PackageString = PackageString + " | " + get_enum_instruction_operation_char(this->uop_operation);
    PackageString = PackageString + " MEM:0x" + utils_t::uint64_to_char(this->memory_address);
    PackageString = PackageString + " SIZE:" + utils_t::uint32_to_char(this->memory_size);

    PackageString = PackageString + " | " + get_enum_package_state_char(this->state);
    PackageString = PackageString + " BORN:" + utils_t::uint64_to_char(this->born_cycle);
    PackageString = PackageString + " READY:" + utils_t::uint64_to_char(this->ready_cycle);

    PackageString = PackageString + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        PackageString = PackageString + " " + utils_t::uint32_to_string(this->read_regs[i]);
    }

    PackageString = PackageString + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        PackageString = PackageString + " " + utils_t::uint32_to_string(this->write_regs[i]);
    }
    PackageString = PackageString + " ]";

    return PackageString;
};

//==============================================================================
int32_t uop_package_t::find_free(uop_package_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == PACKAGE_STATE_FREE)
            return i;
    }
    return POSITION_FAIL;
};

//==============================================================================
bool uop_package_t::check_age(uop_package_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state != PACKAGE_STATE_FREE) {
            if (input_array[i].born_cycle < min_cycle) {
                WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_array[i].content_to_string().c_str())
                return FAIL;
            }
            /// Statistics of the oldest memory package
            if (sinuca_engine.get_global_cycle() - input_array[i].born_cycle > sinuca_engine.get_stat_old_uop_package()) {
                sinuca_engine.set_stat_old_uop_package(sinuca_engine.get_global_cycle() - input_array[i].born_cycle);
            }
        }
    }
    return OK;
};

//==============================================================================
bool uop_package_t::check_age(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            if (input_matrix[i][j].state != PACKAGE_STATE_FREE) {
                if (input_matrix[i][j].born_cycle < min_cycle) {
                    WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_matrix[i][j].content_to_string().c_str())
                    return FAIL;
                }
                /// Statistics of the oldest memory package
                if (sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle > sinuca_engine.get_stat_old_uop_package()) {
                    sinuca_engine.set_stat_old_uop_package(sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle);
                }
            }
        }
    }
    return OK;
};

//==============================================================================
std::string uop_package_t::print_all(uop_package_t *input_array, uint32_t size_array) {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_array ; i++) {
        PackageString = input_array[i].content_to_string();
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::utils_t::uint32_to_char(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};

//==============================================================================
std::string uop_package_t::print_all(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            PackageString = input_matrix[i][j].content_to_string();
            if (PackageString.size() > 1) {
                FinalString = FinalString +
                                "[" + utils_t::utils_t::uint32_to_char(i) + "] " +
                                "[" + utils_t::utils_t::uint32_to_char(j) + "] " + PackageString + "\n";
            }
        }
        if (FinalString.size() > 1) {
            FinalString = FinalString + "\n";
        }
    }
    return FinalString;
};
