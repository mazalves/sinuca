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

#include "./../sinuca.hpp"
#include <string>

// =============================================================================
uop_package_t::uop_package_t() {
    this->package_clean();
};
// =============================================================================
uop_package_t::~uop_package_t(){
};

// =============================================================================
uop_package_t::uop_package_t(const uop_package_t& package) {
    /// TRACE Variables
    strncpy(opcode_assembly, package.opcode_assembly, sizeof(char) * MAX_ASSEMBLY_SIZE);
    opcode_operation = package.opcode_operation;
    opcode_address = package.opcode_address;
    opcode_size = package.opcode_size;

    memcpy(this->read_regs, package.read_regs, sizeof(this->read_regs));
    memcpy(this->write_regs, package.write_regs, sizeof(this->write_regs));

    uop_operation = package.uop_operation;
    memory_address = package.memory_address;
    memory_size = package.memory_size;

    is_mvx    = package.is_mvx   ;
    mvx_read1 = package.mvx_read1;
    mvx_read2 = package.mvx_read2;
    mvx_write = package.mvx_write;

    /// SINUCA Control Variables
    opcode_number = package.opcode_number;
    uop_number = package.uop_number;
    state = package.state;
    this->ready_cycle = package.ready_cycle;
    born_cycle = sinuca_engine.get_global_cycle();
};

// =============================================================================
uop_package_t& uop_package_t::operator=(const uop_package_t& package) {
    if (this != &package){
        memcpy(this, &package, sizeof(uop_package_t));
        this->born_cycle = sinuca_engine.get_global_cycle();
    }
    return *this;
};

// =============================================================================
bool uop_package_t::operator==(const uop_package_t &package) {
    /// TRACE Variables
    if (strcmp(this->opcode_assembly, package.opcode_assembly) != 0) return FAIL;
    if (this->opcode_operation != package.opcode_operation) return FAIL;
    if (this->opcode_address != package.opcode_address) return FAIL;
    if (this->opcode_size != package.opcode_size) return FAIL;

    if ( memcmp(this->read_regs, package.read_regs, sizeof(int32_t)*MAX_REGISTERS) != 0) return FAIL;
    if ( memcmp(this->write_regs, package.write_regs, sizeof(int32_t)*MAX_REGISTERS) != 0) return FAIL;

    if (this->uop_operation != package.uop_operation) return FAIL;
    if (this->memory_address != package.memory_address) return FAIL;
    if (this->memory_size != package.memory_size) return FAIL;

    // MVX
    if (this->is_mvx    != package.is_mvx   ) return FAIL;
    if (this->mvx_read1 != package.mvx_read1) return FAIL;
    if (this->mvx_read2 != package.mvx_read2) return FAIL;
    if (this->mvx_write != package.mvx_write) return FAIL;

    /// SINUCA Control Variables
    if (this->opcode_number != package.opcode_number) return FAIL;
    if (this->uop_number != package.uop_number) return FAIL;
    if (this->ready_cycle != package.ready_cycle) return FAIL;
    if (this->state != package.state) return FAIL;
    if (this->born_cycle != package.born_cycle) return FAIL;

    return OK;
};

// =============================================================================
void uop_package_t::opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode) {
    ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE,
                        "Trying to decode to uop in a non-free location\n");

    /// TRACE Variables
    strncpy(this->opcode_assembly, opcode.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = opcode.opcode_operation;
    this->opcode_address = opcode.opcode_address;
    this->opcode_size = opcode.opcode_size;

    memcpy(this->read_regs, opcode.read_regs, sizeof(int32_t) * MAX_REGISTERS);
    memcpy(this->write_regs, opcode.write_regs, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = uop_operation;
    this->memory_address = memory_address;
    this->memory_size = memory_size;

    // MVX
    this->is_mvx    = is_mvx   ;
    this->mvx_read1 = mvx_read1;
    this->mvx_read2 = mvx_read2;
    this->mvx_write = mvx_write;

    /// SINUCA Control Variables
    this->opcode_number = opcode.opcode_number;
    this->uop_number = uop_number;
    this->state = PACKAGE_STATE_UNTREATED;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
};

// =============================================================================
void uop_package_t::package_clean() {
    /// TRACE Variables
    strcpy(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = INSTRUCTION_OPERATION_NOP;
    this->memory_address = 0;
    this->memory_size = 0;

    this->is_mvx    = false;
    this->mvx_read1 = -1;
    this->mvx_read2 = -1;
    this->mvx_write = -1;

    /// SINUCA Control Variables
    this->opcode_number = 0;
    this->uop_number = 0;
    this->state = PACKAGE_STATE_FREE;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
};

// =============================================================================
void uop_package_t::package_ready(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_READY;
};


// =============================================================================
void uop_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_UNTREATED;
};


// =============================================================================
void uop_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_WAIT;
};


// =============================================================================
/// Convert Instruction variables into String
std::string uop_package_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif
    content_string = content_string + " UOP: OpCode#" + utils_t::uint64_to_string(this->opcode_number);
    content_string = content_string + " UOP#" + utils_t::uint64_to_string(this->uop_number);
    content_string = content_string + " " + get_enum_instruction_operation_char(this->opcode_operation);
    content_string = content_string + " $" + utils_t::big_uint64_to_string(this->opcode_address);
    content_string = content_string + " Size:" + utils_t::utils_t::uint32_to_string(this->opcode_size);

    content_string = content_string + " | " + get_enum_instruction_operation_char(this->uop_operation);
    content_string = content_string + " MEM:$" + utils_t::big_uint64_to_string(this->memory_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->memory_size);

    // MVX
    if (this->is_mvx == true) {
        content_string = content_string + " | is_mvx";
        content_string = content_string + " R1:" + utils_t::int32_to_string(this->mvx_read1);
        content_string = content_string + " R2:" + utils_t::int32_to_string(this->mvx_read2);
        content_string = content_string + " W:" + utils_t::int32_to_string(this->mvx_write);
    }
    else {
        content_string = content_string + " | not_mvx";
    }


    content_string = content_string + " | " + get_enum_package_state_char(this->state);
    content_string = content_string + " | Ready:" + utils_t::uint64_to_string(this->ready_cycle);
    content_string = content_string + " Born:" + utils_t::uint64_to_string(this->born_cycle);


    content_string = content_string + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->read_regs[i]);
        }
    }

    content_string = content_string + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->write_regs[i]);
        }
    }
    content_string = content_string + " ]";

    return content_string;
};

// =============================================================================
int32_t uop_package_t::find_free(uop_package_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == PACKAGE_STATE_FREE)
            return i;
    }
    return POSITION_FAIL;
};


// =============================================================================
bool uop_package_t::check_age(circular_buffer_t<uop_package_t> *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[0][i].state != PACKAGE_STATE_FREE) {
            if (input_array[0][i].born_cycle < min_cycle) {
                WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_array[0][i].content_to_string().c_str())
                return FAIL;
            }
            /// Statistics of the oldest memory package
            if (sinuca_engine.get_global_cycle() - input_array[0][i].born_cycle > sinuca_engine.get_stat_old_uop_package()) {
                sinuca_engine.set_stat_old_uop_package(sinuca_engine.get_global_cycle() - input_array[0][i].born_cycle);
            }
        }
    }
    return OK;
};

// =============================================================================
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

// =============================================================================
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


// =============================================================================
std::string uop_package_t::print_all(circular_buffer_t<uop_package_t> *input_array, uint32_t size_array) {
    std::string content_string;
    std::string FinalString;
    content_string = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_array ; i++) {
        content_string = input_array[0][i].content_to_string();
        if (content_string.size() > 1) {
            FinalString = FinalString + "[" + utils_t::utils_t::uint32_to_string(i) + "] " + content_string + "\n";
        }
    }
    return FinalString;
};

// =============================================================================
std::string uop_package_t::print_all(uop_package_t *input_array, uint32_t size_array) {
    std::string content_string;
    std::string FinalString;
    content_string = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_array ; i++) {
        content_string = input_array[i].content_to_string();
        if (content_string.size() > 1) {
            FinalString = FinalString + "[" + utils_t::utils_t::uint32_to_string(i) + "] " + content_string + "\n";
        }
    }
    return FinalString;
};

// =============================================================================
std::string uop_package_t::print_all(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    std::string content_string;
    std::string FinalString;
    content_string = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            content_string = input_matrix[i][j].content_to_string();
            if (content_string.size() > 1) {
                FinalString = FinalString +
                                "[" + utils_t::utils_t::uint32_to_string(i) + "] " +
                                "[" + utils_t::utils_t::uint32_to_string(j) + "] " + content_string + "\n";
            }
        }
        if (FinalString.size() > 1) {
            FinalString = FinalString + "\n";
        }
    }
    return FinalString;
};
