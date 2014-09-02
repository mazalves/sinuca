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
opcode_package_t::opcode_package_t() {
    this->package_clean();
};
// =============================================================================
opcode_package_t::~opcode_package_t(){
};

// =============================================================================
opcode_package_t::opcode_package_t(const opcode_package_t &package) {

    /// TRACE Variables
    strncpy(opcode_assembly, package.opcode_assembly, sizeof(char) * MAX_ASSEMBLY_SIZE);
    opcode_operation = package.opcode_operation;
    opcode_address = package.opcode_address;
    opcode_size = package.opcode_size;

    memcpy(read_regs, package.read_regs, sizeof(int32_t) * MAX_REGISTERS);
    memcpy(write_regs, package.write_regs, sizeof(int32_t) * MAX_REGISTERS);

    base_reg = package.base_reg;
    index_reg = package.index_reg;

    /// Flags
    is_read = package.is_read;
    read_address = package.read_address;
    read_size = package.read_size;

    is_read2 = package.is_read2;
    read2_address = package.read2_address;
    read2_size = package.read2_size;

    is_write = package.is_write;
    write_address = package.write_address;
    write_size = package.write_size;

    is_conditional = package.is_conditional;
    is_predicated = package.is_predicated;
    is_prefetch = package.is_prefetch;

    /// SINUCA Control Variables
    opcode_number = package.opcode_number;
    state = package.state;
    ready_cycle = package.ready_cycle;
    // ~ ready_cycle = sinuca_engine.get_global_cycle();
    born_cycle = sinuca_engine.get_global_cycle();
    sync_type = package.sync_type;
};

// =============================================================================
opcode_package_t &opcode_package_t::operator=(const opcode_package_t &package) {
    if (this != &package){
        memcpy(this, &package, sizeof(opcode_package_t));
        this->born_cycle = sinuca_engine.get_global_cycle();
    }
    return *this;

// ~
    // ~ /// TRACE Variables
    // ~ if (this!=&package){
        // ~ strncpy(this->opcode_assembly, package.opcode_assembly, sizeof(char) * MAX_ASSEMBLY_SIZE);
        // ~ this->opcode_operation = package.opcode_operation;
        // ~ this->opcode_address = package.opcode_address;
        // ~ this->opcode_size = package.opcode_size;
// ~
        // ~ memcpy(this->read_regs, package.read_regs, sizeof(int32_t) * MAX_REGISTERS);
        // ~ memcpy(this->write_regs, package.write_regs, sizeof(int32_t) * MAX_REGISTERS);
// ~
        // ~ this->base_reg = package.base_reg;
        // ~ this->index_reg = package.index_reg;
// ~
        // ~ /// Flags
        // ~ this->is_read = package.is_read;
        // ~ this->read_address = package.read_address;
        // ~ this->read_size = package.read_size;
// ~
        // ~ this->is_read2 = package.is_read2;
        // ~ this->read2_address = package.read2_address;
        // ~ this->read2_size = package.read2_size;
// ~
        // ~ this->is_write = package.is_write;
        // ~ this->write_address = package.write_address;
        // ~ this->write_size = package.write_size;
// ~
        // ~ this->is_conditional = package.is_conditional;
        // ~ this->is_predicated = package.is_predicated;
        // ~ this->is_prefetch = package.is_prefetch;
// ~
        // ~ /// SINUCA Control Variables
        // ~ this->opcode_number = package.opcode_number;
        // ~ this->state = package.state;
        // ~ this->ready_cycle = package.ready_cycle;
        // ~ this->born_cycle = sinuca_engine.get_global_cycle();
        // ~ this->sync_type = package.sync_type;
    // ~ }
    // ~ return *this;
};

// =============================================================================
bool opcode_package_t::operator==(const opcode_package_t &package) {
    /// TRACE Variables
    if (strncmp(this->opcode_assembly, package.opcode_assembly, sizeof(this->opcode_assembly)) != 0) return FAIL;
    if (this->opcode_operation != package.opcode_operation) return FAIL;
    if (this->opcode_address != package.opcode_address) return FAIL;
    if (this->opcode_size != package.opcode_size) return FAIL;

    if ( memcmp(this->read_regs, package.read_regs, sizeof(int32_t) * MAX_REGISTERS) != 0) return FAIL;
    if ( memcmp(this->write_regs, package.write_regs, sizeof(int32_t) * MAX_REGISTERS) != 0) return FAIL;

    if (this->base_reg != package.base_reg) return FAIL;
    if (this->index_reg != package.index_reg) return FAIL;

    if (this->is_read != package.is_read) return FAIL;
    if (this->read_address != package.read_address) return FAIL;
    if (this->read_size != package.read_size) return FAIL;

    if (this->is_read2 != package.is_read2) return FAIL;
    if (this->read2_address != package.read2_address) return FAIL;
    if (this->read2_size != package.read2_size) return FAIL;

    if (this->is_write != package.is_write) return FAIL;
    if (this->write_address != package.write_address) return FAIL;
    if (this->write_size != package.write_size) return FAIL;

    if (this->is_conditional != package.is_conditional) return FAIL;
    if (this->is_predicated != package.is_predicated) return FAIL;
    if (this->is_prefetch != package.is_prefetch) return FAIL;

    /// SINUCA Control Variables
    if (this->opcode_number != package.opcode_number) return FAIL;
    if (this->ready_cycle != package.ready_cycle) return FAIL;
    if (this->state != package.state) return FAIL;
    if (this->born_cycle != package.born_cycle) return FAIL;
    if (this->sync_type != package.sync_type) return FAIL;

    return OK;
};

// =============================================================================
void opcode_package_t::package_clean() {
    /// TRACE Variables
    sprintf(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    this->base_reg = 0;
    this->index_reg = 0;

    this->is_read = false;
    this->read_address = 0;
    this->read_size = 0;

    this->is_read2 = false;
    this->read2_address = 0;
    this->read2_size = 0;

    this->is_write = false;
    this->write_address = 0;
    this->write_size = 0;

    this->is_conditional = false;
    this->is_predicated = false;
    this->is_prefetch = false;

    /// SINUCA Control Variables
    this->opcode_number = 0;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_FREE;
    this->born_cycle = sinuca_engine.get_global_cycle();
    this->sync_type = SYNC_FREE;
};

// =============================================================================
void opcode_package_t::package_ready(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_READY;
};


// =============================================================================
void opcode_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_UNTREATED;
};


// =============================================================================
void opcode_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_WAIT;
};


// =============================================================================
/// Convert Instruction variables into String
std::string opcode_package_t::content_to_string() {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PACKAGE_STATE_FREE) {
            return PackageString;
        }
    #endif
    PackageString = PackageString + " OpCode#" + utils_t::uint64_to_string(this->opcode_number);
    PackageString = PackageString + " " + get_enum_instruction_operation_char(this->opcode_operation);
    PackageString = PackageString + " 0x" + utils_t::big_uint64_to_string(this->opcode_address);
    PackageString = PackageString + " Size:" + utils_t::uint32_to_string(this->opcode_size);

    PackageString = PackageString + " | R1 0x" + utils_t::big_uint64_to_string(this->read_address);
    PackageString = PackageString + " Size:" + utils_t::uint32_to_string(this->read_size);

    PackageString = PackageString + " | R2 0x" + utils_t::big_uint64_to_string(this->read2_address);
    PackageString = PackageString + " Size:" + utils_t::uint32_to_string(this->read2_size);

    PackageString = PackageString + " | W 0x" + utils_t::big_uint64_to_string(this->write_address);
    PackageString = PackageString + " Size:" + utils_t::uint32_to_string(this->write_size);

    PackageString = PackageString + " | " + get_enum_package_state_char(this->state);
    PackageString = PackageString + " Ready:" + utils_t::uint64_to_string(this->ready_cycle);
    PackageString = PackageString + " Born:" + utils_t::uint64_to_string(this->born_cycle);


    PackageString = PackageString + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            PackageString = PackageString + " " + utils_t::uint32_to_string(this->read_regs[i]);
        }
    }

    PackageString = PackageString + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            PackageString = PackageString + " " + utils_t::uint32_to_string(this->write_regs[i]);
        }
    }
    PackageString = PackageString + " ]";

    if (this->is_conditional == true)
        PackageString = PackageString + " | is_condt";
    else
        PackageString = PackageString + " | not_cond";

    return PackageString;
};


// ============================================================================= NEW
// Instruction Class
// Convert Instruction variables into String
void opcode_package_t::opcode_to_trace_string(char *trace_string) {
    char register_string[TRACE_LINE_SIZE];
    uint32_t reg_count;

    trace_string[0] = '\0';
    sprintf(trace_string, "%s", this->opcode_assembly);
    sprintf(trace_string, "%s %"PRId32"", trace_string, this->opcode_operation);
    sprintf(trace_string, "%s 0x%"PRId64"", trace_string, this->opcode_address);
    sprintf(trace_string, "%s %"PRId32"", trace_string, this->opcode_size);

    register_string[0] = '\0';
    reg_count = 0;
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            reg_count++;
            sprintf(register_string, "%s %"PRId32"", register_string, this->read_regs[i]);
        }
    }
    sprintf(trace_string, "%s %"PRId32"", trace_string, reg_count);
    sprintf(trace_string, "%s%s", trace_string, register_string);


    register_string[0] = '\0';
    reg_count = 0;
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            reg_count++;
            sprintf(register_string, "%s %"PRId32"", register_string, this->write_regs[i]);
        }
    }
    sprintf(trace_string, "%s %"PRId32"", trace_string, reg_count);
    sprintf(trace_string, "%s%s", trace_string, register_string);


    sprintf(trace_string, "%s %"PRId32"", trace_string, this->base_reg);
    sprintf(trace_string, "%s %"PRId32"", trace_string, this->index_reg);


    register_string[0] = '\0';
    if (this->is_read == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    if (this->is_read2 == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    if (this->is_write == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    if (this->is_conditional == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    if (this->is_predicated == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    if (this->is_prefetch == true)
        strcat(register_string, " 1");
    else
        strcat(register_string, " 0");

    sprintf(trace_string, "%s%s\n", trace_string, register_string);

};


// ============================================================================= NEW
/// Convert Dynamic Memory Trace line into Instruction Memory Operands
/// Field N.:   01 |  02  |      03
///     Ex:     W 8 0x140735291283448
///             W 8 0x140735291283440
///             W 8 0x140735291283432
void opcode_package_t::trace_string_to_read(char *input_string, uint32_t actual_bbl) {
    char *sub_string = NULL;
    char *tmp_ptr = NULL;
    uint32_t count = 0, i = 0;
    while (input_string[i] != '\0') {
        count += (input_string[i] == ' ');
        i++;
    }
    ERROR_ASSERT_PRINTF(count != 4, "Error converting Text to Memory (Wrong  number of fields %d)\n", count)

    sub_string = strtok_r(input_string, " ", &tmp_ptr);
    ERROR_ASSERT_PRINTF(strcmp(sub_string, "R") == 0, "MemoryTraceFile Type (R) expected.\n Inst: %s\n Mem:%s\n",
                                                                                this->content_to_string().c_str(), input_string)

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->read_size = strtoull(sub_string, NULL, 10);
    this->read_size = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->read_address = strtoull(sub_string, NULL, 10);
    this->read_address = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
    ERROR_ASSERT_PRINTF((uint32_t)utils_t::string_to_uint64(sub_string) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
};


// ============================================================================= NEW
void opcode_package_t::trace_string_to_read2(char * input_string, uint32_t actual_bbl) {
    char *sub_string = NULL;
    char *tmp_ptr = NULL;
    uint32_t count = 0, i = 0;
    while (input_string[i] != '\0') {
        count += (input_string[i] == ' ');
        i++;
    }
    ERROR_ASSERT_PRINTF(count != 4, "Error converting Text to Memory (Wrong  number of fields %d)\n", count)

    sub_string = strtok_r (input_string, " ", &tmp_ptr);
    ERROR_ASSERT_PRINTF(strcmp(sub_string, "R") == 0, "MemoryTraceFile Type (R) expected.\n Inst: %s\n Mem:%s\n",
                                                                                this->content_to_string().c_str(), input_string)

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->read2_size = strtoull(sub_string, NULL, 10);
    this->read2_size = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->read2_address = strtoull(sub_string, NULL, 10);
    this->read2_address = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
    ERROR_ASSERT_PRINTF((uint32_t)utils_t::string_to_uint64(sub_string) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
};


// ============================================================================= NEW
void opcode_package_t::trace_string_to_write(char *input_string, uint32_t actual_bbl) {
    char *sub_string = NULL;
    char *tmp_ptr = NULL;
    uint32_t count = 0, i = 0;
    while (input_string[i] != '\0') {
        count += (input_string[i] == ' ');
        i++;
    }
    ERROR_ASSERT_PRINTF(count != 4, "Error converting Text to Memory (Wrong  number of fields %d)\n", count)

    sub_string = strtok_r(input_string, " ", &tmp_ptr);
    ERROR_ASSERT_PRINTF(strcmp(sub_string, "W") == 0, "MemoryTraceFile Type (W) expected.\n Inst: %s\n Mem:%s\n",
                                                                                this->content_to_string().c_str(), input_string)

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->write_size = strtoull(sub_string, NULL, 10);
    this->write_size = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ this->write_address = strtoull(sub_string, NULL, 10);
    this->write_address = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    // ~ ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
    ERROR_ASSERT_PRINTF((uint32_t)utils_t::string_to_uint64(sub_string) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%s)\n", actual_bbl, sub_string)
};


// ============================================================================= NEW
/// Convert Static Trace line into Instruction
/// Field N.:   01            |   02                | 03           | 04        |   05      |    06   |   07        |     08        |     09
/// Type:    opcode_operation | instruction_address | Inst. Size   | N.Rregs   | read_regs | N.Wregs | write_regs  | IsPredicated  | IsPrefetch
/// Static File Example:
/// #
/// # Compressed Trace Generated By Pin to SiNUCA
/// #
/// @1
/// 1 0x140647360289520 3 1 15 1 12 0 0
/// 9 0x140647360289523 5 2 35 15 2 35 15 0 0
/// @2
/// 9 0x140647360305456 1 2 14 15 1 15 0 0
/// 1 0x140647360305457 3 1 15 1 14 0 0
/// 9 0x140647360305460 2 2 27 15 1 15 0 0
/// 9 0x140647360305462 2 2 26 15 1 15 0 0
/// 9 0x140647360305464 2 2 25 15 1 15 0 0
///
void opcode_package_t::trace_string_to_opcode(char *input_string) {
    char *sub_string = NULL;
    char *tmp_ptr = NULL;
    uint32_t sub_fields, count, i;
    count = 0;

    for (i = 0; input_string[i] != '\0'; i++) {
        count += (input_string[i] == ' ');
    }
    ERROR_ASSERT_PRINTF(count >= 13, "Error converting Text to Instruction (Wrong  number of fields %d), input_string = %s\n", count, input_string)

    sub_string = strtok_r(input_string, " ", &tmp_ptr);
    strcpy(this->opcode_assembly, sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->opcode_operation = instruction_operation_t(utils_t::string_to_uint64(sub_string));

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    ERROR_ASSERT_PRINTF(memcmp(sub_string, "0x", 2) == 0, "Error converting Text to Instruction (Wrong number of fields), input_string = %s\n", sub_string)
    this->opcode_address = utils_t::string_to_uint64(sub_string + 2);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->opcode_size = utils_t::string_to_uint64(sub_string);

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    sub_fields = utils_t::string_to_uint64(sub_string);

    for (i = 0; i < sub_fields; i++) {
        sub_string = strtok_r(NULL, " ", &tmp_ptr);
        this->read_regs[i] = utils_t::string_to_uint64(sub_string);
    }

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    sub_fields = utils_t::string_to_uint64(sub_string);

    for (i = 0; i < sub_fields; i++) {
        sub_string = strtok_r(NULL, " ", &tmp_ptr);
        this->write_regs[i] = utils_t::string_to_uint64(sub_string);
    }

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->base_reg = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->index_reg = utils_t::string_to_uint64(sub_string);

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_read = (sub_string[0] == '1');

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_read2 = (sub_string[0] == '1');

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_write = (sub_string[0] == '1');

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_conditional = (sub_string[0] == '1');

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_predicated = (sub_string[0] == '1');

    sub_string = strtok_r(NULL, " ", &tmp_ptr);
    this->is_prefetch = (sub_string[0] == '1');

};


// =============================================================================
int32_t opcode_package_t::find_free(opcode_package_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == PACKAGE_STATE_FREE)
            return i;
    }
    return POSITION_FAIL;
};


// =============================================================================
int32_t opcode_package_t::find_opcode_number(opcode_package_t *input_array, uint32_t size_array, uint64_t opcode_number) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].opcode_number == opcode_number) {
            return i;
        }
    }
    return POSITION_FAIL;
};

// =============================================================================
bool opcode_package_t::check_age(circular_buffer_t<opcode_package_t> *input_array, uint32_t size_array) {
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
            if (sinuca_engine.get_global_cycle() - input_array[0][i].born_cycle > sinuca_engine.get_stat_old_opcode_package()) {
                sinuca_engine.set_stat_old_opcode_package(sinuca_engine.get_global_cycle() - input_array[0][i].born_cycle);
            }
        }
    }
    return OK;
};

// =============================================================================
bool opcode_package_t::check_age(opcode_package_t *input_array, uint32_t size_array) {
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
            if (sinuca_engine.get_global_cycle() - input_array[i].born_cycle > sinuca_engine.get_stat_old_opcode_package()) {
                sinuca_engine.set_stat_old_opcode_package(sinuca_engine.get_global_cycle() - input_array[i].born_cycle);
            }
        }
    }
    return OK;
};

// =============================================================================
bool opcode_package_t::check_age(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
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
                if (sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle > sinuca_engine.get_stat_old_opcode_package()) {
                    sinuca_engine.set_stat_old_opcode_package(sinuca_engine.get_global_cycle() - input_matrix[i][j].born_cycle);
                }
            }
        }
    }
    return OK;
};

// =============================================================================
std::string opcode_package_t::print_all(circular_buffer_t<opcode_package_t> *input_array, uint32_t size_array) {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_array ; i++) {
        PackageString = input_array[0][i].content_to_string();
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::uint32_to_string(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};

// =============================================================================
std::string opcode_package_t::print_all(opcode_package_t *input_array, uint32_t size_array) {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_array ; i++) {
        PackageString = input_array[i].content_to_string();
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::uint32_to_string(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};

// =============================================================================
std::string opcode_package_t::print_all(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix) {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < size_x_matrix ; i++) {
        for (uint32_t j = 0; j < size_y_matrix ; j++) {
            PackageString = input_matrix[i][j].content_to_string();
            if (PackageString.size() > 1) {
                FinalString = FinalString +
                                "[" + utils_t::uint32_to_string(i) + "] " +
                                "[" + utils_t::uint32_to_string(j) + "] " + PackageString;
            }
        }
        if (FinalString.size() > 1) {
            FinalString = FinalString + "\n";
        }
    }
    return FinalString;
};
