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
#include "../sinuca.hpp"

#ifdef PROCESSOR_DEBUG
    #define PROCESSOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PROCESSOR_DEBUG_PRINTF(...)
#endif

#ifdef SYNC_DEBUG
    #define SYNC_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define SYNC_DEBUG_PRINTF(...)
#endif

//==============================================================================
processor_t::processor_t() {
    this->set_type_component(COMPONENT_PROCESSOR);

    /// Synchronization Control Variables
    this->sync_status = SYNC_FREE;          /// Used to sync or request sync
    this->sync_status_time = 0;             /// Last time the sync changed
    this->trace_over = false;

    this->send_instruction_ready_cycle = 0;
    this->send_data_ready_cycle = 0;

    /// Stages Control Variables
    this->trace_next_opcode.package_clean();
    this->fetch_pc = 0;                     /// Last PC requested to IC
    this->fetch_pc_line_buffer = 0;         /// Last PC answered by IC

    this->fetch_opcode_counter = 1;         /// Last opcode fetched
    this->decode_opcode_counter = 1;        /// Last opcode decoded

    this->decode_uop_counter = 1;           /// Last uop decoded
    this->rename_uop_counter = 1;           /// Last uop decoded
    this->commit_uop_counter = 1;           /// Last uop commited

    this->fetch_buffer = NULL;
    this->decode_buffer = NULL;
    this->reorder_buffer = NULL;

    /// Buffers' size
    this->fetch_buffer_size = 0;
    this->decode_buffer_size = 0;
    this->reorder_buffer_size = 0;
    this->register_alias_table_size = 0;

    /// Latency for each processor stage
    this->stage_fetch_cycles = 0;
    this->stage_decode_cycles = 0;
    this->stage_rename_cycles = 0;
    this->stage_dispatch_cycles = 0;
    this->stage_execution_cycles = 0;
    this->stage_commit_cycles = 0;

    /// Width for each processor stage
    this->stage_fetch_width = 0;
    this->stage_decode_width = 0;
    this->stage_rename_width = 0;
    this->stage_dispatch_width = 0;
    this->stage_execution_width = 0;
    this->stage_commit_width = 0;

    /// Integer Funcional Units
    this->fu_int_alu = 0;
    this->number_fu_int_alu = 0;
    this->latency_fu_int_alu = 0;

    this->fu_int_mul = 0;
    this->number_fu_int_mul = 0;
    this->latency_fu_int_mul = 0;

    this->fu_int_div = 0;
    this->number_fu_int_div = 0;
    this->latency_fu_int_div = 0;

    /// Floating Point Functional Units
    this->fu_fp_alu = 0;
    this->number_fu_fp_alu = 0;
    this->latency_fu_fp_alu = 0;

    this->fu_fp_mul = 0;
    this->number_fu_fp_mul = 0;
    this->latency_fu_fp_mul = 0;

    this->fu_fp_div = 0;
    this->number_fu_fp_div = 0;
    this->latency_fu_fp_div = 0;

    /// Memory Functional Units
    this->fu_mem_load = 0;
    this->number_fu_mem_load = 0;
    this->latency_fu_mem_load = 0;

    this->fu_mem_store = 0;
    this->number_fu_mem_store = 0;
    this->latency_fu_mem_store = 0;

    this->read_buffer_size = 0;
    this->write_buffer_size = 0;

    this->branch_per_fetch = 0;

    this->data_cache = NULL;
    this->inst_cache = NULL;
};

//==============================================================================
processor_t::~processor_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint64_t>(recv_ready_cycle);

    utils_t::template_delete_array<opcode_package_t>(fetch_buffer);
    utils_t::template_delete_array<uop_package_t>(decode_buffer);
    utils_t::template_delete_array<reorder_buffer_line_t>(reorder_buffer);

    utils_t::template_delete_array<reorder_buffer_line_t*>(register_alias_table);
    utils_t::template_delete_array<reorder_buffer_line_t*>(memory_map_table);

    utils_t::template_delete_array<memory_package_t>(read_buffer);
    utils_t::template_delete_array<memory_package_t>(write_buffer);
};

//==============================================================================
void processor_t::allocate() {
    PROCESSOR_DEBUG_PRINTF("allocate()\n");
    this->trace_next_opcode.package_clean();

    this->recv_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_max_ports(), 0);

    /// Fetch Buffer
    this->fetch_buffer = utils_t::template_allocate_array<opcode_package_t>(this->fetch_buffer_size);
    this->fetch_buffer_position_start = 0;
    this->fetch_buffer_position_end = 0;
    this->fetch_buffer_position_used = 0;

    /// Decode Buffer
    ERROR_ASSERT_PRINTF(this->decode_buffer_size >= MAX_UOP_DECODED, "Decode buffer size should be bigger than %d (Max uops per opcode decoded).\n", MAX_UOP_DECODED);
    this->decode_buffer = utils_t::template_allocate_array<uop_package_t>(this->decode_buffer_size);
    this->decode_buffer_position_start = 0;
    this->decode_buffer_position_end = 0;
    this->decode_buffer_position_used = 0;

    /// ReOrder Buffer
    this->reorder_buffer = utils_t::template_allocate_array<reorder_buffer_line_t>(this->reorder_buffer_size);
    for (uint32_t i = 0; i < this->reorder_buffer_size; i++) {
        this->reorder_buffer[i].deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(this->reorder_buffer_size, NULL);
    }
    this->reorder_buffer_position_start = 0;
    this->reorder_buffer_position_end = 0;
    this->reorder_buffer_position_used = 0;

    /// Register Alias Table for Renaming
    this->register_alias_table_size = 256; /// Number of registers on the trace
    this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(this->register_alias_table_size, NULL);

    /// Memory Map for Renaming
    this->memory_map_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(this->reorder_buffer_size, NULL);

    /// Propagate the allocate()
    this->branch_predictor.allocate();
    this->branch_solve_stage = PROCESSOR_STAGE_FETCH;
    this->branch_opcode_number = 0;

    ERROR_ASSERT_PRINTF(this->branch_per_fetch > 0, "Maximum number of branches per fetch must be greater than 0.\n")

    /// Functional Units
    uint32_t total_dispatched = this->number_fu_int_alu + this->number_fu_int_mul + this->number_fu_int_div
                            + this->number_fu_fp_alu + this->number_fu_fp_mul + this->number_fu_fp_div
                            + this->number_fu_mem_load + this->number_fu_mem_store;
    ERROR_ASSERT_PRINTF(this->stage_dispatch_width <= total_dispatched, "Dispatch width must be less or equal to the number of functional units (%u).\n", total_dispatched);
    ERROR_ASSERT_PRINTF(this->stage_execution_width <= total_dispatched, "Execution width must be less or equal to the number of functional units (%u).\n", total_dispatched);

    this->read_buffer = utils_t::template_allocate_array<memory_package_t>(this->read_buffer_size);
    this->write_buffer = utils_t::template_allocate_array<memory_package_t>(this->write_buffer_size);

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size()), "Wrong line_size.\n")

    /// OFFSET MASK
    this->offset_bits_mask = 0;
    this->not_offset_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(sinuca_engine.get_global_line_size()); i++) {
        this->offset_bits_mask |= 1 << i;
    }
    this->not_offset_bits_mask = ~offset_bits_mask;

    char label[50] = "";
    sprintf(label, "Branch_Predictor_%s", this->get_label());
    this->branch_predictor.set_label(label);
    this->branch_predictor.allocate();


    #ifdef PROCESSOR_DEBUG
        this->print_configuration();
    #endif
};

//==============================================================================
void processor_t::synchronize(sync_t new_sync) {
    processor_t** sinuca_processor = sinuca_engine.get_processor_array();

    SYNC_DEBUG_PRINTF("All Status Before:");
    for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
        SYNC_DEBUG_PRINTF("[%s]", get_enum_sync_char(sinuca_processor[proc]->sync_status))
    }

    SYNC_DEBUG_PRINTF(" -- Processor[%u] => ", this->get_core_id());

    switch (new_sync) {
        /// ====================================================================
        /// IF any other has (SYNC_WAIT_CRITICAL_START or SYNC_CRITICAL_START)  =>  WAIT
        /// ELSE                                                                =>  CONTINUE
        case SYNC_CRITICAL_START: {
            SYNC_DEBUG_PRINTF("SYNC_CRITICAL_START => ");
            bool found_critical = false;
            for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
                if (sinuca_processor[proc]->sync_status == SYNC_CRITICAL_START ||
                    sinuca_processor[proc]->sync_status == SYNC_WAIT_CRITICAL_START) {
                        SYNC_DEBUG_PRINTF("WAIT");
                        this->sync_status = SYNC_WAIT_CRITICAL_START;
                        this->sync_status_time = sinuca_engine.get_global_cycle();
                        found_critical = true;
                        break;
                }

                ERROR_ASSERT_PRINTF(sinuca_processor[proc]->sync_status != SYNC_CRITICAL_END,
                                    "While processor[%u] synchronize[%s], found on processor[%u] sync_status[%s]\n",
                                    this->get_core_id(), get_enum_sync_char(new_sync), proc, get_enum_sync_char(sinuca_processor[proc]->sync_status));
            }
            if (!found_critical) {
                SYNC_DEBUG_PRINTF("START");
                this->sync_status = SYNC_CRITICAL_START;
                this->sync_status_time = sinuca_engine.get_global_cycle();
            }
        }
        break;

        /// ====================================================================
        /// IF any other has (SYNC_WAIT_CRITICAL_START)     =>  WAKE UP
        /// ELSE                                            =>  CONTINUE
        case SYNC_CRITICAL_END: {
            SYNC_DEBUG_PRINTF("SYNC_CRITICAL_END => ");

            /// System should not be here, but it may be recoverable
            if (this->sync_status != SYNC_CRITICAL_START) {
                WARNING_PRINTF("While processor[%u] synchronize[%s], found on processor[%d] sync_status[%s]\n",
                                this->get_core_id(), get_enum_sync_char(this->sync_status), this->get_core_id(), get_enum_sync_char(new_sync));
            }

            this->sync_status = SYNC_FREE;
            this->sync_status_time = sinuca_engine.get_global_cycle();

            bool found_wait_critical_start = false;
            uint32_t older_wait_critical_start = this->get_core_id();
            uint64_t cycle_older_wait_critical_start = this->sync_status_time;
            for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
                if (sinuca_processor[proc]->sync_status == SYNC_WAIT_CRITICAL_START) {
                    found_wait_critical_start = true;
                    if (sinuca_processor[proc]->sync_status_time < cycle_older_wait_critical_start) {
                        older_wait_critical_start = proc;
                        cycle_older_wait_critical_start = sinuca_processor[proc]->sync_status_time;
                    }
                }

                ERROR_ASSERT_PRINTF(sinuca_processor[proc]->sync_status != SYNC_CRITICAL_START &&
                                    sinuca_processor[proc]->sync_status != SYNC_CRITICAL_END,
                                    "While processor[%d] synchronize[%s], found on processor[%d] sync_status[%s]\n",
                                    this->get_core_id(), get_enum_sync_char(new_sync), proc, get_enum_sync_char(sinuca_processor[proc]->sync_status));
            }

            /// Wakeup some other processor waiting
            if (found_wait_critical_start) {
                SYNC_DEBUG_PRINTF("FOUND WAIT");
                sinuca_processor[older_wait_critical_start]->sync_status = SYNC_CRITICAL_START;
                sinuca_processor[older_wait_critical_start]->sync_status_time = sinuca_engine.get_global_cycle();
            }
        }
        break;

        /// ====================================================================
        /// IF all other has (SYNC_BARRIER)     =>  WAKE UP ALL & CONTINUE
        /// ELSE                                =>  WAIT
        case SYNC_BARRIER: {
            SYNC_DEBUG_PRINTF("SYNC_BARRIER => ");
            /// System should not be here, but it may be recoverable
            if (this->sync_status != SYNC_FREE) {
                WARNING_PRINTF("While processor[%u] synchronize[%s], found on processor[%d] sync_status[%s]\n",
                                this->get_core_id(), get_enum_sync_char(this->sync_status), this->get_core_id(), get_enum_sync_char(new_sync));
                switch (this->sync_status) {
                    case SYNC_CRITICAL_START:
                        WARNING_PRINTF("Propagating synchronize[%s] to avoid dead-lock.\n", get_enum_sync_char(SYNC_CRITICAL_END));
                        this->synchronize(SYNC_CRITICAL_END);
                    break;

                    case SYNC_CRITICAL_END:
                    case SYNC_BARRIER:
                    case SYNC_WAIT_CRITICAL_START:
                    case SYNC_FREE:
                        ERROR_ASSERT_PRINTF(false,
                                            "Processor[%d] should not receive synchronize[%s]\n",
                                            this->get_core_id(), get_enum_sync_char(new_sync));
                    break;
                }
            }

            bool is_last_barrier = true;
            for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
                if (proc != this->get_core_id() && !sinuca_processor[proc]->trace_over) {
                    if (sinuca_processor[proc]->sync_status != SYNC_BARRIER) {
                        SYNC_DEBUG_PRINTF("WAIT");
                        is_last_barrier = false;
                        this->sync_status = SYNC_BARRIER;
                        this->sync_status_time = sinuca_engine.get_global_cycle();
                        break;
                    }
                }
            }
            if (is_last_barrier) {
                SYNC_DEBUG_PRINTF("FREE");
                for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
                    sinuca_processor[proc]->sync_status = SYNC_FREE;
                    sinuca_processor[proc]->sync_status_time = sinuca_engine.get_global_cycle();
                }
            }
        }
        break;

        /// ====================================================================
        /// Should not receive these Sync_T
        case SYNC_WAIT_CRITICAL_START:
        case SYNC_FREE:
            ERROR_ASSERT_PRINTF(false,
                        "Processor[%d] should not receive synchronize[%s]\n",
                        this->get_core_id(), get_enum_sync_char(new_sync));
        break;
    }


    SYNC_DEBUG_PRINTF(" -- All Status After:");
    for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
        SYNC_DEBUG_PRINTF("[%s]", get_enum_sync_char(sinuca_processor[proc]->sync_status))
    }
    SYNC_DEBUG_PRINTF("\n");
};

//==============================================================================
void processor_t::solve_branch(uint64_t opcode_number, processor_stage_t processor_stage) {
    if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH &&
        this->branch_opcode_number == opcode_number &&
        this->branch_solve_stage == processor_stage) {
            this->branch_solve_stage = PROCESSOR_STAGE_FETCH;
    }
};

//==============================================================================
void processor_t::stage_fetch() {
    PROCESSOR_DEBUG_PRINTF("stage_fetch()\n");
    int32_t position_buffer = -1;
    uint32_t i;

    /// 1st. Trace => Fetch_Buffer
    bool valid_opcode = false;
    uint32_t count_branches = 0;

    for (i = 0; i < this->stage_fetch_width; i++) {
        /// If must wait Branch miss prediction
        if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH) {
            add_stat_branch_stall_cycles();
            break;
        }

        /// If must wait synchronization
        if (this->sync_status != SYNC_FREE && this->sync_status != SYNC_CRITICAL_START) {
            add_stat_sync_stall_cycles();
            break;
        }

        if (this->trace_next_opcode.state == PACKAGE_STATE_FREE) {
            valid_opcode = sinuca_engine.trace_reader->trace_fetch(this->core_id, &this->trace_next_opcode);
            if (!valid_opcode) {
                this->trace_over = true;
                break;
            }
        }

        /// If the instruction is a synchronization
        if (this->trace_next_opcode.sync_type != SYNC_FREE) {
            synchronize(this->trace_next_opcode.sync_type);
            this->trace_next_opcode.package_clean();
            continue;
        }

        /// If the instruction is a branch
        if (this->trace_next_opcode.opcode_operation == INSTRUCTION_OPERATION_BRANCH) {
            count_branches++;
            if (count_branches > this->branch_per_fetch){
                break;
            }
        }


        position_buffer = this->fetch_buffer_insert();
        if (position_buffer == POSITION_FAIL) {
            break;
        }

        PROCESSOR_DEBUG_PRINTF("\t Inserting on fetch_buffer[%d] package:%s\n", position_buffer, trace_next_opcode.opcode_to_string().c_str());
        this->fetch_buffer[position_buffer] = this->trace_next_opcode;
        this->fetch_buffer[position_buffer].package_untreated(this->stage_fetch_cycles);
        trace_next_opcode.package_clean();
        valid_opcode = sinuca_engine.trace_reader->trace_fetch(this->core_id, &this->trace_next_opcode);
        if (!valid_opcode) {
            this->trace_over = true;
            break;
        }

        /// If return different from PROCESSOR_STAGE_FETCH the fetch will stall,
        /// until the branch be solved (DECODE or EXECUTION stages)
        this->branch_solve_stage = this->branch_predictor.predict_branch(this->fetch_buffer[position_buffer], this->trace_next_opcode);
        this->branch_opcode_number = this->fetch_buffer[position_buffer].opcode_number;
        if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH) {
            PROCESSOR_DEBUG_PRINTF("\t Stalling fetch due to a miss prediction on package:%s\n", this->fetch_buffer[position_buffer].opcode_to_string().c_str());
        }
    }


    /// 2nd. Fetch_Buffer => Inst.Cache
    for (i = 0; i < this->stage_fetch_width ; i++) {
        position_buffer = this->fetch_buffer_find_opcode_number(this->fetch_opcode_counter);
        if (position_buffer == POSITION_FAIL ||
            this->fetch_buffer[position_buffer].state != PACKAGE_STATE_UNTREATED ||
            this->fetch_buffer[position_buffer].ready_cycle > sinuca_engine.get_global_cycle()) {
                break;
        }
        PROCESSOR_DEBUG_PRINTF("\t Sending INST package:%s\n", this->fetch_buffer[position_buffer].opcode_to_string().c_str());

        bool waiting = false;
        /// Check if the package still WAITING
        for (uint32_t k = 0; k < this->fetch_buffer_position_used; k++) {
            /// Update the FETCH BUFFER position
            uint32_t i = this->fetch_buffer_position_start + k;
            if (i >= this->fetch_buffer_size) {
                i -= this->fetch_buffer_size;
            }

            if (this->fetch_buffer[i].state == PACKAGE_STATE_WAIT &&
            this->cmp_index_tag(this->fetch_buffer[i].opcode_address, this->fetch_buffer[position_buffer].opcode_address)) {
                waiting = true;
                break;
            }
        }

        /// Requested Line
        if (waiting == true) {
            PROCESSOR_DEBUG_PRINTF("\t ALREADY REQUESTED THE SAME LINE (WAIT)\n");
            fetch_buffer[position_buffer].package_wait(1);
        }
        /// Line Buffer
        else if (this->cmp_index_tag(this->fetch_buffer[position_buffer].opcode_address, this->fetch_pc_line_buffer)) {
            PROCESSOR_DEBUG_PRINTF("\t FOUND INTO FETCH BUFFER (READY)\n");
            fetch_buffer[position_buffer].package_ready(1);
        }
        else {
            int32_t transmission_latency = this->send_instruction_package(this->fetch_buffer+position_buffer);
            if (transmission_latency != POSITION_FAIL) {  /// Try to send to the IC.
                fetch_buffer[position_buffer].package_wait(transmission_latency);
            }
            else {  /// Inst. Cache cannot receive, stall the fetch for one cycle.
                break;
            }
        }
        this->fetch_opcode_counter++;
    }
};

//==============================================================================
/// Create a memory package
/// Get the route
/// Try to send
///     If OK
///         Get and Return the Latency
///     Else
///         Return POSITION_FAIL
/*
            const ADDRINT highAddr = addr + size;
            const ADDRINT lineSize = GetLineSize();
            const ADDRINT notLineMask = ~(lineSize - 1);

            ADDRINT tag=0;
            UINT32 setIndex=0;
            UINT32 setOffSet=0;
            UINT32 sizeNow=0;

            do {
                SplitAddress(addr, tag, setIndex, setOffSet);
                //!If the size ends on the next line...
                sizeNow = size;                               //SizeNow = |......xxxxx.|
                if (setOffSet+size > this->_lineSize) {
                    sizeNow = this->_lineSize - setOffSet;    //SizeNow = |.........xxx|xx
                    size = size-sizeNow;
                }
                addr = (addr & notLineMask) + lineSize; // start of next cache line request
            }
            while (addr < highAddr); or  MAX_LINE_ACCESS
*/

int32_t processor_t::send_instruction_package(opcode_package_t *inst_package) {
    PROCESSOR_DEBUG_PRINTF("send_instruction_package() package:%s\n", inst_package->opcode_to_string().c_str());

    if (this->send_instruction_ready_cycle <= sinuca_engine.get_global_cycle()) {
        memory_package_t package;
        package.packager(
            this->get_id(),                     /// Request Owner Id
            inst_package->opcode_number,        /// opcode Number
            inst_package->opcode_address,       /// opcode Address
            0,                                  /// uop Number

            /// Request the whole line
            inst_package->opcode_address & this->not_offset_bits_mask,     /// Mem. Address
            sinuca_engine.get_global_line_size(),               /// Instruction Size

            PACKAGE_STATE_TRANSMIT,             /// Pack. State
            0,                                  /// Ready Cycle Latency

            MEMORY_OPERATION_INST,              /// Mem Op. Type
            false,                              /// Is Answer

            this->get_id(),                                                                 /// Src ID
            this->get_interface_output_component(PROCESSOR_PORT_INST_CACHE)->get_id(),      /// Dst ID
            NULL,                               /// *Hops
            POSITION_FAIL                       /// Hop Counter
        );

        sinuca_engine.interconnection_controller->find_package_route(&package);
        ERROR_ASSERT_PRINTF(package.hop_count != POSITION_FAIL, "Achieved the end of the route\n");
        uint32_t output_port = package.hops[package.hop_count];  /// Where to send the package ?
        ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
        package.hop_count--;  /// Consume its own port

        uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(&package, this, this->get_interface_output_component(output_port));
        bool sent = this->get_interface_output_component(output_port)->receive_package(&package, this->get_ports_output_component(output_port), transmission_latency);
        if (sent) {
            PROCESSOR_DEBUG_PRINTF("\tSEND DATA OK\n");
            this->send_instruction_ready_cycle = sinuca_engine.get_global_cycle() + transmission_latency;
            return transmission_latency;
        }
        else {
            PROCESSOR_DEBUG_PRINTF("\tSEND DATA FAIL\n");
            package.hop_count++;  /// Do not Consume its own port
            return POSITION_FAIL;
        }
    }
    PROCESSOR_DEBUG_PRINTF("\tSEND DATA FAIL (BUSY)\n");
    return POSITION_FAIL;

};

//==============================================================================
/// Divide the opcode into
///  1st. uop READ MEM. + unaligned
///  2st. uop READ 2 MEM. + unaligned
///  3rd. uop BRANCH
///  4th. uop ALU
///  5th. uop WRITE MEM. + unaligned
void processor_t::stage_decode() {
    PROCESSOR_DEBUG_PRINTF("stage_decode()\n");
    int32_t position_buffer = POSITION_FAIL;

    /// Fetch_Buffer => (Decode) => Decode_Buffer
    for (uint32_t i = 0; i < this->stage_decode_width ; i++) {
        /// ====================================================================
        /// DECODE =============================================================

        /// Check if there is OpCode inside the fetch buffer
        /// Check if there is enough space for one OpCode be Decoded
        /// Check if the oldest fetch buffer position is ready
        if (this->fetch_buffer_position_used == 0 ||
        this->decode_buffer_size - this->decode_buffer_position_used < MAX_UOP_DECODED ||
        this->fetch_buffer[fetch_buffer_position_start].state != PACKAGE_STATE_READY ||
        this->fetch_buffer[fetch_buffer_position_start].ready_cycle > sinuca_engine.get_global_cycle()) {
            break;
        }
        ERROR_ASSERT_PRINTF(this->decode_opcode_counter == this->fetch_buffer[fetch_buffer_position_start].opcode_number, "Renaming out-of-order.\n")
        this->decode_opcode_counter++;

        /// DECODE WRITE ===================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_write) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_STORE,
                                                                        this->fetch_buffer[fetch_buffer_position_start].write_address, this->fetch_buffer[fetch_buffer_position_start].write_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);
            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].uop_to_string().c_str());
        }

        /// DECODE ALU =====================================================
        if (this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD &&
        this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_STORE &&
        this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_BRANCH) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, this->fetch_buffer[fetch_buffer_position_start].opcode_operation,
                                                                        0, 0,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);
            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].uop_to_string().c_str());
        }

        /// DECODE BRANCH ==================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_branch) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_BRANCH,
                                                                        0, 0,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);
            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].uop_to_string().c_str());

            /// Solve the Branch Prediction
            this->solve_branch(this->fetch_buffer[fetch_buffer_position_start].opcode_number, PROCESSOR_STAGE_DECODE);
        }

        /// DECODE READ 2 ==================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_read2) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_LOAD,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read2_address, this->fetch_buffer[fetch_buffer_position_start].read2_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);
            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].uop_to_string().c_str());
        }

        /// DECODE READ ====================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_read) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_LOAD,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read_address, this->fetch_buffer[fetch_buffer_position_start].read_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);
            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].uop_to_string().c_str());
        }
        /// Remove the oldest OPCODE (just decoded) from the fetch buffer
        this->fetch_buffer_remove();
    }
};

//==============================================================================
void processor_t::stage_rename() {
    PROCESSOR_DEBUG_PRINTF("stage_rename()\n");
    int32_t position_buffer;
    int32_t position_mmt, free_position_mmt;

    /// Copy from Rename => ROB and rename
    for (uint32_t i = 0; i < this->stage_rename_width; i++) {

        /// Check if there is any Uop decoded
        /// Check if the oldest fetch buffer position is ready
        if (decode_buffer_position_used == 0 ||
            this->decode_buffer[decode_buffer_position_start].state != PACKAGE_STATE_READY ||
            this->decode_buffer[decode_buffer_position_start].ready_cycle > sinuca_engine.get_global_cycle()) {
                break;
        }
        ERROR_ASSERT_PRINTF(this->rename_uop_counter == this->decode_buffer[decode_buffer_position_start].uop_number, "Renaming out-of-order.\n")

        /// Check if there is free space on the ROB
        position_buffer = this->rob_insert();
        if (position_buffer == POSITION_FAIL) {
            break;
        }

        PROCESSOR_DEBUG_PRINTF("\t Inserting ROB[%d] %s\n", position_buffer, this->decode_buffer[decode_buffer_position_start].uop_to_string().c_str());
        this->reorder_buffer[position_buffer].uop = this->decode_buffer[decode_buffer_position_start];
        this->decode_buffer_remove();

        this->rename_uop_counter++;
        this->reorder_buffer[position_buffer].stage = PROCESSOR_STAGE_RENAME;
        this->reorder_buffer[position_buffer].uop.package_ready(this->stage_rename_cycles);

        /// Insert into the reservation station
        this->unified_reservation_station.push_back(&this->reorder_buffer[position_buffer]);

        /// Solve the Branch Prediction
        this->solve_branch(this->reorder_buffer[position_buffer].uop.opcode_number, PROCESSOR_STAGE_RENAME);

        /// Control the Memory Dependency - READ
        if (this->reorder_buffer[position_buffer].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
            position_mmt = POSITION_FAIL;
            /// Find the Last Write to the same address
            for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
                if (this->memory_map_table[j] != NULL &&
                    cmp_index_tag(this->memory_map_table[j]->uop.memory_address, this->reorder_buffer[position_buffer].uop.memory_address)) {
                        position_mmt = j;
                        break;
                }
            }
            if (position_mmt != POSITION_FAIL) {
                /// Find FREE in the dependency list
                for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
                    if (this->memory_map_table[position_mmt]->deps_ptr_array[j] == NULL) {
                        this->memory_map_table[position_mmt]->deps_ptr_array[j] = &this->reorder_buffer[position_buffer];
                        this->reorder_buffer[position_buffer].wait_deps_number++;
                        break;
                    }
                }
            }
        }

        /// Control the Memory Dependency - WRITE
        else if (this->reorder_buffer[position_buffer].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
            position_mmt = POSITION_FAIL;
            free_position_mmt = POSITION_FAIL;
            /// Find the Last Write to the same address OR free space
            for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
                if (this->memory_map_table[j] == NULL) {
                    free_position_mmt = j;
                }
                else if (cmp_index_tag(this->memory_map_table[j]->uop.memory_address, this->reorder_buffer[position_buffer].uop.memory_address)) {
                    position_mmt = j;
                    break;
                }
            }
            ERROR_ASSERT_PRINTF(position_mmt != POSITION_FAIL || free_position_mmt != POSITION_FAIL, "Could not find a free position on the Memory Map Table\n");
            /// Store the write information
            if (position_mmt == POSITION_FAIL) {
                position_mmt = free_position_mmt;
            }
            this->memory_map_table[position_mmt] = &this->reorder_buffer[position_buffer];
        }


        /// Control the Register Dependency - READ
        for (uint32_t k = 0; k < this->reorder_buffer[position_buffer].uop.read_regs.size(); k++) {
            uint32_t read_register = this->reorder_buffer[position_buffer].uop.read_regs[k];
            ERROR_ASSERT_PRINTF(read_register < this->register_alias_table_size, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, this->register_alias_table_size);
            /// If there is a dependency
            if (this->register_alias_table[read_register] != NULL) {
                for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
                    if (this->register_alias_table[read_register]->deps_ptr_array[j] == NULL) {
                        this->register_alias_table[read_register]->deps_ptr_array[j] = &this->reorder_buffer[position_buffer];
                        this->reorder_buffer[position_buffer].wait_deps_number++;
                        break;
                    }
                }
            }
        }

        /// Control the Register Dependency - WRITE
        for (uint32_t k = 0; k < this->reorder_buffer[position_buffer].uop.write_regs.size(); k++) {
            uint32_t write_register = this->reorder_buffer[position_buffer].uop.write_regs[k];
            ERROR_ASSERT_PRINTF(write_register < this->register_alias_table_size, "Read Register (%d) > Register Alias Table Size (%d)\n", write_register, this->register_alias_table_size);
            this->register_alias_table[write_register] = &this->reorder_buffer[position_buffer];
        }
    }
};

//==============================================================================
void processor_t::stage_dispatch() {
    PROCESSOR_DEBUG_PRINTF("stage_dispatch()\n");

    /// Reset the number of instructions dispatched in this cycle
    uint32_t total_dispatched = 0;
    this->fu_int_alu = 0;
    this->fu_int_mul = 0;
    this->fu_int_div = 0;

    this->fu_fp_alu = 0;
    this->fu_fp_mul = 0;
    this->fu_fp_div = 0;

    this->fu_mem_load = 0;
    this->fu_mem_store = 0;

    for (uint32_t k = 0; k < this->unified_reservation_station.size(); k++) {
        reorder_buffer_line_t* reorder_buffer_line = this->unified_reservation_station[k];
        bool dispatched = false;

        /// No other Functional Unit is available
        if (total_dispatched >= this->stage_dispatch_width) {
            break;
        }

        if (reorder_buffer_line->wait_deps_number == 0 &&
        reorder_buffer_line->uop.ready_cycle <= sinuca_engine.get_global_cycle()) {

            ERROR_ASSERT_PRINTF(reorder_buffer_line->stage == PROCESSOR_STAGE_RENAME, "Reservation Station with package not in Rename Stage.\n")
            ERROR_ASSERT_PRINTF(reorder_buffer_line->uop.state == PACKAGE_STATE_READY, "Reservation Station with package not Ready.\n")
            PROCESSOR_DEBUG_PRINTF("\t Dispatching package\n");
            switch (reorder_buffer_line->uop.uop_operation) {
            ///=============================================================
                /// BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    /// Solve the Branch Prediction
                    this->solve_branch(reorder_buffer_line->uop.opcode_number, PROCESSOR_STAGE_DISPATCH);
                /// INTEGERS ALU ===============================================
                case INSTRUCTION_OPERATION_INT_ALU:
                /// NOP
                case INSTRUCTION_OPERATION_NOP:
                /// NOT INDENTIFIED
                case INSTRUCTION_OPERATION_OTHER:
                    if (this->fu_int_alu < this->number_fu_int_alu) {
                        this->stat_dispatch_cycles_fu_int_alu += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_int_alu++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_int_alu);
                    }
                break;
                /// INTEGERS MUL ===============================================
                case INSTRUCTION_OPERATION_INT_MUL:
                    if (this->fu_int_mul < this->number_fu_int_mul) {
                        this->stat_dispatch_cycles_fu_int_mul += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_int_mul++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_int_mul);
                    }
                break;
                /// INTEGERS DIV ===============================================
                case INSTRUCTION_OPERATION_INT_DIV:
                    if (this->fu_int_div < this->number_fu_int_div) {
                        this->stat_dispatch_cycles_fu_int_div += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_int_div++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_int_div);
                    }
                break;

                ///=============================================================
                /// FLOAT POINT ALU ============================================
                case INSTRUCTION_OPERATION_FP_ALU:
                    if (this->fu_fp_alu < this->number_fu_fp_alu) {
                        this->stat_dispatch_cycles_fu_fp_alu += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_fp_alu++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_fp_alu);
                    }
                break;
                /// FLOAT POINT MUL ============================================
                case INSTRUCTION_OPERATION_FP_MUL:
                    if (this->fu_fp_mul < this->number_fu_fp_mul) {
                        this->stat_dispatch_cycles_fu_fp_mul += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_fp_mul++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_fp_mul);
                    }
                break;
                /// FLOAT POINT DIV ============================================
                case INSTRUCTION_OPERATION_FP_DIV:
                    if (this->fu_fp_div < this->number_fu_fp_div) {
                        this->stat_dispatch_cycles_fu_fp_div += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_fp_div++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_fp_div);
                    }
                break;

                ///=============================================================
                /// MEMORY OPERATIONS ==========================================
                case INSTRUCTION_OPERATION_MEM_LOAD:
                    if (this->fu_mem_load < this->number_fu_mem_load) {
                        this->stat_dispatch_cycles_fu_mem_load += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_mem_load++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_mem_load);
                    }
                break;
                case INSTRUCTION_OPERATION_MEM_STORE:
                    if (this->fu_mem_store < this->number_fu_mem_store) {
                        this->stat_dispatch_cycles_fu_mem_store += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;

                        this->fu_mem_store++;
                        dispatched = true;
                        reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                        reorder_buffer_line->uop.package_ready(this->stage_dispatch_cycles + this->latency_fu_mem_store);
                    }
                break;

                case INSTRUCTION_OPERATION_BARRIER:
                    ERROR_PRINTF("Invalid instruction BARRIER being dispatched.\n");
                break;
            }

            if (dispatched == true) {
                total_dispatched++;
                /// Insert into the Functional Unit
                this->unified_functional_units.push_back(reorder_buffer_line);
                /// Remove from the Reservation Station
                this->unified_reservation_station.erase(this->unified_reservation_station.begin() + k);
                k--;
            }
            else {
                PROCESSOR_DEBUG_PRINTF("\t\t Could not insert on functional_unit\n");
            }

        }
    }
};

//==============================================================================
void processor_t::stage_execution() {
    PROCESSOR_DEBUG_PRINTF("stage_execution()\n");
    /// ========================================================================
    /// MEMORY OPERATIONS - READ
    /// ========================================================================
    /// READ_BUFFER(PACKAGE_STATE_READY) =>  REORDER_BUFFER
    int32_t position_mem = memory_package_t::find_old_answer_state_ready(this->read_buffer, this->read_buffer_size, PACKAGE_STATE_READY);
    if (position_mem != POSITION_FAIL) {
        int32_t position_buffer = rob_find_uop_number(this->read_buffer[position_mem].uop_number);
        ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Instruction is executed but it is not on the reorder buffer anymore.\n");
        PROCESSOR_DEBUG_PRINTF("\t Executing package:%s\n", this->reorder_buffer[position_buffer].uop.uop_to_string().c_str());
        this->reorder_buffer[position_buffer].stage = PROCESSOR_STAGE_COMMIT;
        this->reorder_buffer[position_buffer].uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
        this->solve_data_forward(&this->reorder_buffer[position_buffer]);
        this->read_buffer[position_mem].package_clean();
    }

    /// READ_BUFFER(PACKAGE_STATE_TO_LOWER) =>  send_data_package()
    position_mem = memory_package_t::find_old_request_state_ready(this->read_buffer, this->read_buffer_size, PACKAGE_STATE_TRANSMIT);
    if (position_mem != POSITION_FAIL) {
        int32_t transmission_latency = this->send_data_package(&this->read_buffer[position_mem]);
        if (transmission_latency != POSITION_FAIL) {  /// Try to send to the DC.
            this->read_buffer[position_mem].package_wait(transmission_latency);
        }
    }

    /// ========================================================================
    /// MEMORY OPERATIONS - WRITE
    /// ========================================================================
    /// WRITE_BUFFER(PACKAGE_STATE_READY) =>  free()
    position_mem = memory_package_t::find_old_answer_state_ready(this->write_buffer, this->write_buffer_size, PACKAGE_STATE_READY);
    if (position_mem != POSITION_FAIL) {
        this->write_buffer[position_mem].package_clean();
    }

    /// WRITE_BUFFER(PACKAGE_STATE_TO_LOWER) =>  send_data_package()
    position_mem = memory_package_t::find_old_request_state_ready(this->write_buffer, this->write_buffer_size, PACKAGE_STATE_TRANSMIT);
    if (position_mem != POSITION_FAIL) {
        int32_t transmission_latency = this->send_data_package(&this->write_buffer[position_mem]);
        if (transmission_latency != POSITION_FAIL) {  /// Try to send to the DC.
            /// Never wait for answer after SEND a WRITE
            this->write_buffer[position_mem].is_answer = true;
            this->write_buffer[position_mem].package_ready(transmission_latency);
        }
    }

    /// ========================================================================
    /// Commit the executed
    /// ========================================================================
    uint32_t total_executed = 0;
    for (uint32_t k = 0; k < this->unified_functional_units.size(); k++) {
        reorder_buffer_line_t* reorder_buffer_line = this->unified_functional_units[k];

        /// No other Functional Unit is available
        if (total_executed >= this->stage_execution_width) {
            break;
        }

        if (reorder_buffer_line->uop.ready_cycle <= sinuca_engine.get_global_cycle()) {

            ERROR_ASSERT_PRINTF(reorder_buffer_line->stage == PROCESSOR_STAGE_EXECUTION, "Reservation Station with package not in Rename Stage.\n")
            ERROR_ASSERT_PRINTF(reorder_buffer_line->uop.state == PACKAGE_STATE_READY, "Reservation Station with package not Ready.\n")

            switch (reorder_buffer_line->uop.uop_operation) {
            ///=============================================================
                /// BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    /// Solve the Branch Prediction
                    this->solve_branch(reorder_buffer_line->uop.opcode_number, PROCESSOR_STAGE_EXECUTION);
                /// INTEGERS ===============================================
                case INSTRUCTION_OPERATION_INT_ALU:
                case INSTRUCTION_OPERATION_NOP:
                case INSTRUCTION_OPERATION_OTHER:

                case INSTRUCTION_OPERATION_INT_MUL:
                case INSTRUCTION_OPERATION_INT_DIV:
                /// FLOAT POINT ===============================================
                case INSTRUCTION_OPERATION_FP_ALU:
                case INSTRUCTION_OPERATION_FP_MUL:
                case INSTRUCTION_OPERATION_FP_DIV:
                /// INTEGERS ===============================================
                    PROCESSOR_DEBUG_PRINTF("\t Executing package:%s\n", reorder_buffer_line->uop.uop_to_string().c_str());
                    reorder_buffer_line->stage = PROCESSOR_STAGE_COMMIT;
                    reorder_buffer_line->uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
                    this->solve_data_forward(reorder_buffer_line);
                    total_executed++;
                    /// Remove from the Functional Units
                    this->unified_functional_units.erase(this->unified_functional_units.begin() + k);
                    k--;
                break;


                /// FUNC_UNIT_MEM_LOAD => READ_BUFFER
                case INSTRUCTION_OPERATION_MEM_LOAD:
                    position_mem = memory_package_t::find_free(this->read_buffer, this->read_buffer_size);
                    if (position_mem != POSITION_FAIL) {
                        /// Fix the request size to fit inside the cache line
                        uint64_t offset = reorder_buffer_line->uop.memory_address & this->offset_bits_mask;
                        if (offset + reorder_buffer_line->uop.memory_size >= sinuca_engine.get_global_line_size()) {
                            reorder_buffer_line->uop.memory_size = sinuca_engine.get_global_line_size() - offset;
                        }

                        this->read_buffer[position_mem].packager(
                            this->get_id(),                                         /// Request Owner
                            reorder_buffer_line->uop.opcode_number,      /// Opcode. Number
                            reorder_buffer_line->uop.opcode_address,      /// Opcode. Address
                            reorder_buffer_line->uop.uop_number,         /// Uop. Number

                            reorder_buffer_line->uop.memory_address,     /// Mem. Address
                            reorder_buffer_line->uop.memory_size,        /// Block Size

                            PACKAGE_STATE_TRANSMIT,                                 /// Pack. State
                            0,                                                      /// Ready Cycle

                            MEMORY_OPERATION_READ,                                  /// Mem. Operation
                            false,                                                  /// Is Answer

                            this->get_id(),                                                                 /// Src ID
                            this->get_interface_output_component(PROCESSOR_PORT_DATA_CACHE)->get_id(),      /// Dst ID
                            NULL,                          /// *Hops
                            POSITION_FAIL                  /// Hop Counter
                        );
                        reorder_buffer_line->uop.state = PACKAGE_STATE_WAIT;
                        total_executed++;
                        /// Remove from the Functional Units
                        this->unified_functional_units.erase(this->unified_functional_units.begin() + k);
                        k--;
                    }
                break;


                /// FUNC_UNIT_MEM_STORE => WRITE_BUFFER
                case INSTRUCTION_OPERATION_MEM_STORE:
                    position_mem = memory_package_t::find_free(this->write_buffer, this->write_buffer_size);
                    if (position_mem != POSITION_FAIL) {
                        /// Fix the request size to fit inside the cache line
                        uint64_t offset = reorder_buffer_line->uop.memory_address & this->offset_bits_mask;
                        if (offset + reorder_buffer_line->uop.memory_size >= sinuca_engine.get_global_line_size()) {
                            reorder_buffer_line->uop.memory_size = sinuca_engine.get_global_line_size() - offset;
                        }

                        this->write_buffer[position_mem].packager(
                            this->get_id(),                                         /// Request Owner
                            reorder_buffer_line->uop.opcode_number,     /// Opcode. Number
                            reorder_buffer_line->uop.opcode_address,     /// Opcode. Address
                            reorder_buffer_line->uop.uop_number,        /// Uop. Number

                            reorder_buffer_line->uop.memory_address,    /// Mem. Address
                            reorder_buffer_line->uop.memory_size,       /// Block Size

                            PACKAGE_STATE_TRANSMIT,                                 /// Pack. State
                            0,                                                      /// Ready Cycle

                            MEMORY_OPERATION_WRITE,                                 /// Mem. Operation
                            false,                                                  /// Is Answer

                            this->get_id(),                                                                 /// Src ID
                            this->get_interface_output_component(PROCESSOR_PORT_DATA_CACHE)->get_id(),      /// Dst ID
                            NULL,                           /// *Hops
                            POSITION_FAIL                   /// Hop Counter
                        );

                        // INSERTED ON WRITE_BUFFER - reorder_buffer => READY
                        PROCESSOR_DEBUG_PRINTF("\t Executing package:%s\n", reorder_buffer_line->uop.uop_to_string().c_str());
                        reorder_buffer_line->stage = PROCESSOR_STAGE_COMMIT;
                        reorder_buffer_line->uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
                        this->solve_data_forward(reorder_buffer_line);
                        total_executed++;
                        /// Remove from the Functional Units
                        this->unified_functional_units.erase(this->unified_functional_units.begin() + k);
                        k--;
                    }
                break;

                case INSTRUCTION_OPERATION_BARRIER:
                    ERROR_PRINTF("Invalid instruction BARRIER being dispatched.\n");
                break;
            }
        }
    }
};

//==============================================================================
/// Get the route
/// Try to send
///     If OK
///         Get and Return the Latency
///     Else
///         Return POSITION_FAIL
int32_t processor_t::send_data_package(memory_package_t *package) {
    PROCESSOR_DEBUG_PRINTF("send_data_package() package:%s\n", package->memory_to_string().c_str());

    if (this->send_data_ready_cycle <= sinuca_engine.get_global_cycle()) {
        sinuca_engine.interconnection_controller->find_package_route(package);
        ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
        uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
        ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
        package->hop_count--;  /// Consume its own port

        uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package, this, this->get_interface_output_component(output_port));
        bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port), transmission_latency);
        if (sent) {
            PROCESSOR_DEBUG_PRINTF("\tSEND DATA OK\n");
            this->send_data_ready_cycle = sinuca_engine.get_global_cycle() + transmission_latency;
            return transmission_latency;
        }
        else {
            PROCESSOR_DEBUG_PRINTF("\tSEND DATA FAIL\n");
            package->hop_count++;  /// Do not Consume its own port
            return POSITION_FAIL;
        }
    }
    return POSITION_FAIL;
};

//==============================================================================
void processor_t::solve_data_forward(reorder_buffer_line_t *rob_line) {

    /// Remove pointer from Memory Map Table
    if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
        for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
            if (this->memory_map_table[j] != NULL && this->memory_map_table[j]->uop.uop_number == rob_line->uop.uop_number) {
                this->memory_map_table[j] = NULL;
                break;
            }
        }
    }

    /// Remove pointers from Register Alias Table
    for (uint32_t j = 0; j < rob_line->uop.write_regs.size(); j++) {
        uint32_t write_register = rob_line->uop.write_regs[j];
        ERROR_ASSERT_PRINTF(write_register < this->register_alias_table_size, "Read Register (%d) > Register Alias Table Size (%d)\n", write_register, this->register_alias_table_size);
        if (this->register_alias_table[write_register] != NULL && this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number) {
            this->register_alias_table[write_register] = NULL;
        }
    }

    /// Send message to acknowledge the dependency is over
    for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
        /// There is an unsolved dependency
        if (rob_line->deps_ptr_array[j] != NULL) {
            rob_line->deps_ptr_array[j]->wait_deps_number--;
            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            rob_line->deps_ptr_array[j]->uop.ready_cycle = sinuca_engine.get_global_cycle();
            rob_line->deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
    }
};

//==============================================================================
void processor_t::stage_commit() {
    PROCESSOR_DEBUG_PRINTF("stage_commit()\n");
    int32_t position_buffer;

    /// Commit the packages
    for (uint32_t i = 0 ; i < this->stage_commit_width ; i++) {
        position_buffer = this->reorder_buffer_position_start;
        if (this->reorder_buffer_position_used != 0 &&
        this->reorder_buffer[position_buffer].stage == PROCESSOR_STAGE_COMMIT &&
        this->reorder_buffer[position_buffer].uop.state == PACKAGE_STATE_READY &&
        this->reorder_buffer[position_buffer].uop.ready_cycle <= sinuca_engine.get_global_cycle()) {

            this->commit_uop_counter++;
            PROCESSOR_DEBUG_PRINTF("\t Commiting package:%s\n", this->rob_line_to_string(position_buffer).c_str());
            this->add_stat_instruction_read_completed(this->reorder_buffer[position_buffer].uop.born_cycle);

            switch (this->reorder_buffer[position_buffer].uop.uop_operation) {
                // INTEGERS ALU
                case INSTRUCTION_OPERATION_INT_ALU:
                    this->add_stat_int_alu_completed();
                break;

                // INTEGERS MUL
                case INSTRUCTION_OPERATION_INT_MUL:
                    this->add_stat_int_mul_completed();
                break;

                // INTEGERS DIV
                case INSTRUCTION_OPERATION_INT_DIV:
                    this->add_stat_int_div_completed();
                break;

                // FLOAT POINT ALU
                case INSTRUCTION_OPERATION_FP_ALU:
                    this->add_stat_fp_alu_completed();
                break;

                // FLOAT POINT MUL
                case INSTRUCTION_OPERATION_FP_MUL:
                    this->add_stat_fp_mul_completed();
                break;

                // FLOAT POINT DIV
                case INSTRUCTION_OPERATION_FP_DIV:
                    this->add_stat_fp_div_completed();
                break;

                // MEMORY OPERATIONS - READ
                case INSTRUCTION_OPERATION_MEM_LOAD:
                    this->add_stat_memory_read_completed(this->reorder_buffer[position_buffer].uop.born_cycle);
                break;
                // MEMORY OPERATIONS - WRITE
                case INSTRUCTION_OPERATION_MEM_STORE:
                    this->add_stat_memory_write_completed(this->reorder_buffer[position_buffer].uop.born_cycle);
                break;

                // BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    this->add_stat_branch_completed();
                break;

                // NOP
                case INSTRUCTION_OPERATION_NOP:
                    this->add_stat_nop_completed();
                break;

                // NOT INDENTIFIED
                case INSTRUCTION_OPERATION_OTHER:
                    this->add_stat_other_completed();
                break;

                case INSTRUCTION_OPERATION_BARRIER:
                    ERROR_PRINTF("Invalid instruction BARRIER being commited.\n");
                break;
            }

            ERROR_ASSERT_PRINTF(uint32_t(position_buffer) == this->reorder_buffer_position_start, "Commiting different from the position start\n");
            this->rob_remove();
        }
    }
};

//==============================================================================
void processor_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PROCESSOR_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    PROCESSOR_DEBUG_PRINTF("====================\n");
    PROCESSOR_DEBUG_PRINTF("cycle() \n");

    this->branch_predictor.clock(subcycle);

    /// Read from FU
    /// Mark ROB instructions as DONE
    /// Remove ISSUE WIDE oldest instructions
    this->stage_commit();

    /// Read each FU pipeline
    /// Creates latency for each instruction
    /// LOAD/STORES ready, are send to READ/WRITE buffer
    /// After allocate to READ/WRITE buffer, wait only for LOADS
    this->stage_execution();

    /// Read Ready instructions from ROB
    /// Send to free Functional Units
    this->stage_dispatch();

    /// Read from the Decode Buffer
    /// Solve the dependencies
    /// Store on ROB
    this->stage_rename();

    /// Read from the Fetch Buffer
    /// Split Load_Load into 2xLoads
    /// Split Load_Store into 1xLoad + 1xStore
    /// Store on Decode Buffer
    this->stage_decode();

    /// Read from the trace
    /// Send request to ICache
    /// Store on Fetch Buffer, Waiting for ICache
    this->stage_fetch();
};

//==============================================================================
bool processor_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {

    if (this->recv_ready_cycle[input_port] <= sinuca_engine.get_global_cycle()) {
        PROCESSOR_DEBUG_PRINTF("receive_package() port:%u, package:%s\n", input_port, package->memory_to_string().c_str());
        ERROR_ASSERT_PRINTF(package->id_owner == this->get_id(), "Received some package for a different owner.\n");
        ERROR_ASSERT_PRINTF(package->id_dst == this->get_id(), "Received some package for a different id_dst.\n");
        ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");
        ERROR_ASSERT_PRINTF(package->is_answer == true, "Only answers are expected.\n");

        int32_t slot = POSITION_FAIL;

        switch (package->memory_operation) {
            case MEMORY_OPERATION_INST:
                ERROR_ASSERT_PRINTF(input_port == PROCESSOR_PORT_INST_CACHE, "Receiving instruction package from a wrong port.\n");

                /// Add to the buffer the whole line fetched
                this->fetch_pc_line_buffer = package->memory_address;

                /// Find packages WAITING
                for (uint32_t k = 0; k < this->fetch_buffer_position_used; k++) {
                    /// Update the FETCH BUFFER position
                    uint32_t i = this->fetch_buffer_position_start + k;
                    if (i >= this->fetch_buffer_size) {
                        i -= this->fetch_buffer_size;
                    }

                    /// Wake up ALL instructions waiting
                    if (this->fetch_buffer[i].state == PACKAGE_STATE_WAIT && this->cmp_index_tag(this->fetch_buffer[i].opcode_address, package->memory_address)) {
                        PROCESSOR_DEBUG_PRINTF("\t WANTED INSTRUCTION\n");
                        this->fetch_buffer[i].package_ready(transmission_latency);
                        slot = i;
                    }
                }
                ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Processor Read Instruction done, but it is not on the fetch-buffer anymore.\n")
                this->recv_ready_cycle[input_port] = sinuca_engine.get_global_cycle() + transmission_latency;
                return OK;
            break;

            case MEMORY_OPERATION_READ:
                ERROR_ASSERT_PRINTF(input_port == PROCESSOR_PORT_DATA_CACHE, "Receiving read package from a wrong port.\n");

                slot = memory_package_t::find_state_mem_address(this->read_buffer, this->read_buffer_size, PACKAGE_STATE_WAIT, package->memory_address);
                ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Processor Read done, but it is not on the read-buffer anymore.\n")

                PROCESSOR_DEBUG_PRINTF("\t WANTED READ.\n");
                this->read_buffer[slot].is_answer = true;
                this->read_buffer[slot].package_ready(transmission_latency);
                this->recv_ready_cycle[input_port] = sinuca_engine.get_global_cycle() + transmission_latency;
                return OK;
            break;

            case MEMORY_OPERATION_WRITE:
            case MEMORY_OPERATION_COPYBACK:
            case MEMORY_OPERATION_PREFETCH:
                ERROR_PRINTF("Processor receiving %s.\n", get_enum_memory_operation_char(package->memory_operation))
                return FAIL;
            break;
        }
        ERROR_PRINTF("Processor receiving %s.\n", get_enum_memory_operation_char(package->memory_operation))
        return FAIL;
    }
    return FAIL;
};

//==============================================================================
void processor_t::print_structures() {


    SINUCA_PRINTF("%s BRANCH_SOLVE_STAGE:%s  BRANCH_OPCODE_NUMBER:%"PRIu64"\n", this->get_label(), get_enum_processor_stage_char(this->branch_solve_stage), this->branch_opcode_number);
    SINUCA_PRINTF("%s SYNC_STATUS:%s\n", this->get_label(), get_enum_sync_char(this->sync_status));

    SINUCA_PRINTF("%s FETCH_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->fetch_buffer_position_start, this->fetch_buffer_position_end, this->fetch_buffer_position_used);
    SINUCA_PRINTF("%s FETCH_BUFFER:\n%s", this->get_label(), opcode_package_t::print_all(this->fetch_buffer, this->fetch_buffer_size).c_str());

    SINUCA_PRINTF("%s DECODE_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->decode_buffer_position_start, this->decode_buffer_position_end, this->decode_buffer_position_used);
    SINUCA_PRINTF("%s DECODE_BUFFER:\n%s", this->get_label(), uop_package_t::print_all(this->decode_buffer, this->decode_buffer_size).c_str());

    SINUCA_PRINTF("%s REORDER_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->reorder_buffer_position_start, this->reorder_buffer_position_end, this->reorder_buffer_position_used);
    SINUCA_PRINTF("%s REORDER_BUFFER:\n%s",  this->get_label(), rob_print_all().c_str());

    /// Integer Functional Units
    // ~ SINUCA_PRINTF("FUNC_UNIT_INT_ALU:\n%s", uop_package_t::print_all(this->fu_int_alu, this->number_fu_int_alu, latency_fu_int_alu).c_str());
    // ~ SINUCA_PRINTF("FUNC_UNIT_INT_MUL:\n%s", uop_package_t::print_all(this->fu_int_mul, this->number_fu_int_mul, latency_fu_int_mul).c_str());
    // ~ SINUCA_PRINTF("FUNC_UNIT_INT_DIV:\n%s", uop_package_t::print_all(this->fu_int_div, this->number_fu_int_div, latency_fu_int_div).c_str());
    /// Floating Point Functional Units
    // ~ SINUCA_PRINTF("FUNC_UNIT_FP_ALU:\n%s", uop_package_t::print_all(this->fu_fp_alu, this->number_fu_fp_alu, latency_fu_fp_alu).c_str());
    // ~ SINUCA_PRINTF("FUNC_UNIT_FP_MUL:\n%s", uop_package_t::print_all(this->fu_fp_mul, this->number_fu_fp_mul, latency_fu_fp_mul).c_str());
    // ~ SINUCA_PRINTF("FUNC_UNIT_FP_DIV:\n%s", uop_package_t::print_all(this->fu_fp_div, this->number_fu_fp_div, latency_fu_fp_div).c_str());
    /// Memory Functional Units
    // ~ SINUCA_PRINTF("FUNC_UNIT_MEM_LOAD:\n%s", uop_package_t::print_all(this->fu_mem_load, this->number_fu_mem_load, latency_fu_mem_load).c_str());
    // ~ SINUCA_PRINTF("FUNC_UNIT_MEM_STORE:\n%s", uop_package_t::print_all(this->fu_mem_store, this->number_fu_mem_store, latency_fu_mem_store).c_str());

    SINUCA_PRINTF("%s READ_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->read_buffer, this->read_buffer_size).c_str());
    SINUCA_PRINTF("%s WRITE_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->write_buffer, this->write_buffer_size).c_str());
};

// =============================================================================
void processor_t::panic() {
    this->print_structures();
    this->branch_predictor.panic();
};

//==============================================================================
void processor_t::periodic_check(){
    #ifdef PROCESSOR_DEBUG
        PROCESSOR_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(this->rob_check_age() == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(uop_package_t::check_age(this->decode_buffer, this->decode_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(opcode_package_t::check_age(this->fetch_buffer, this->fetch_buffer_size) == OK, "Check_age failed.\n");

    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->read_buffer, this->read_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->write_buffer, this->write_buffer_size) == OK, "Check_age failed.\n");
    this->branch_predictor.periodic_check();
};

//==============================================================================
// STATISTICS
//==============================================================================
void processor_t::reset_statistics() {

    this->set_stat_reset_fetch_opcode_counter(this->fetch_opcode_counter);
    this->set_stat_reset_decode_uop_counter(this->decode_uop_counter);

    this->set_stat_branch_stall_cycles(0);
    this->set_stat_sync_stall_cycles(0);

    /// Executed Instructions
    this->set_stat_nop_completed(0);
    this->set_stat_branch_completed(0);
    this->set_stat_other_completed(0);

    this->set_stat_int_alu_completed(0);
    this->set_stat_int_mul_completed(0);
    this->set_stat_int_div_completed(0);

    this->set_stat_fp_alu_completed(0);
    this->set_stat_fp_mul_completed(0);
    this->set_stat_fp_div_completed(0);

    this->set_stat_memory_read_completed(0);
    this->set_stat_instruction_read_completed(0);
    this->set_stat_memory_write_completed(0);

    /// Dispatch Cycles Stall
    this->set_stat_dispatch_cycles_fu_int_alu(0);
    this->set_stat_dispatch_cycles_fu_int_mul(0);
    this->set_stat_dispatch_cycles_fu_int_div(0);

    this->set_stat_dispatch_cycles_fu_fp_alu(0);
    this->set_stat_dispatch_cycles_fu_fp_mul(0);
    this->set_stat_dispatch_cycles_fu_fp_div(0);

    this->set_stat_dispatch_cycles_fu_mem_load(0);
    this->set_stat_dispatch_cycles_fu_mem_store(0);

    /// Memory Cycles Stall
    this->stat_min_instruction_read_wait_time = MAX_ALIVE_TIME;
    this->stat_max_instruction_read_wait_time = 0;
    this->stat_acumulated_instruction_read_wait_time = 0;

    this->stat_min_memory_read_wait_time = MAX_ALIVE_TIME;
    this->stat_max_memory_read_wait_time = 0;
    this->stat_acumulated_memory_read_wait_time = 0;

    this->stat_min_memory_write_wait_time = MAX_ALIVE_TIME;
    this->stat_max_memory_write_wait_time = 0;
    this->stat_acumulated_memory_write_wait_time = 0;

    this->branch_predictor.reset_statistics();
    return;
};

//==============================================================================
void processor_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fetch_opcode_counter", this->fetch_opcode_counter);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "decode_uop_counter", this->decode_uop_counter);

    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "opcode_per_cycle_ratio_total", this->fetch_opcode_counter, sinuca_engine.get_global_cycle());
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "uop_per_cycle_ratio_total", this->decode_uop_counter, sinuca_engine.get_global_cycle());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fetch_opcode_counter_warm", this->fetch_opcode_counter - this->stat_reset_fetch_opcode_counter);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "decode_uop_counter_warm", this->decode_uop_counter - this->stat_reset_decode_uop_counter);

    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "opcode_per_cycle_ratio_warm", this->fetch_opcode_counter - this->stat_reset_fetch_opcode_counter,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "uop_per_cycle_ratio_warm", this->decode_uop_counter - this->stat_reset_decode_uop_counter,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sync_stall_cycles", stat_sync_stall_cycles);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_sync_stall_cycles_ratio_warm", this->stat_sync_stall_cycles,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());

    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "opcode_per_cycle_ratio_warm_nosync", this->fetch_opcode_counter - this->stat_reset_fetch_opcode_counter,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle() - stat_sync_stall_cycles);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "uop_per_cycle_ratio_warm_nosync", this->decode_uop_counter - this->stat_reset_decode_uop_counter,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle() - stat_sync_stall_cycles);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_stall_cycles", stat_branch_stall_cycles);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_branch_stall_cycles_ratio_warm", this->stat_branch_stall_cycles,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());

    /// Executed Instructions
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_nop_completed", stat_nop_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_completed", stat_branch_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_other_completed", stat_other_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_int_alu_completed", stat_int_alu_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_int_mul_completed", stat_int_mul_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_int_div_completed", stat_int_div_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_fp_alu_completed", stat_fp_alu_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_fp_mul_completed", stat_fp_mul_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_fp_div_completed", stat_fp_div_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_read_completed", stat_instruction_read_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_memory_read_completed", stat_memory_read_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_memory_write_completed", stat_memory_write_completed);

    /// Dispatch Cycles Stall
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_alu", stat_dispatch_cycles_fu_int_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_mul", stat_dispatch_cycles_fu_int_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_div", stat_dispatch_cycles_fu_int_div);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_alu", stat_dispatch_cycles_fu_fp_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_mul", stat_dispatch_cycles_fu_fp_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_div", stat_dispatch_cycles_fu_fp_div);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_mem_load", stat_dispatch_cycles_fu_mem_load);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_mem_store", stat_dispatch_cycles_fu_mem_store);

    /// Dispatch Cycles Stall per Uop
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_alu_ratio", stat_dispatch_cycles_fu_int_alu, stat_int_alu_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_mul_ratio", stat_dispatch_cycles_fu_int_mul, stat_int_mul_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_int_div_ratio", stat_dispatch_cycles_fu_int_div, stat_int_div_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_alu_ratio", stat_dispatch_cycles_fu_fp_alu, stat_fp_alu_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_mul_ratio", stat_dispatch_cycles_fu_fp_mul, stat_fp_mul_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_fp_div_ratio", stat_dispatch_cycles_fu_fp_div, stat_fp_div_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_mem_load_ratio", stat_dispatch_cycles_fu_mem_load, stat_memory_read_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_dispatch_cycles_fu_mem_store_ratio", stat_dispatch_cycles_fu_mem_store, stat_memory_write_completed);

    /// Memory Cycles Stall
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_instruction_read_wait_time", stat_min_instruction_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_instruction_read_wait_time", stat_max_instruction_read_wait_time);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_memory_read_wait_time", stat_min_memory_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_memory_read_wait_time", stat_max_memory_read_wait_time);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_memory_write_wait_time", stat_min_memory_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_memory_write_wait_time", stat_max_memory_write_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_instruction_read_wait_time_ratio",stat_acumulated_instruction_read_wait_time, stat_instruction_read_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_memory_read_wait_time_ratio",stat_acumulated_memory_read_wait_time, stat_memory_read_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_memory_write_wait_time_ratio",stat_acumulated_memory_write_wait_time, stat_memory_write_completed);

    this->branch_predictor.print_statistics();
};

//==============================================================================
void processor_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "core_id", core_id);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_latency", this->get_interconnection_latency());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_width", this->get_interconnection_width());

    sinuca_engine.write_statistics_small_separator();
    /// Buffers' Size
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fetch_buffer_size", fetch_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "reorder_buffer_size", reorder_buffer_size);

    /// Stages' Latency
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_fetch_cycles", stage_fetch_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_decode_cycles", stage_decode_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_rename_cycles", stage_rename_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_dispatch_cycles", stage_dispatch_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_execution_cycles", stage_execution_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_commit_cycles", stage_commit_cycles);


    /// Stages' Width
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_fetch_width", stage_fetch_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_decode_width", stage_decode_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_rename_width", stage_rename_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_dispatch_width", stage_dispatch_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_execution_width", stage_execution_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stage_commit_width", stage_commit_width);


    /// Integer Functional Units
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_int_alu", number_fu_int_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_int_alu", latency_fu_int_alu);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_int_mul", number_fu_int_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_int_mul", latency_fu_int_mul);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_int_div", number_fu_int_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_int_div", latency_fu_int_div);


    /// Floating Point Functional Units
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_alu", number_fu_fp_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_alu", latency_fu_fp_alu);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_mul", number_fu_fp_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_mul", latency_fu_fp_mul);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_div", number_fu_fp_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_div", latency_fu_fp_div);

    /// Memory Functional Units
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_mem_load", number_fu_mem_load);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_mem_load", latency_fu_mem_load);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_mem_store", number_fu_mem_store);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_mem_store", latency_fu_mem_store);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "read_buffer_size", read_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_buffer_size", write_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "offset_bits_mask", utils_t::address_to_binary(this->offset_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "not_offset_bits_mask", utils_t::address_to_binary(this->not_offset_bits_mask).c_str());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "branch_per_fetch", branch_per_fetch);

    this->branch_predictor.print_configuration();
};

//==============================================================================
// Fetch Buffer Methods
//==============================================================================
/*! Should make all the verifications before call this method, because it will
 * update the position_end and position_used for the fetch_buffer
 */
int32_t processor_t::fetch_buffer_insert() {
    int32_t valid_position = POSITION_FAIL;
    /// There is free space.
    if (this->fetch_buffer_position_used < this->fetch_buffer_size) {
        valid_position = this->fetch_buffer_position_end;
        this->fetch_buffer_position_used++;
        this->fetch_buffer_position_end++;
        if (this->fetch_buffer_position_end >= this->fetch_buffer_size) {
            this->fetch_buffer_position_end = 0;
        }
    }
    return valid_position;
};

//==============================================================================
/*! Make sure that you want to remove the first element before call this method
 * because it will remove the buffer[position_start] and update the controls
 */
void processor_t::fetch_buffer_remove() {
    ERROR_ASSERT_PRINTF(this->fetch_buffer_position_used > 0, "Trying to remove from fetch_buffer with no used position.\n");

    this->fetch_buffer[this->fetch_buffer_position_start].package_clean();

    this->fetch_buffer_position_used--;
    this->fetch_buffer_position_start++;
    if (this->fetch_buffer_position_start >= this->fetch_buffer_size) {
        this->fetch_buffer_position_start = 0;
    }
};

//==============================================================================
int32_t processor_t::fetch_buffer_find_opcode_number(uint64_t opcode_number) {
    uint32_t start_position;
    for (uint32_t i = 0; i < this->fetch_buffer_position_used; i++) {
        start_position = this->fetch_buffer_position_start + i;
        if (start_position >= this->fetch_buffer_size) {
            start_position -= this->fetch_buffer_size;
        }

        if (this->fetch_buffer[start_position].opcode_number == opcode_number) {
            return start_position;
        }
    }
    return POSITION_FAIL;
};

//==============================================================================
// Decode Buffer Methods
//==============================================================================
/*! Should make all the verifications before call this method, because it will
 * update the position_end and position_used for the decode_buffer
 */
int32_t processor_t::decode_buffer_insert() {
    int32_t valid_position = POSITION_FAIL;
    /// There is free space.
    if (this->decode_buffer_position_used < this->decode_buffer_size) {
        valid_position = this->decode_buffer_position_end;
        this->decode_buffer_position_used++;
        this->decode_buffer_position_end++;
        if (this->decode_buffer_position_end >= this->decode_buffer_size) {
            this->decode_buffer_position_end = 0;
        }
    }
    return valid_position;
};

//==============================================================================
/*! Make sure that you want to remove the first element before call this method
 * because it will remove the buffer[position_start] and update the controls
 */
void processor_t::decode_buffer_remove() {
    ERROR_ASSERT_PRINTF(this->decode_buffer_position_used > 0, "Trying to remove from decode_buffer with no used position.\n");

    this->decode_buffer[this->decode_buffer_position_start].package_clean();

    this->decode_buffer_position_used--;
    this->decode_buffer_position_start++;
    if (this->decode_buffer_position_start >= this->decode_buffer_size) {
        this->decode_buffer_position_start = 0;
    }
};


//==============================================================================
// Reorder Buffer Methods
//==============================================================================
std::string processor_t::rob_line_to_string(uint32_t reorder_buffer_line) {
    std::string PackageString;
    PackageString = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->reorder_buffer[reorder_buffer_line].uop.state == PACKAGE_STATE_FREE) {
            return PackageString;
        }
    #endif

    PackageString = this->reorder_buffer[reorder_buffer_line].uop.uop_to_string();
    PackageString = PackageString + " | STAGE:" + get_enum_processor_stage_char(this->reorder_buffer[reorder_buffer_line].stage);
    PackageString = PackageString + " | #WAIT:" + utils_t::uint32_to_char(this->reorder_buffer[reorder_buffer_line].wait_deps_number);

    return PackageString;
};

//==============================================================================
std::string processor_t::rob_print_all() {
    std::string PackageString;
    std::string FinalString;
    PackageString = "";
    FinalString = "";

    for (uint32_t i = 0; i < this->reorder_buffer_size ; i++) {
        PackageString = this->rob_line_to_string(i);
        if (PackageString.size() > 1) {
            FinalString = FinalString + "[" + utils_t::uint32_to_char(i) + "] " + PackageString + "\n";
        }
    }
    return FinalString;
};

//==============================================================================
int32_t processor_t::rob_insert() {
    int32_t valid_position = POSITION_FAIL;
    /// There is free space.
    if (this->reorder_buffer_position_used < this->reorder_buffer_size) {
        valid_position = this->reorder_buffer_position_end;
        this->reorder_buffer_position_used++;
        this->reorder_buffer_position_end++;
        if (this->reorder_buffer_position_end >= this->reorder_buffer_size) {
            this->reorder_buffer_position_end = 0;
        }
    }
    return valid_position;
};

//==============================================================================
void processor_t::rob_remove() {
    ERROR_ASSERT_PRINTF(this->reorder_buffer_position_used > 0, "Trying to remove from ROB with no used position.\n");
    ERROR_ASSERT_PRINTF(this->reorder_buffer[this->reorder_buffer_position_start].deps_ptr_array[0] == NULL, "Removing from ROB without solve the dependencies.\n");

    this->reorder_buffer[this->reorder_buffer_position_start].uop.package_clean();
    this->reorder_buffer[this->reorder_buffer_position_start].stage = PROCESSOR_STAGE_DECODE;
    this->reorder_buffer[this->reorder_buffer_position_start].wait_deps_number = 0;

    this->reorder_buffer_position_used--;
    this->reorder_buffer_position_start++;
    if (this->reorder_buffer_position_start >= this->reorder_buffer_size) {
        this->reorder_buffer_position_start = 0;
    }
};

//==============================================================================
int32_t processor_t::rob_find_uop_number(uint64_t uop_number) {
    uint32_t start_position;
    for (uint32_t i = 0; i < this->reorder_buffer_position_used; i++) {
        start_position = this->reorder_buffer_position_start + i;
        if (start_position >= this->reorder_buffer_size) {
            start_position -= this->reorder_buffer_size;
        }

        if (this->reorder_buffer[start_position].uop.uop_number == uop_number) {
            return start_position;
        }
    }
    return POSITION_FAIL;
};

//==============================================================================
bool processor_t::rob_check_age() {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < this->reorder_buffer_size ; i++) {
        if (this->reorder_buffer[i].uop.state != PACKAGE_STATE_FREE && this->reorder_buffer[i].uop.born_cycle < min_cycle) {
            return FAIL;
        }
    }
    return OK;
};
