//==============================================================================
//
// Copyright (C) 2010, 2011
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
#include "./sinuca.hpp"
#include <string>

//==============================================================================
memory_package_t &memory_package_t::operator=(const memory_package_t &package) {

    this->id_owner = package.id_owner;
    this->opcode_number = package.opcode_number;
    this->opcode_address = package.opcode_address;
    this->uop_number = package.uop_number;
    this->memory_address = package.memory_address;
    this->memory_size = package.memory_size;

    this->state = package.state;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();

    this->memory_operation = package.memory_operation;
    this->is_answer = package.is_answer;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->sub_blocks[i] = package.sub_blocks[i];
    }

    /// Routing Control
    this->id_src = package.id_src;
    this->id_dst = package.id_dst;
    this->hops = package.hops;
    this->hop_count = package.hop_count;

    return *this;
};

//==============================================================================
void memory_package_t::package_clean() {

    this->id_owner = 0;
    this->opcode_number = 0;
    this->opcode_address = 0;
    this->uop_number = 0;
    this->memory_address = 0;
    this->memory_size = 0;

    this->state = PACKAGE_STATE_FREE;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();

    this->memory_operation = MEMORY_OPERATION_INST;
    this->is_answer = false;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->sub_blocks[i] = 0;
    }

    /// Routing Control
    this->id_src = 0;
    this->id_dst = 0;
    this->hops = NULL;
    this->hop_count = 0;
};

//==============================================================================
void memory_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_UNTREATED;
};

//==============================================================================
void memory_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_WAIT;
};

//==============================================================================
void memory_package_t::package_ready(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_READY;
};

//==============================================================================
void memory_package_t::package_transmit(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_TRANSMIT;
};

//==============================================================================
void memory_package_t::packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                                uint64_t memory_address, uint32_t memory_size,
                                package_state_t state, uint32_t stall_time,
                                memory_operation_t memory_operation, bool is_answer,
                                uint32_t id_src, uint32_t id_dst, uint32_t *hops, uint32_t hop_count) {

    ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE, "Wrong package state.\n");
    ERROR_ASSERT_PRINTF(stall_time < MAX_ALIVE_TIME, "Stall time should be less than MAX_ALIVE_TIME.\n");
    ERROR_ASSERT_PRINTF(memory_size > 0, "Memory size should be bigger than 0.\n");

    this->id_owner = id_owner;
    this->opcode_number = opcode_number;
    this->opcode_address = opcode_address;
    this->uop_number = uop_number;

    this->memory_address = memory_address;
    this->memory_size = memory_size;

    this->state = state;
    this->ready_cycle = stall_time + sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();

    this->memory_operation = memory_operation;
    this->is_answer = is_answer;

    /// Routing Control
    this->id_src = id_src;
    this->id_dst = id_dst;
    this->hops = hops;
    this->hop_count = hop_count;
};

//==============================================================================
std::string memory_package_t::memory_to_string() {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PACKAGE_STATE_FREE) {
            return PackageString;
        }
    #endif
    PackageString = PackageString + " MEM: Owner:" + utils_t::uint32_to_char(this->id_owner);
    PackageString = PackageString + " OPCode#" + utils_t::uint64_to_char(this->opcode_number);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->opcode_address);
    PackageString = PackageString + " UOP#" + utils_t::uint64_to_char(this->uop_number);

    PackageString = PackageString + " | " + get_enum_memory_operation_char(this->memory_operation);
    if (this->is_answer) {
        PackageString = PackageString + " ANS ";
    }
    else {
        PackageString = PackageString + " RQST";
    }

    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->memory_address);
    PackageString = PackageString + " Size:" + utils_t::uint32_to_char(this->memory_size);

    PackageString = PackageString + " | " + get_enum_package_state_char(this->state);
    PackageString = PackageString + " Ready:" + utils_t::uint64_to_char(this->ready_cycle);
    PackageString = PackageString + " Born:" + utils_t::uint64_to_char(this->born_cycle);


    PackageString = PackageString + " | SRC:" + utils_t::uint32_to_char(this->id_src);
    PackageString = PackageString + " => DST:" + utils_t::uint32_to_char(this->id_dst);
    PackageString = PackageString + " HOPS:" + utils_t::int32_to_char(this->hop_count);

    if (this->hops != NULL) {
        for (int32_t i = 0; i <= this->hop_count; i++) {
            PackageString = PackageString + " [" + utils_t::uint32_to_char(this->hops[i]) + "]";
        }
    }
    return PackageString;
};

//==============================================================================
//
//  From
//
//==============================================================================
int32_t memory_package_t::find_free(memory_package_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == PACKAGE_STATE_FREE)
            return i;
    }
    return POSITION_FAIL;
};


//==============================================================================
void memory_package_t::find_old_rqst_ans_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state, int32_t &position_rqst, int32_t &position_ans) {
    position_rqst = POSITION_FAIL;
    position_ans = POSITION_FAIL;

    uint64_t old_rqst_cycle = sinuca_engine.get_global_cycle() + 1;
    uint64_t old_ans_cycle = sinuca_engine.get_global_cycle() + 1;

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == state &&
        input_array[i].ready_cycle <= sinuca_engine.get_global_cycle()) {

            if (input_array[i].is_answer == false) {
                if (old_rqst_cycle > input_array[i].born_cycle) {
                    old_rqst_cycle = input_array[i].born_cycle;
                    position_rqst = i;
                }
            }
            else {
                if (old_ans_cycle > input_array[i].born_cycle) {
                    old_ans_cycle = input_array[i].born_cycle;
                    position_ans = i;
                }
            }
        }
    }
};


//==============================================================================
int32_t memory_package_t::find_old_request_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    uint64_t old_cycle = sinuca_engine.get_global_cycle() + 1;

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == state &&
        input_array[i].is_answer == false &&
        old_cycle > input_array[i].born_cycle &&
        input_array[i].ready_cycle <= sinuca_engine.get_global_cycle()) {
            old_cycle = input_array[i].born_cycle;
            old_pos = i;
        }
    }
    return old_pos;
};


//==============================================================================
int32_t memory_package_t::find_old_answer_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    uint64_t old_cycle = sinuca_engine.get_global_cycle() + 1;

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == state &&
        input_array[i].is_answer == true &&
        old_cycle > input_array[i].born_cycle &&
        input_array[i].ready_cycle <= sinuca_engine.get_global_cycle()) {
            old_cycle = input_array[i].born_cycle;
            old_pos = i;
        }
    }
    return old_pos;
};

//==============================================================================
int32_t memory_package_t::find_state_mem_address(memory_package_t *input_array, uint32_t size_array, package_state_t state, uint64_t address) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == state &&
        input_array[i].ready_cycle <= sinuca_engine.get_global_cycle() &&
        input_array[i].memory_address == address) {
            return i;
        }
    }
    return POSITION_FAIL;
};


//==============================================================================
std::string memory_package_t::print_all(memory_package_t *input_array, uint32_t size_array) {
    std::string PackageString;
    std::string FinalString;

    FinalString = "";
    for (uint32_t i = 0; i < size_array ; i++) {
        PackageString = "";
        PackageString = input_array[i].memory_to_string();
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::uint32_to_string(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};

//==============================================================================
std::string memory_package_t::print_all(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    std::string PackageString;
    std::string ColumnString;
    std::string FinalString;

    FinalString = "";
    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        ColumnString = "";
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            PackageString = "";
            PackageString = input_matrix[i][j].memory_to_string();
            if (PackageString.size() > 1) {
                ColumnString = ColumnString +
                                "[" + utils_t::uint32_to_string(i) + "]" +
                                "[" + utils_t::uint32_to_string(j) + "] " + PackageString + "\n";
            }
        }
        if (ColumnString.size() > 1) {
            FinalString = FinalString + ColumnString + "\n";
        }
    }
    return FinalString;
};


//==============================================================================
bool memory_package_t::check_age(memory_package_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state != PACKAGE_STATE_FREE) {
            if (input_array[i].born_cycle < min_cycle) {
                WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_array[i].memory_to_string().c_str())
                return FAIL;
            }
            /// Statistics of the oldest memory package
            if (sinuca_engine.get_global_cycle() - input_array[i].born_cycle > sinuca_engine.get_stat_old_memory_package()) {
                sinuca_engine.set_stat_old_memory_package(sinuca_engine.get_global_cycle() - input_array[i].born_cycle);
            }
        }
    }
    return OK;
};

//==============================================================================
bool memory_package_t::check_age(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            if (input_matrix[i][j].state != PACKAGE_STATE_FREE) {
                if (input_matrix[i][j].born_cycle < min_cycle) {
                    WARNING_PRINTF("CHECK AGE FAIL: %s\n", input_matrix[i][j].memory_to_string().c_str())
                    return FAIL;
                }
                /// Statistics of the oldest memory package
                if (sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle > sinuca_engine.get_stat_old_memory_package()) {
                    sinuca_engine.set_stat_old_memory_package(sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle);
                }
            }
        }
    }
    return OK;
};
