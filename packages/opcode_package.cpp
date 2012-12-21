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
opcode_package_t &opcode_package_t::operator=(const opcode_package_t &package) {
    /// TRACE Variables
    strncpy(this->opcode_assembly, package.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = package.opcode_operation;
    this->opcode_address = package.opcode_address;
    this->opcode_size = package.opcode_size;

    this->read_regs.clear();
    for (uint32_t i = 0; i < package.read_regs.size(); i++) {
        this->read_regs.push_back(package.read_regs[i]);
    }

    this->write_regs.clear();
    for (uint32_t i = 0; i < package.write_regs.size(); i++) {
        this->write_regs.push_back(package.write_regs[i]);
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

    if (this->read_regs.size() != package.read_regs.size()) return FAIL;
    for (uint32_t i = 0; i < package.read_regs.size(); i++) {
        if (this->read_regs[i] != package.read_regs[i]) return FAIL;
    }

    if (this->write_regs.size() != package.write_regs.size()) return FAIL;
    for (uint32_t i = 0; i < package.write_regs.size(); i++) {
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

    this->read_regs.clear();
    this->write_regs.clear();

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
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_READY;
};


//==============================================================================
void opcode_package_t::package_untreated(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
    this->state = PACKAGE_STATE_UNTREATED;
};


//==============================================================================
void opcode_package_t::package_wait(uint32_t stall_time) {
    this->ready_cycle = sinuca_engine.get_global_cycle() + stall_time;
    // ~ this->born_cycle = sinuca_engine.get_global_cycle();
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
    for (uint32_t i = 0; i < this->read_regs.size(); i++) {
        PackageString = PackageString + " " + utils_t::uint32_to_string(this->read_regs[i]);
    }

    PackageString = PackageString + " ] | WRegs[";
    for (uint32_t i = 0; i < this->write_regs.size(); i++) {
        PackageString = PackageString + " " + utils_t::uint32_to_string(this->write_regs[i]);
    }
    PackageString = PackageString + " ]";

    if (this->is_branch == true)
        PackageString = PackageString + " | JUMP  ";
    else
        PackageString = PackageString + " | NO JMP";

    return PackageString;
};


//==============================================================================
// Instruction Class
// Convert Instruction variables into String
std::string opcode_package_t::opcode_to_trace_string() {
    std::string traceString;
    traceString = this->opcode_assembly;
    traceString = traceString + " " + utils_t::uint32_to_char(this->opcode_operation);
    traceString = traceString + " 0x" + utils_t::uint64_to_char(this->opcode_address);
    traceString = traceString + " " + utils_t::uint32_to_char(this->opcode_size);

    traceString = traceString + " " + utils_t::uint32_to_char(this->read_regs.size());
    for (std::size_t i = 0; i < this->read_regs.size(); i++) {
        traceString = traceString + " " + utils_t::uint32_to_char(this->read_regs[i]);
    }

    traceString = traceString + " " + utils_t::uint32_to_char(this->write_regs.size());
    for (std::size_t i = 0; i < this->write_regs.size(); i++) {
        traceString = traceString + " " + utils_t::uint32_to_char(this->write_regs[i]);
    }

    traceString = traceString + " " + utils_t::uint32_to_char(this->base_reg);
    traceString = traceString + " " + utils_t::uint32_to_char(this->index_reg);

    if (this->is_read == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";

    if (this->is_read2 == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";

    if (this->is_write == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";


    if (this->is_branch == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";

    if (this->is_predicated == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";

    if (this->is_prefetch == true)
        traceString = traceString + " 1";
    else
        traceString = traceString + " 0";

    traceString = traceString + "\n";

    return traceString;
};


//==============================================================================
// Instruction Class
// Convert Instruction variables into char
void opcode_package_t::opcode_to_trace_char(char *trace_line) {
    std::size_t i;
    char tmp_str[64];

    strcpy(trace_line, this->opcode_assembly);

    sprintf(tmp_str, " %"PRIu32"", this->opcode_operation);
    strcat(trace_line, tmp_str);

    sprintf(tmp_str, " 0x%"PRIu64"", this->opcode_address);
    strcat(trace_line, tmp_str);

    sprintf(tmp_str, " %"PRIu32"", this->opcode_size);
    strcat(trace_line, tmp_str);

    sprintf(tmp_str, " %"PRIu32"", uint32_t(this->read_regs.size()));
    strcat(trace_line, tmp_str);
    for (i = 0; i < this->read_regs.size(); i++) {
        sprintf(tmp_str, " %"PRIu32"", this->read_regs[i]);
        strcat(trace_line, tmp_str);
    }

    sprintf(tmp_str, " %"PRIu32"", uint32_t(this->write_regs.size()));
    strcat(trace_line, tmp_str);
    for (i = 0; i < this->write_regs.size(); i++) {
        sprintf(tmp_str, " %"PRIu32"", this->write_regs[i]);
        strcat(trace_line, tmp_str);
    }

    /// Base and Index Registers
    sprintf(tmp_str, " %"PRIu32"", this->base_reg);
    strcat(trace_line, tmp_str);

    sprintf(tmp_str, " %"PRIu32"", this->index_reg);
    strcat(trace_line, tmp_str);

    /// Flags
    if (this->is_read == true)
        strcat(trace_line, " 1");
    else
        strcat(trace_line, " 0");

    if (this->is_read2 == true)
        strcat(trace_line, " 1");
    else
        strcat(trace_line,  " 0");

    if (this->is_write == true)
        strcat(trace_line,  " 1");
    else
        strcat(trace_line,  " 0");


    if (this->is_branch == true)
        strcat(trace_line,  " 1");
    else
        strcat(trace_line,  " 0");

    if (this->is_predicated == true)
        strcat(trace_line,  " 1");
    else
        strcat(trace_line,  " 0");

    if (this->is_prefetch == true)
        strcat(trace_line,  " 1");
    else
        strcat(trace_line,  " 0");

    strcat(trace_line,  "\n");
};

//==============================================================================
/// Convert Dynamic Memory Trace line into Instruction Memory Operands
/// Field N.:   01 |  02  |      03
///     Ex:     W 8 0x140735291283448
///             W 8 0x140735291283440
///             W 8 0x140735291283432
void opcode_package_t::trace_string_to_read(std::string input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    std::string sub_string;

/*
    // THE FOLLOWING CODE IS SLOWER


    // ~ int32_t end_pos = 0;

    // 1st field
    end_pos = input_string.find_first_of(' ',  end_pos);
    sub_string = input_string.substr(start_pos, end_pos - start_pos);
    /// Read or Write (Check the Instruction Type and the Memory Type)
    ERROR_ASSERT_PRINTF(sub_string.compare("R") == 0, "MemoryTraceFile Wrong Type. Type (R) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string.c_str())

    // 2nd field
    start_pos = end_pos;
    end_pos = input_string.find_first_of(' ',  end_pos + 1);
    sub_string = input_string.substr(start_pos, end_pos - start_pos);
    /// Load/Store Size
    this->read_size = strtoull(sub_string.c_str(), NULL, 10);

    // 3rd field
    start_pos = end_pos;
    end_pos = input_string.find_first_of(' ',  end_pos + 1);
    sub_string = input_string.substr(start_pos, end_pos - start_pos);
    /// Memory Address
    this->read_address = strtoull(sub_string.c_str(), NULL, 10);

    // 4th field
    start_pos = end_pos;
    end_pos = input_string.find_first_of(' ',  end_pos + 1);
    sub_string = input_string.substr(start_pos, end_pos - start_pos);
    /// Basic Block Number
    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string.c_str(), NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string.c_str(), NULL, 10))
*/


    for (end_pos = 0 ; end_pos <= input_string.length() ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == input_string.length()) {
            sub_string = input_string.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;

            switch (field) {
                case 1:
                    /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(sub_string.compare("R") == 0, "MemoryTraceFile Wrong Type. Type (R) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string.c_str())
                    field = 2;  /// Next Field
                break;

                case 2:
                    /// Load/Store Size
                    this->read_size = strtoull(sub_string.c_str(), NULL, 10);
                    field = 3;  /// Next Field
                break;


                case 3:
                    /// Memory Address
                    this->read_address = strtoull(sub_string.c_str(), NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    field = 5; /// Next Field
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string.c_str(), NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string.c_str(), NULL, 10))
                break;

                default:
                    ERROR_ASSERT_PRINTF(false, "Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};

//==============================================================================
void opcode_package_t::trace_string_to_read2(std::string input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    std::string sub_string;

    for (end_pos = 0 ; end_pos <= input_string.length() ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == input_string.length()) {
            sub_string = input_string.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;

            switch (field) {
                case 1:     /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(sub_string.compare("R") == 0, "MemoryTraceFile Wrong Type. Type (R) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string.c_str())
                    field = 2;  /// Next Field
                break;

                case 2:     /// Load/Store Size
                    this->read2_size = strtoull(sub_string.c_str(), NULL, 10);
                    field = 3;  /// Next Field
                break;

                case 3:     /// Memory Address
                    this->read2_address = strtoull(sub_string.c_str(), NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    field = 5; /// Next Field
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string.c_str(), NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string.c_str(), NULL, 10))
                break;

                default:
                    ERROR_ASSERT_PRINTF(false, "Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};

//==============================================================================
void opcode_package_t::trace_string_to_write(std::string input_string, uint32_t actual_bbl) {
    int32_t start_pos = 0;
    uint32_t end_pos = 0;
    uint32_t field = 1;
    std::string sub_string;

    for (end_pos = 0 ; end_pos <= input_string.length() ; end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == input_string.length()) {
            sub_string = input_string.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;

            switch (field) {
                case 1:     /// Read or Write (Check the Instruction Type and the Memory Type)
                    ERROR_ASSERT_PRINTF(sub_string.compare("W") == 0, "MemoryTraceFile Wrong Type. Type (W) expected.\n Inst: %s\n Mem:%s\n", this->content_to_string().c_str(), input_string.c_str())
                    field = 2;  /// Next Field
                break;

                case 2:     /// Load/Store Size
                    this->write_size = strtoull(sub_string.c_str(), NULL, 10);
                    field = 3;  /// Next Field
                break;

                case 3:     /// Memory Address
                    this->write_address = strtoull(sub_string.c_str(), NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    /// Basic Block Number
                    field = 5; /// Next Field
                    ERROR_ASSERT_PRINTF((uint32_t)strtoul(sub_string.c_str(), NULL, 10) == actual_bbl, "Wrong bbl inside memory_trace. Actual bbl (%u) - trace has (%u)\n", actual_bbl, (uint32_t)strtoul(sub_string.c_str(), NULL, 10))
                break;

                default:
                    ERROR_ASSERT_PRINTF(false, "Error converting Text to Memory (Wrong  number of fields)\n")
                break;
            }
        }
    }
};


//==============================================================================
/// Convert Static Trace line into Instruction
void opcode_package_t::trace_string_to_opcode(std::string input_string) {
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
    int32_t start_pos = 0;
    uint32_t end_pos = 0, field = 1, sub_fields = 0;
    std::string sub_string;

    ERROR_ASSERT_PRINTF(!input_string.empty() && input_string[0] != '#' && input_string[0] != '@', "Error converting Text to Instruction\n String::%s \n", input_string.c_str())

    for (end_pos = 0; end_pos <= input_string.length(); end_pos++) {
        if (input_string[end_pos] == ' ' || end_pos == input_string.length()) {
            sub_string = input_string.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;
            /// printf("%s\n", sub_string.c_str());

            switch (field) {
                case 1:
                    strncpy(this->opcode_assembly, sub_string.c_str(), sizeof(this->opcode_assembly));
                    field = 2;  /// Next Field
                break;

                case 2:
                    this->opcode_operation = instruction_operation_t(atoi(sub_string.c_str()));
                    field = 3;  /// Next Field
                break;


                case 3:
                    ERROR_ASSERT_PRINTF(sub_string.compare(0, 2, "0x") == 0, "Error converting Text to Instruction (Wrong number of fields)\n")
                    sub_string = sub_string.substr(2, sub_string.length());

                    this->opcode_address = strtoull(sub_string.c_str(), NULL, 10);
                    field = 4;  /// Next Field
                break;

                case 4:
                    this->opcode_size = atoi(sub_string.c_str());
                    field = 5;  /// Next Field
                break;


                case 5:
                    this->read_regs.clear();
                    sub_fields = atoi(sub_string.c_str());
                    if (sub_fields > 0)
                        field = 6;  /// Next Field
                    else
                        field = 7;  /// Next Field
                break;
                        case 6:
                            this->read_regs.push_back(strtoull(sub_string.c_str(), NULL, 10));

                            sub_fields--;
                            if (sub_fields > 0)
                                field = 6;  /// Next Field
                            else
                                field = 7;  /// Next Field
                        break;


                case 7:
                    this->write_regs.clear();
                    sub_fields = atoi(sub_string.c_str());
                    if (sub_fields > 0)
                        field = 8;  /// Next Field
                    else
                        field = 9;  /// Next Field
                break;
                        case 8:
                            this->write_regs.push_back(strtoull(sub_string.c_str(), NULL, 10));

                            sub_fields--;
                            if (sub_fields > 0)
                                field = 8;  /// Next Field
                            else
                                field = 9;  /// Next Field
                        break;
                /// Base and Index Registers
                case 9:
                    this->base_reg = strtoull(sub_string.c_str(), NULL, 10);
                    field = 10;  /// Next Field
                break;


                case 10:
                    this->index_reg = strtoull(sub_string.c_str(), NULL, 10);
                    field = 11;  /// Next Field
                break;

                /// Flags
                case 11:
                    if (sub_string == "1")
                        this->is_read = true;
                    else
                        this->is_read = false;
                    field = 12;  /// Next Field
                break;

                case 12:
                    if (sub_string == "1")
                        this->is_read2 = true;
                    else
                        this->is_read2 = false;
                    field = 13;  /// Next Field
                break;

                case 13:
                    if (sub_string == "1")
                        this->is_write = true;
                    else
                        this->is_write = false;
                    field = 14;  /// Next Field
                break;

                case 14:
                    if (sub_string == "1")
                        this->is_branch = true;
                    else
                        this->is_branch = false;
                    field = 15;  /// Next Field
                break;

                case 15:
                    if (sub_string == "1")
                        this->is_predicated = true;
                    else
                        this->is_predicated = false;
                    field = 16;  /// Next Field
                break;

                case 16:
                    if (sub_string == "1")
                        this->is_prefetch = true;
                    else
                        this->is_prefetch = false;
                    field = 17;  /// Next Field
                break;

                default:
                    ERROR_ASSERT_PRINTF(false, "Error converting Text to Instruction (More fields than wanted) - %s\n", sub_string.c_str())
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
