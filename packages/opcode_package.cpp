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
opcode_package_t::opcode_package_t() {
    this->package_clean();
};
//==============================================================================
opcode_package_t::~opcode_package_t(){
};

//==============================================================================
opcode_package_t &opcode_package_t::operator=(const opcode_package_t &package) {
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

    this->base_reg = package.base_reg;
    this->index_reg = package.index_reg;

    /// Flags
    this->is_read = package.is_read;
    this->read_address = package.read_address;
    this->read_size = package.read_size;

    this->is_read2 = package.is_read2;
    this->read2_address = package.read2_address;
    this->read2_size = package.read2_size;

    this->is_write = package.is_write;
    this->write_address = package.write_address;
    this->write_size = package.write_size;

    this->is_branch = package.is_branch;
    this->is_predicated = package.is_predicated;
    this->is_prefetch = package.is_prefetch;

    /// SINUCA Control Variables
    this->opcode_number = package.opcode_number;
    this->state = package.state;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->born_cycle = sinuca_engine.get_global_cycle();
    this->sync_type = package.sync_type;

    return *this;
};

//==============================================================================
bool opcode_package_t::operator==(const opcode_package_t &package) {
    /// TRACE Variables
    if (strncmp(this->opcode_assembly, package.opcode_assembly, sizeof(this->opcode_assembly)) != 0) return FAIL;
    if (this->opcode_operation != package.opcode_operation) return FAIL;
    if (this->opcode_address != package.opcode_address) return FAIL;
    if (this->opcode_size != package.opcode_size) return FAIL;

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] != package.read_regs[i]) return FAIL;
    }

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] != package.write_regs[i]) return FAIL;
    }

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

    if (this->is_branch != package.is_branch) return FAIL;
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

//==============================================================================
void opcode_package_t::package_clean() {
    /// TRACE Variables
    sprintf(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->read_regs[i] = POSITION_FAIL;
    }

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        this->write_regs[i] = POSITION_FAIL;
    }

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

    this->is_branch = false;
    this->is_predicated = false;
    this->is_prefetch = false;

    /// SINUCA Control Variables
    this->opcode_number = 0;
    this->ready_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_FREE;
    this->born_cycle = sinuca_engine.get_global_cycle();
    this->sync_type = SYNC_FREE;
};

//==============================================================================
void opcode_package_t::package_ready(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_READY;
};


//==============================================================================
void opcode_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_UNTREATED;
};


//==============================================================================
void opcode_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    this->state = PACKAGE_STATE_WAIT;
};


//==============================================================================
/// Convert Instruction variables into String
std::string opcode_package_t::content_to_string() {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->state == PACKAGE_STATE_FREE) {
            return PackageString;
        }
    #endif
    PackageString = PackageString + " OPCODE#" + utils_t::uint64_to_string(this->opcode_number);
    PackageString = PackageString + " " + get_enum_instruction_operation_char(this->opcode_operation);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_string(this->opcode_address);
    PackageString = PackageString + " SIZE:" + utils_t::uint32_to_string(this->opcode_size);

    PackageString = PackageString + " | R1 0x" + utils_t::uint64_to_string(this->read_address);
    PackageString = PackageString + " SIZE:" + utils_t::uint32_to_string(this->read_size);

    PackageString = PackageString + " | R2 0x" + utils_t::uint64_to_string(this->read2_address);
    PackageString = PackageString + " SIZE:" + utils_t::uint32_to_string(this->read2_size);

    PackageString = PackageString + " | W 0x" + utils_t::uint64_to_string(this->write_address);
    PackageString = PackageString + " SIZE:" + utils_t::uint32_to_string(this->write_size);

    PackageString = PackageString + " | " + get_enum_package_state_char(this->state);
    PackageString = PackageString + " BORN:" + utils_t::uint64_to_string(this->born_cycle);
    PackageString = PackageString + " READY:" + utils_t::uint64_to_string(this->ready_cycle);

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

    if (this->is_branch == true)
        PackageString = PackageString + " | JUMP  ";
    else
        PackageString = PackageString + " | NO JMP";

    return PackageString;
};


//============================================================================== NEW
// Instruction Class
// Convert Instruction variables into String
void opcode_package_t::opcode_to_trace_string(char *trace_string) {
    static char *register_string = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    uint32_t reg_count;

    trace_string[0] = '\0';
    strcat(trace_string, this->opcode_assembly);

    strcat(trace_string, " ");
    strcat(trace_string, utils_t::uint32_to_char(this->opcode_operation));

    strcat(trace_string," 0x" );
    strcat(trace_string, utils_t::uint64_to_char(this->opcode_address));

    strcat(trace_string, " " );
    strcat(trace_string,  utils_t::uint32_to_char(this->opcode_size));

    register_string[0] = '\0';
    reg_count = 0;
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            reg_count++;
            strcat(register_string, " ");
            strcat(register_string, utils_t::uint32_to_char(this->read_regs[i]));
        }
    }
    strcat(trace_string, " ");
    strcat(trace_string, utils_t::uint32_to_char(reg_count));
    strcat(trace_string, register_string);

    register_string[0] = '\0';
    reg_count = 0;
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            reg_count++;
            strcat(register_string, " ");
            strcat(register_string, utils_t::uint32_to_char(this->write_regs[i]));
        }
    }
    strcat(trace_string, " ");
    strcat(trace_string, utils_t::uint32_to_char(reg_count));
    strcat(trace_string, register_string);

    strcat(trace_string, " ") ;
    strcat(trace_string,  utils_t::uint32_to_char(this->base_reg));


    strcat(trace_string, " " );
    strcat(trace_string,  utils_t::uint32_to_char(this->index_reg));


    if (this->is_read == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string," 0");

    if (this->is_read2 == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string," 0");

    if (this->is_write == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string," 0");

    if (this->is_branch == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string, " 0");

    if (this->is_predicated == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string," 0");

    if (this->is_prefetch == true)
        strcat(trace_string," 1");
    else
        strcat(trace_string," 0");

   strcat(trace_string,"\n");

};


//============================================================================== NEW
/// Convert Dynamic Memory Trace line into Instruction Memory Operands
/// Field N.:   01 |  02  |      03
///     Ex:     W 8 0x140735291283448
///             W 8 0x140735291283440
///             W 8 0x140735291283432
void opcode_package_t::trace_string_to_read(const char * input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    static char * sub_string = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    sub_string[0] = '\0';

    for (end_pos = 0 ; end_pos <= strlen(input_string) ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == strlen(input_string)) {
            strncpy(sub_string, input_string + start_pos, end_pos - start_pos);
            sub_string[end_pos - start_pos] = '\0';
            start_pos = end_pos + 1;

            switch (field) {
                case 1:
                    /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(strcmp(sub_string, "R") == 0, "MemoryTraceFile Wrong Type. Type (R) expected.\n Inst: %s\n Mem:%s\n",
                                                                                            this->content_to_string().c_str(), input_string)
                    field = 2;  /// Next Field
                break;

                case 2:
                    /// Load/Store Size
                    this->read_size = strtoull(sub_string, NULL, 10);
                    field = 3;  /// Next Field
                break;

                case 3:
                    /// Memory Address
                    this->read_address = strtoull(sub_string, NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n",
                                                                                            actual_bbl, (uint32_t)strtoul(sub_string, NULL, 10))
                    field = 5; /// Next Field
                break;

                default:
                    ERROR_PRINTF("Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};


//============================================================================== NEW
void opcode_package_t::trace_string_to_read2(const char * input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    static char * sub_string = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    sub_string[0] = '\0';

    for (end_pos = 0 ; end_pos <= strlen(input_string) ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == strlen(input_string)) {
            strncpy(sub_string, input_string + start_pos, end_pos - start_pos);
            sub_string[end_pos - start_pos] = '\0';
            start_pos = end_pos + 1;

            switch (field) {
                case 1:     /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(strcmp(sub_string,"R") == 0, "MemoryTraceFile Wrong Type. Type (R) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string)
                    field = 2;  /// Next Field
                break;

                case 2:     /// Load/Store Size
                    this->read2_size = strtoull(sub_string, NULL, 10);
                    field = 3;  /// Next Field
                break;

                case 3:     /// Memory Address
                    this->read2_address = strtoull(sub_string, NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    field = 5; /// Next Field
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string, NULL, 10))
                break;

                default:
                    ERROR_PRINTF("Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};


//============================================================================== NEW
void opcode_package_t::trace_string_to_write(const char * input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    static char * sub_string = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    sub_string[0] = '\0';

    for (end_pos = 0 ; end_pos <= strlen(input_string) ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == strlen(input_string)) {
            strncpy(sub_string, input_string + start_pos, end_pos - start_pos);
            sub_string[end_pos - start_pos] = '\0';
            start_pos = end_pos + 1;

            switch (field) {
                case 1:     /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(strcmp(sub_string,"W") == 0, "MemoryTraceFile Wrong Type. Type (W) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string)
                    field = 2;  /// Next Field
                break;

                case 2:     /// Load/Store Size
                    this->write_size = strtoull(sub_string, NULL, 10);
                    field = 3;  /// Next Field
                break;

                case 3:     /// Memory Address
                    this->write_address = strtoull(sub_string, NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    field = 5; /// Next Field
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string, NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string, NULL, 10))
                break;

                default:
                    ERROR_PRINTF("Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};


//============================================================================== NEW
/// Convert Static Trace line into Instruction
/// Field N.:   01                 |   02                | 03           | 04        |   05      |    06   |   07        |     08        |     09
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
void opcode_package_t::trace_string_to_opcode(const char *input_string) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0, field = 1, sub_fields = 0, reg_num = 0;

    static char *sub_string = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    sub_string[0] = '\0';

    ERROR_ASSERT_PRINTF(strlen(input_string) != 0 && input_string[0] != '#' && input_string[0] != '@', "Error converting Text to Instruction\n String::%s \n", input_string)
    for (end_pos = 0; end_pos <= strlen(input_string); end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == strlen(input_string)) {

            strncpy(sub_string, input_string + start_pos, end_pos - start_pos);
            sub_string[end_pos - start_pos] = '\0';
            start_pos = end_pos + 1;
            /// printf("%s\n", sub_string);

            switch (field) {
                case 1:
                    strncpy(this->opcode_assembly, sub_string, sizeof(this->opcode_assembly));
                    field = 2;  /// Next Field
                break;

                case 2:
                    this->opcode_operation = instruction_operation_t(atoi(sub_string));
                    field = 3;  /// Next Field
                break;

                case 3:
                    ERROR_ASSERT_PRINTF(memcmp(sub_string, "0x", 2) == 0, "Error converting Text to Instruction (Wrong number of fields), input_string = %s\n", sub_string)
                    this->opcode_address = strtoull(sub_string + 2, NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    this->opcode_size = atoi(sub_string);
                    field = 5;  /// Next Field
                break;

                case 5:
                    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                        this->read_regs[i] = POSITION_FAIL;
                    }

                    sub_fields = atoi(sub_string);
                    reg_num = 0;

                    if (sub_fields > 0)
                        field = 6;  /// Next Field
                    else
                        field = 7;  /// Next Field
                break;
                        case 6:
                            this->read_regs[reg_num] = strtoull(sub_string, NULL, 10);
                            reg_num++;
                            sub_fields--;

                            if (sub_fields > 0)
                                field = 6;  /// Next Field
                            else
                                field = 7;  /// Next Field
                        break;


                case 7:
                    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                        this->write_regs[i] = POSITION_FAIL;
                    }

                    sub_fields = atoi(sub_string);
                    reg_num = 0;
                    if (sub_fields > 0)
                        field = 8;  /// Next Field
                    else
                        field = 9;  /// Next Field
                break;
                        case 8:
                            this->write_regs[reg_num] = strtoull(sub_string, NULL, 10);
                            reg_num++;
                            sub_fields--;

                            if (sub_fields > 0)
                                field = 8;  /// Next Field
                            else
                                field = 9;  /// Next Field
                        break;
                /// Base and Index Registers
                case 9:
                    this->base_reg = strtoull(sub_string, NULL, 10);
                    field = 10;  /// Next Field
                break;


                case 10:
                    this->index_reg = strtoull(sub_string, NULL, 10);
                    field = 11;  /// Next Field
                break;

                /// Flags
                case 11:
                    this->is_read = (sub_string[0] == '1');
                    field = 12;  /// Next Field
                break;

                case 12:
                    this->is_read2 = (sub_string[0] == '1');
                    field = 13;  /// Next Field
                break;

                case 13:
                    this->is_write = (sub_string[0] == '1');
                    field = 14;  /// Next Field
                break;

                case 14:
                    this->is_branch = (sub_string[0] == '1');
                    field = 15;  /// Next Field
                break;

                case 15:
                    this->is_predicated = (sub_string[0] == '1');
                    field = 16;  /// Next Field
                break;

                case 16:
                    this->is_prefetch = (sub_string[0] == '1');
                    field = 17;  /// Next Field
                break;

                default:
                    ERROR_PRINTF("Error converting Text to Instruction (More fields than wanted) - %d %s\n", field, sub_string)
                break;
            }
        }
    }
    ERROR_ASSERT_PRINTF(field == 17, "Error converting Text to Instruction (Less fields than wanted) - %d\n", field)
};


//==============================================================================
int32_t opcode_package_t::find_free(opcode_package_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].state == PACKAGE_STATE_FREE)
            return i;
    }
    return POSITION_FAIL;
};


//==============================================================================
int32_t opcode_package_t::find_opcode_number(opcode_package_t *input_array, uint32_t size_array, uint64_t opcode_number) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].opcode_number == opcode_number) {
            return i;
        }
    }
    return POSITION_FAIL;
};

//==============================================================================
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

//==============================================================================
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

//==============================================================================
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

//==============================================================================
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
