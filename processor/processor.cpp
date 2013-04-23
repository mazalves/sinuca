/// ============================================================================
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
/// ============================================================================
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

/// ============================================================================
processor_t::processor_t() {
    this->set_type_component(COMPONENT_PROCESSOR);

    this->branch_predictor = NULL;
    /// Synchronization Control Variables
    this->sync_status = SYNC_FREE;          /// Used to sync or request sync
    this->sync_status_time = 0;             /// Last time the sync changed
    this->trace_over = false;

    /// Branch Prediction Control Variables
    this->branch_solve_stage = PROCESSOR_STAGE_FETCH;
    this->branch_opcode_number = 0;
    this->inflight_branches = 0;
    this->inflight_branches_size = 0;
    /// Branch flush latency
    this->branch_flush_latency = 0;
    this->branch_flush_cycle_ready = 0;

    /// Stages Control Variables
    this->trace_next_opcode.package_clean();
    this->fetch_opcode_address = 0;                     /// Last PC requested to IC
    this->fetch_opcode_address_line_buffer = 0;         /// Last PC answered by IC

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

    /// ====================================================================
    /// Integer Functional Units
    this->ready_cycle_fu_int_alu = NULL;
    this->ready_cycle_fu_int_mul = NULL;
    this->ready_cycle_fu_int_div = NULL;

    /// ====================================================================
    /// Floating Point Functional Units
    this->ready_cycle_fu_fp_alu = NULL;
    this->ready_cycle_fu_fp_mul = NULL;
    this->ready_cycle_fu_fp_div = NULL;

    /// ====================================================================
    /// Memory Functional Units
    this->ready_cycle_fu_mem_load = NULL;
    this->ready_cycle_fu_mem_store = NULL;


    /// Integer Funcional Units
    this->number_fu_int_alu = 0;
    this->latency_fu_int_alu = 0;
    this->wait_between_fu_int_alu = 0;

    this->number_fu_int_mul = 0;
    this->latency_fu_int_mul = 0;
    this->wait_between_fu_int_mul = 0;

    this->number_fu_int_div = 0;
    this->latency_fu_int_div = 0;
    this->wait_between_fu_int_div = 0;

    /// Floating Point Functional Units
    this->number_fu_fp_alu = 0;
    this->latency_fu_fp_alu = 0;
    this->wait_between_fu_fp_alu = 0;

    this->number_fu_fp_mul = 0;
    this->latency_fu_fp_mul = 0;
    this->wait_between_fu_fp_mul = 0;

    this->number_fu_fp_div = 0;
    this->latency_fu_fp_div = 0;
    this->wait_between_fu_fp_div = 0;

    /// Memory Functional Units
    this->number_fu_mem_load = 0;
    this->latency_fu_mem_load = 0;
    this->wait_between_fu_mem_load = 0;

    this->number_fu_mem_store = 0;
    this->latency_fu_mem_store = 0;
    this->wait_between_fu_mem_store = 0;

    this->recv_ready_cycle = NULL;

    this->memory_order_buffer_read_size = 0;
    this->memory_order_buffer_write_size = 0;
    this->memory_order_buffer_read = NULL;
    this->memory_order_buffer_write = NULL;
    this->memory_order_buffer_read_used = 0;
    this->memory_order_buffer_write_used = 0;


    this->disambiguation_block_size = 0;
    this->disambiguation_type = DISAMBIGUATION_DISABLE;

    this->register_alias_table_size = 0;
    this->register_alias_table = NULL;

    this->branch_per_fetch = 0;
    this->unified_reservation_station_window_size = 0;

    this->data_cache = NULL;
    this->inst_cache = NULL;
};

/// ============================================================================
processor_t::~processor_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint64_t>(recv_ready_cycle);

    utils_t::template_delete_array<opcode_package_t>(fetch_buffer);
    utils_t::template_delete_array<uop_package_t>(decode_buffer);
    utils_t::template_delete_array<reorder_buffer_line_t>(reorder_buffer);

    utils_t::template_delete_array<reorder_buffer_line_t*>(register_alias_table);

    utils_t::template_delete_array<memory_order_buffer_line_t>(memory_order_buffer_read);
    utils_t::template_delete_array<memory_order_buffer_line_t>(memory_order_buffer_write);
};

/// ============================================================================
void processor_t::allocate() {
    PROCESSOR_DEBUG_PRINTF("allocate()\n");

    ERROR_ASSERT_PRINTF(MAX_REGISTERS >= 2, "MAX_REGISTERS should be greater than 2.\n")

    this->trace_next_opcode.package_clean();

    this->recv_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_max_ports(), 0);

    ERROR_ASSERT_PRINTF(this->number_fu_int_alu > 0 && this->wait_between_fu_int_alu > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(this->number_fu_int_mul > 0 && this->wait_between_fu_int_mul > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(this->number_fu_int_div > 0 && this->wait_between_fu_int_div > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");

    ERROR_ASSERT_PRINTF(this->number_fu_fp_alu > 0 && this->wait_between_fu_fp_alu > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(this->number_fu_fp_mul > 0 && this->wait_between_fu_fp_mul > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(this->number_fu_fp_div > 0 && this->wait_between_fu_fp_div > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");

    ERROR_ASSERT_PRINTF(this->number_fu_mem_load > 0 && this->wait_between_fu_mem_load > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(this->number_fu_mem_store > 0 && this->wait_between_fu_mem_store > 0, "Functional units / Wait betweeen requests should be bigger than zero.\n");

    /// ====================================================================
    /// Integer Functional Units
    this->ready_cycle_fu_int_alu = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_int_alu, 0);
    this->ready_cycle_fu_int_mul = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_int_mul, 0);
    this->ready_cycle_fu_int_div = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_int_div, 0);

    /// ====================================================================
    /// Floating Point Functional Units
    this->ready_cycle_fu_fp_alu = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_fp_alu, 0);
    this->ready_cycle_fu_fp_mul = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_fp_mul, 0);
    this->ready_cycle_fu_fp_div = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_fp_div, 0);

    /// ====================================================================
    /// Memory Functional Units
    this->ready_cycle_fu_mem_load = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_mem_load, 0);
    this->ready_cycle_fu_mem_store = utils_t::template_allocate_initialize_array<uint64_t>(this->number_fu_mem_store, 0);


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
        this->reorder_buffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(this->reorder_buffer_size, NULL);
    }
    this->reorder_buffer_position_start = 0;
    this->reorder_buffer_position_end = 0;
    this->reorder_buffer_position_used = 0;

    this->unified_functional_units.reserve(this->reorder_buffer_size);
    this->unified_reservation_station.reserve(this->reorder_buffer_size);

    /// Register Alias Table for Renaming
    this->register_alias_table_size = 260; /// Number of registers on the trace (258 is used for SiNUCA only maintain the dependency between uops)
    this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(this->register_alias_table_size, NULL);

    /// Propagate the allocate()
    this->branch_predictor->allocate();


    ERROR_ASSERT_PRINTF(this->branch_per_fetch > 0, "Maximum number of branches per fetch must be greater than 0.\n")

    /// Functional Units
    uint32_t total_dispatched = this->number_fu_int_alu + this->number_fu_int_mul + this->number_fu_int_div
                            + this->number_fu_fp_alu + this->number_fu_fp_mul + this->number_fu_fp_div
                            + this->number_fu_mem_load + this->number_fu_mem_store;
    ERROR_ASSERT_PRINTF(this->stage_dispatch_width <= total_dispatched, "Dispatch width must be less or equal to the number of functional units (%u).\n", total_dispatched);
    ERROR_ASSERT_PRINTF(this->stage_execution_width <= total_dispatched, "Execution width must be less or equal to the number of functional units (%u).\n", total_dispatched);

    this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(this->memory_order_buffer_read_size);
    for (uint32_t i = 0; i < this->memory_order_buffer_read_size; i++) {
        this->memory_order_buffer_read[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(this->reorder_buffer_size, NULL);
    }

    this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(this->memory_order_buffer_write_size);
    for (uint32_t i = 0; i < this->memory_order_buffer_write_size; i++) {
        this->memory_order_buffer_write[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(this->reorder_buffer_size, NULL);
    }

    /// OFFSET MASK
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size()), "Wrong line_size.\n")
    this->offset_bits_mask = 0;
    this->not_offset_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(sinuca_engine.get_global_line_size()); i++) {
        this->offset_bits_mask |= 1 << i;
    }
    this->not_offset_bits_mask = ~offset_bits_mask;

    /// FETCH OFFSET MASK
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->fetch_block_size), "Wrong fetch_block_size.\n")
    this->fetch_offset_bits_mask = 0;
    this->not_fetch_offset_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->fetch_block_size); i++) {
        this->fetch_offset_bits_mask |= 1 << i;
    }
    this->not_fetch_offset_bits_mask = ~fetch_offset_bits_mask;

    /// DISAMBIGUATION OFFSET MASK
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->disambiguation_block_size), "Wrong disambiguation_block_size.\n")
    this->disambiguation_offset_bits_mask = 0;
    this->not_disambiguation_offset_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->disambiguation_block_size); i++) {
        this->disambiguation_offset_bits_mask |= 1 << i;
    }
    this->not_disambiguation_offset_bits_mask = ~disambiguation_offset_bits_mask;

    char label[50] = "";
    sprintf(label, "%s_BRANCH_PREDICTOR", this->get_label());
    this->branch_predictor->set_label(label);
    this->branch_predictor->allocate();

    #ifdef PROCESSOR_DEBUG
        this->print_configuration();
    #endif
};

/// ============================================================================
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
            /// If the system is here, the critical_end was already propagated.
            if (this->sync_status != SYNC_CRITICAL_START) {
                WARNING_PRINTF("While processor[%u] synchronize[%s], found on processor[%d] sync_status[%s]\n",
                                this->get_core_id(), get_enum_sync_char(this->sync_status), this->get_core_id(), get_enum_sync_char(new_sync));
                this->sync_status = SYNC_FREE;
                this->sync_status_time = sinuca_engine.get_global_cycle();
            }
            /// System is normal, propagate the critical_end
            else {
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
        }
        break;

        /// ====================================================================
        /// IF all other has (SYNC_BARRIER)     =>  WAKE UP ALL & CONTINUE
        /// ELSE                                =>  WAIT
        case SYNC_BARRIER: {
            SYNC_DEBUG_PRINTF("SYNC_BARRIER => ");
            /// System should not be here, but it may be recoverable
            /// The trace has:      atomic_start    -> barrier      -> atomic_end
            /// Now treating as:    atomic_start    -> atomic_end   -> barrier
            if (this->sync_status != SYNC_FREE) {
                WARNING_PRINTF("While processor[%u] synchronize[%s], found on processor[%d] sync_status[%s]\n",
                                this->get_core_id(), get_enum_sync_char(this->sync_status), this->get_core_id(), get_enum_sync_char(new_sync));
                switch (this->sync_status) {
                    case SYNC_CRITICAL_START:
                        /// Propagate the critical_end
                        WARNING_PRINTF("Propagating synchronize[%s] to avoid dead-lock.\n", get_enum_sync_char(SYNC_CRITICAL_END));
                        this->synchronize(SYNC_CRITICAL_END);
                    break;

                    case SYNC_CRITICAL_END:
                    case SYNC_BARRIER:
                    case SYNC_WAIT_CRITICAL_START:
                    case SYNC_FREE:
                        ERROR_PRINTF("Processor[%d] should not receive synchronize[%s]\n",
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
            ERROR_PRINTF("Processor[%d] should not receive synchronize[%s]\n",
                        this->get_core_id(), get_enum_sync_char(new_sync));
        break;
    }


    SYNC_DEBUG_PRINTF(" -- All Status After:");
    for (uint32_t proc = 0; proc < sinuca_engine.get_processor_array_size(); proc++) {
        SYNC_DEBUG_PRINTF("[%s]", get_enum_sync_char(sinuca_processor[proc]->sync_status))
    }
    SYNC_DEBUG_PRINTF("\n");
};

/// ============================================================================
void processor_t::solve_branch(uint64_t opcode_number, processor_stage_t processor_stage, instruction_operation_t operation) {
    ERROR_ASSERT_PRINTF(operation == INSTRUCTION_OPERATION_BRANCH, "Asking to solve branch with non branch instruction")

    /// Control the total of inflight instructions
    if (processor_stage == PROCESSOR_STAGE_EXECUTION) {
        ERROR_ASSERT_PRINTF(this->inflight_branches > 0, "Decreasing inflight branches to -1")
        this->inflight_branches--;
    }

    if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH &&
        this->branch_solve_stage == processor_stage &&
        this->branch_opcode_number == opcode_number) {
            this->branch_solve_stage = PROCESSOR_STAGE_FETCH;
            this->branch_flush_cycle_ready = sinuca_engine.get_global_cycle() + this->branch_flush_latency;
    }
};

/// ============================================================================
void processor_t::stage_fetch() {
    PROCESSOR_DEBUG_PRINTF("stage_fetch()\n");
    int32_t position_buffer = -1;
    uint32_t i;

    /// 1st. Trace => Fetch_Buffer
    bool valid_opcode = false;
    uint32_t count_branches = 0;

    for (i = 0; i < this->stage_fetch_width; i++) {
        /// If the trace is over
        // ~ if (this->trace_over) {
            // ~ this->synchronize(SYNC_BARRIER);
            // ~ break;
        // ~ }


        /// If must wait Branch miss prediction
        if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH ||
        this->branch_flush_cycle_ready >= sinuca_engine.get_global_cycle()) {
            add_stat_branch_stall_cycles();
            break;
        }

        /// If must wait synchronization
        if (this->sync_status != SYNC_FREE && this->sync_status != SYNC_CRITICAL_START) {
            add_stat_sync_stall_cycles();
            break;
        }

        /// Get the next opcode
        if (this->trace_next_opcode.state == PACKAGE_STATE_FREE) {
            valid_opcode = sinuca_engine.trace_reader->trace_fetch(this->core_id, &this->trace_next_opcode);
            /// If the trace is over
            if (!valid_opcode) {
                this->trace_over = true;
                this->synchronize(SYNC_BARRIER);
                break;
            }
        }

        /// If the instruction is a synchronization
        if (this->trace_next_opcode.sync_type != SYNC_FREE) {
            this->synchronize(this->trace_next_opcode.sync_type);
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
            this->add_stat_full_fetch_buffer();
            break;
        }

        PROCESSOR_DEBUG_PRINTF("\t Inserting on fetch_buffer[%d] package:%s\n", position_buffer, trace_next_opcode.content_to_string().c_str());
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
        this->branch_solve_stage = this->branch_predictor->predict_branch(this->fetch_buffer[position_buffer], this->trace_next_opcode);
        this->branch_opcode_number = this->fetch_buffer[position_buffer].opcode_number;
        if (this->branch_solve_stage != PROCESSOR_STAGE_FETCH) {
            PROCESSOR_DEBUG_PRINTF("\t Stalling fetch due to a miss prediction on package:%s\n", this->fetch_buffer[position_buffer].content_to_string().c_str());
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
        PROCESSOR_DEBUG_PRINTF("\t Sending INST package:%s\n", this->fetch_buffer[position_buffer].content_to_string().c_str());

        bool waiting = false;
        /// Check if the package still WAITING
        for (uint32_t k = 0; k < this->fetch_buffer_position_used; k++) {
            /// Update the FETCH BUFFER position
            uint32_t j = this->fetch_buffer_position_start + k;
            if (j >= this->fetch_buffer_size) {
                j -= this->fetch_buffer_size;
            }

            if (this->fetch_buffer[j].state == PACKAGE_STATE_WAIT &&
            /// Make sure the instruction starts and ends inside the block
            this->cmp_fetch_block(this->fetch_buffer[j].opcode_address, this->fetch_buffer[position_buffer].opcode_address)) {
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
        // ~ else if (this->cmp_fetch_block(this->fetch_buffer[position_buffer].opcode_address, this->fetch_opcode_address_line_buffer)) {
            // ~ PROCESSOR_DEBUG_PRINTF("\t FOUND INTO FETCH BUFFER (READY)\n");
            // ~ fetch_buffer[position_buffer].package_ready(1);
        // ~ }
        else {
            int32_t transmission_latency = this->send_instruction_package(this->fetch_buffer + position_buffer);
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

/// ============================================================================
/// Create a memory package
/// Get the route
/// Try to send
///     If OK
///         Get and Return the Latency
///     Else
///         Return POSITION_FAIL
int32_t processor_t::send_instruction_package(opcode_package_t *inst_package) {
    PROCESSOR_DEBUG_PRINTF("send_instruction_package() package:%s\n", inst_package->content_to_string().c_str());

    memory_package_t package;
    package.packager(
        this->get_id(),                     /// Request Owner Id
        inst_package->opcode_number,        /// opcode Number
        inst_package->opcode_address,       /// opcode Address
        0,                                  /// uop Number

        /// Request the whole line
        inst_package->opcode_address & this->not_fetch_offset_bits_mask,     /// Mem. Address
        this->fetch_block_size,             /// Instruction Size

        PACKAGE_STATE_TRANSMIT,             /// Pack. State
        0,                                  /// Ready Cycle Latency

        MEMORY_OPERATION_INST,              /// Mem Op. Type
        false,                              /// Is Answer

        this->get_id(),                                                                 /// Src ID
        this->get_interface_output_component(PROCESSOR_PORT_INST_CACHE)->get_id(),      /// Dst ID
        NULL,                               /// *Hops
        POSITION_FAIL                       /// Hop Counter
    );
    return send_package(&package);
};




/// ============================================================================
/// Divide the opcode into
///  1st. uop READ MEM. + unaligned
///  2st. uop READ 2 MEM. + unaligned
///  3rd. uop BRANCH
///  4th. uop ALU
///  5th. uop WRITE MEM. + unaligned
/// ============================================================================
/// To maintain the right dependencies between the uops and opcodes
/// If the opcode generates multiple uops, they must be in this format:
///
/// READ    ReadRegs    = BaseRegs + IndexRegs
///         WriteRegs   = 258 (Aux Register)
///
/// ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
///         WriteRegs   = * + 258 (Aux Register) (if is_write)
///
/// WRITE   ReadRegs    = * + 258 (Aux Register)
///         WriteRegs   = NULL
/// ============================================================================
void processor_t::stage_decode() {
    PROCESSOR_DEBUG_PRINTF("stage_decode()\n");
    int32_t position_buffer = POSITION_FAIL;
    uop_package_t *uop_ptr = NULL;

    /// Fetch_Buffer => (Decode) => Decode_Buffer
    for (uint32_t i = 0; i < this->stage_decode_width ; i++) {

        /// Stop to decode if achieved the maximum number of parallel branches in execution
        if (this->inflight_branches > this->inflight_branches_size) {
            break;
        }

        /// ====================================================================
        /// DECODE =============================================================

        /// Check if there is OpCode inside the fetch buffer
        /// Check if there is enough space for one OpCode be Decoded
        /// Check if the oldest fetch buffer position is ready
        if (this->fetch_buffer_position_used == 0 ||
        this->fetch_buffer[fetch_buffer_position_start].state != PACKAGE_STATE_READY ||
        this->fetch_buffer[fetch_buffer_position_start].ready_cycle > sinuca_engine.get_global_cycle()) {
            break;
        }

        /// Don't have enough space into the decode buffer
        if (this->decode_buffer_size - this->decode_buffer_position_used < MAX_UOP_DECODED) {
            this->add_stat_full_decode_buffer();
            break;
        }


        ERROR_ASSERT_PRINTF(this->decode_opcode_counter == this->fetch_buffer[fetch_buffer_position_start].opcode_number, "Renaming out-of-order.\n")
        this->decode_opcode_counter++;

        /// DECODE READ ====================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_read) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_LOAD,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read_address,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);

            /// Fix the dependencies between uops
            /// READ    ReadRegs    = BaseRegs + IndexRegs
            ///         WriteRegs   = 258 (Aux Register)

            uop_ptr = &this->decode_buffer[position_buffer];
            if (this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD){
                ///===== Read Regs =============================================
                /// Clear RRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    uop_ptr->read_regs[i] = POSITION_FAIL;
                }
                /// Insert BASE and INDEX into RReg
                uop_ptr->read_regs[0] = this->fetch_buffer[fetch_buffer_position_start].base_reg;
                uop_ptr->read_regs[1] = this->fetch_buffer[fetch_buffer_position_start].index_reg;

                ///===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    uop_ptr->write_regs[i] = POSITION_FAIL;
                }
                /// Insert 258 into WRegs
                uop_ptr->write_regs[0] = 258;
            }

            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].content_to_string().c_str());
        }

        /// DECODE READ 2 ==================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_read2) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_LOAD,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read2_address,
                                                                        this->fetch_buffer[fetch_buffer_position_start].read2_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);

            /// Fix the dependencies between uops
            /// READ    ReadRegs    = BaseRegs + IndexRegs
            ///         WriteRegs   = 258 (Aux Register)
            uop_ptr = &this->decode_buffer[position_buffer];
            if (this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD){
                ///===== Read Regs =============================================
                /// Clear RRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    uop_ptr->read_regs[i] = POSITION_FAIL;
                }
                /// Insert BASE and INDEX into RReg
                uop_ptr->read_regs[0] = this->fetch_buffer[fetch_buffer_position_start].base_reg;
                uop_ptr->read_regs[1] = this->fetch_buffer[fetch_buffer_position_start].index_reg;

                ///===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    uop_ptr->write_regs[i] = POSITION_FAIL;
                }
                /// Insert 258 into WRegs
                uop_ptr->write_regs[0] = 258;
            }

            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].content_to_string().c_str());
        }

        /// DECODE ALU =====================================================
        if (this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD &&
        this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_STORE &&
        this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_BRANCH) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, this->fetch_buffer[fetch_buffer_position_start].opcode_operation,
                                                                        0,
                                                                        0,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);

            /// Fix the dependencies between uops
            /// ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
            ///         WriteRegs   = * + 258 (Aux Register) (if is_write)
            uop_ptr = &this->decode_buffer[position_buffer];
            if (this->fetch_buffer[fetch_buffer_position_start].is_read || this->fetch_buffer[fetch_buffer_position_start].is_read2){
                ///===== Read Regs =============================================
                /// Insert Reg258 into RReg
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (uop_ptr->read_regs[i] == POSITION_FAIL) {
                        uop_ptr->read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
            }
            if (this->fetch_buffer[fetch_buffer_position_start].is_write){
                ///===== Write Regs =============================================
                /// Insert Reg258 into WReg
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (uop_ptr->write_regs[i] == POSITION_FAIL) {
                        uop_ptr->write_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
            }

            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].content_to_string().c_str());
        }

        /// DECODE BRANCH ==================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_branch) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_BRANCH,
                                                                        0,
                                                                        0,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);

            /// If the instruction is a branch
            this->inflight_branches++;

            /// Fix the dependencies between uops
            /// ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
            ///         WriteRegs   = * + 258 (Aux Register) (if is_write)
            uop_ptr = &this->decode_buffer[position_buffer];
            if (this->fetch_buffer[fetch_buffer_position_start].is_read || this->fetch_buffer[fetch_buffer_position_start].is_read2){
                ///===== Read Regs =============================================
                /// Insert Reg258 into RReg
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (uop_ptr->read_regs[i] == POSITION_FAIL) {
                        uop_ptr->read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
            }
            if (this->fetch_buffer[fetch_buffer_position_start].is_write){
                ///===== Write Regs =============================================
                /// Insert Reg258 into WReg
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (uop_ptr->write_regs[i] == POSITION_FAIL) {
                        uop_ptr->write_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
            }

            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].content_to_string().c_str());

            /// Solve the Branch Prediction
            this->solve_branch(this->fetch_buffer[fetch_buffer_position_start].opcode_number, PROCESSOR_STAGE_DECODE, this->fetch_buffer[fetch_buffer_position_start].opcode_operation);
        }


        /// DECODE WRITE ===================================================
        if (this->fetch_buffer[fetch_buffer_position_start].is_write) {
            position_buffer = this->decode_buffer_insert();
            ERROR_ASSERT_PRINTF(position_buffer != POSITION_FAIL, "Decoding more uops than MAX_UOP_DECODED (%d)", MAX_UOP_DECODED)
            this->decode_buffer[position_buffer].opcode_to_uop(this->decode_uop_counter++, INSTRUCTION_OPERATION_MEM_STORE,
                                                                        this->fetch_buffer[fetch_buffer_position_start].write_address,
                                                                        this->fetch_buffer[fetch_buffer_position_start].write_size,
                                                                        this->fetch_buffer[fetch_buffer_position_start]);

            /// Fix the dependencies between uops
            /// WRITE   ReadRegs    = * + 258 (Aux Register)
            ///         WriteRegs   = NULL
            uop_ptr = &this->decode_buffer[position_buffer];
            if (this->fetch_buffer[fetch_buffer_position_start].opcode_operation != INSTRUCTION_OPERATION_MEM_STORE){
                ///===== Read Regs =============================================
                /// Insert Reg258 into RReg
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (uop_ptr->read_regs[i] == POSITION_FAIL) {
                        uop_ptr->read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)

                ///===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    uop_ptr->write_regs[i] = POSITION_FAIL;
                }

            }

            this->decode_buffer[position_buffer].package_ready(this->stage_decode_cycles);
            PROCESSOR_DEBUG_PRINTF("\t Decode[%d] %s\n", position_buffer, this->decode_buffer[position_buffer].content_to_string().c_str());
        }

        /// Remove the oldest OPCODE (just decoded) from the fetch buffer
        this->fetch_buffer_remove();
    }
};


/// ============================================================================
bool processor_t::check_if_memory_overlaps(uint64_t memory_address1, uint32_t size1, uint64_t memory_address2, uint32_t size2) {
    PROCESSOR_DEBUG_PRINTF("check_if_memory_overlaps()\n");

    /// Address of the new load
    uint64_t new_read_start = memory_address1 & not_disambiguation_offset_bits_mask;
    uint64_t new_read_end = (new_read_start + size1 -1) & not_disambiguation_offset_bits_mask;

    /// No overlap
    /// | XX        | New Read
    /// |     XX    | Last Write
    uint64_t last_write_start = memory_address2 & not_disambiguation_offset_bits_mask;
    if (new_read_end < last_write_start) {
        return false;
    }

    /// No overlap
    /// |     XX    | New Read
    /// | XX        | Last Write
    uint64_t last_write_end = (last_write_start + size2 - 1) & not_disambiguation_offset_bits_mask;
    if (last_write_end < new_read_start) {
        return false;
    }

    return true;
};

/// ============================================================================
void processor_t::make_memory_dependencies(memory_order_buffer_line_t *new_mob_line, memory_order_buffer_line_t *input_array, uint32_t size_array){
        /// Find all the old READs/WRITEs into the MOB
    for (uint32_t j = 0; j < size_array; j++) {
        memory_order_buffer_line_t *old_mob_line = &input_array[j];

        /// Skip if (FREE) or (SAME MOB_LINE) or (ALREADY EXECUTED)
        if (old_mob_line->memory_request.state == PACKAGE_STATE_FREE ||
        old_mob_line->memory_request.uop_number == new_mob_line->memory_request.uop_number) {
            continue;
        }

        switch (this->disambiguation_type) {
            case DISAMBIGUATION_PERFECT: {
                /// If memory overlaps
                if (check_if_memory_overlaps(new_mob_line->memory_request.memory_address, new_mob_line->memory_request.memory_size,
                old_mob_line->memory_request.memory_address, old_mob_line->memory_request.memory_size)) {
                    /// Make dependencies to be solved
                    for (uint32_t k = 0; k < this->reorder_buffer_size; k++) {
                        if (old_mob_line->mem_deps_ptr_array[k] == NULL) {
                            old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
                            new_mob_line->wait_mem_deps_number++;
                            break;
                        }
                    }
                }
            }
            break;

            case DISAMBIGUATION_DISABLE: {
                /// For every old memory write
                /// Make dependencies to be solved
                /// However, read to read will never generate deps nor suffer READ-TO-READ solutions
                if (new_mob_line->memory_request.memory_operation == MEMORY_OPERATION_READ && old_mob_line->memory_request.memory_operation == MEMORY_OPERATION_READ){
                    break;
                }
                else {
                    for (uint32_t k = 0; k < this->reorder_buffer_size; k++) {
                        if (old_mob_line->mem_deps_ptr_array[k] == NULL) {
                            old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
                            new_mob_line->wait_mem_deps_number++;
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
};

/// ============================================================================
void processor_t::stage_rename() {
    PROCESSOR_DEBUG_PRINTF("stage_rename()\n");
    int32_t position_rob, position_mob;

    /// Copy from Rename => ROB and rename
    for (uint32_t i = 0; i < this->stage_rename_width; i++) {
        memory_order_buffer_line_t *mob_line = NULL;
        /// Check if there is any Uop decoded
        /// Check if the oldest fetch buffer position is ready
        if (decode_buffer_position_used == 0 ||
            this->decode_buffer[decode_buffer_position_start].state != PACKAGE_STATE_READY ||
            this->decode_buffer[decode_buffer_position_start].ready_cycle > sinuca_engine.get_global_cycle()) {
                break;
        }
        ERROR_ASSERT_PRINTF(this->rename_uop_counter == this->decode_buffer[decode_buffer_position_start].uop_number, "Renaming out-of-order.\n")

        /// If READ => Check free space on the MOB-READ
        if (this->decode_buffer[decode_buffer_position_start].uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {

            position_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_read, this->memory_order_buffer_read_size);
            if (position_mob == POSITION_FAIL) {
                this->add_stat_full_memory_order_buffer_read();
                break;
            }
            mob_line = &this->memory_order_buffer_read[position_mob];
        }
        /// If WRITE => Check free space on the MOB-WRITE
        else if (this->decode_buffer[decode_buffer_position_start].uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
            position_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_write, this->memory_order_buffer_write_size);
            if (position_mob == POSITION_FAIL) {
                this->add_stat_full_memory_order_buffer_write();
                break;
            }
            mob_line = &this->memory_order_buffer_write[position_mob];
        }


        /// Check free space on the ROB
        position_rob = this->rob_insert();
        if (position_rob == POSITION_FAIL) {
            this->add_stat_full_reorder_buffer();
            break;
        }

        ///=====================================================================
        /// Insert into ROB
        ///=====================================================================
        PROCESSOR_DEBUG_PRINTF("\t Inserting ROB[%d] %s\n", position_rob, this->decode_buffer[decode_buffer_position_start].content_to_string().c_str());
        this->reorder_buffer[position_rob].uop = this->decode_buffer[decode_buffer_position_start];
        this->decode_buffer_remove();

        this->rename_uop_counter++;
        this->reorder_buffer[position_rob].stage = PROCESSOR_STAGE_RENAME;
        this->reorder_buffer[position_rob].uop.package_ready(this->stage_rename_cycles + this->stage_dispatch_cycles);
        this->reorder_buffer[position_rob].mob_ptr = mob_line;
        /// Insert into the reservation station
        this->unified_reservation_station.push_back(&this->reorder_buffer[position_rob]);

        /// Solve the Branch Prediction
        if (this->reorder_buffer[position_rob].uop.uop_operation == INSTRUCTION_OPERATION_BRANCH) {
            this->solve_branch(this->reorder_buffer[position_rob].uop.opcode_number, PROCESSOR_STAGE_RENAME, this->reorder_buffer[position_rob].uop.uop_operation);
        }


        ///=====================================================================
        /// Insert into MOB
        ///=====================================================================
        if (this->reorder_buffer[position_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
            ERROR_ASSERT_PRINTF(this->reorder_buffer[position_rob].mob_ptr->memory_request.state == PACKAGE_STATE_FREE, "ROB has a pointer to a non free package.")
            /// Fix the request size to fit inside the cache line
            uint64_t offset = this->reorder_buffer[position_rob].uop.memory_address & this->offset_bits_mask;
            if (offset + this->reorder_buffer[position_rob].uop.memory_size >= sinuca_engine.get_global_line_size()) {
                this->reorder_buffer[position_rob].uop.memory_size = sinuca_engine.get_global_line_size() - offset;
            }

            this->memory_order_buffer_read_used++;

            this->reorder_buffer[position_rob].mob_ptr->memory_request.packager(
                this->get_id(),                                         /// Request Owner
                this->reorder_buffer[position_rob].uop.opcode_number,      /// Opcode. Number
                this->reorder_buffer[position_rob].uop.opcode_address,      /// Opcode. Address
                this->reorder_buffer[position_rob].uop.uop_number,         /// Uop. Number

                this->reorder_buffer[position_rob].uop.memory_address,     /// Mem. Address
                this->reorder_buffer[position_rob].uop.memory_size,        /// Block Size

                PACKAGE_STATE_TRANSMIT,                                 /// Pack. State
                this->stage_execution_cycles,                           /// Stall Cycles

                MEMORY_OPERATION_READ,                                  /// Mem. Operation
                false,                                                  /// Is Answer

                this->get_id(),                                                                 /// Src ID
                this->get_interface_output_component(PROCESSOR_PORT_DATA_CACHE)->get_id(),      /// Dst ID
                NULL,                          /// *Hops
                POSITION_FAIL                  /// Hop Counter
            );
        }
        else if (this->reorder_buffer[position_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
            ERROR_ASSERT_PRINTF(this->reorder_buffer[position_rob].mob_ptr->memory_request.state == PACKAGE_STATE_FREE, "ROB has a pointer to a non free package.")
            /// Fix the request size to fit inside the cache line
            uint64_t offset = this->reorder_buffer[position_rob].uop.memory_address & this->offset_bits_mask;
            if (offset + this->reorder_buffer[position_rob].uop.memory_size >= sinuca_engine.get_global_line_size()) {
                this->reorder_buffer[position_rob].uop.memory_size = sinuca_engine.get_global_line_size() - offset;
            }

            this->memory_order_buffer_write_used++;

            this->reorder_buffer[position_rob].mob_ptr->memory_request.packager(
                this->get_id(),                                         /// Request Owner
                this->reorder_buffer[position_rob].uop.opcode_number,     /// Opcode. Number
                this->reorder_buffer[position_rob].uop.opcode_address,     /// Opcode. Address
                this->reorder_buffer[position_rob].uop.uop_number,        /// Uop. Number

                this->reorder_buffer[position_rob].uop.memory_address,    /// Mem. Address
                this->reorder_buffer[position_rob].uop.memory_size,       /// Block Size

                PACKAGE_STATE_TRANSMIT,                                 /// Pack. State
                this->stage_execution_cycles,                           /// Stall Cycles

                MEMORY_OPERATION_WRITE,                                 /// Mem. Operation
                false,                                                  /// Is Answer

                this->get_id(),                                                                 /// Src ID
                this->get_interface_output_component(PROCESSOR_PORT_DATA_CACHE)->get_id(),      /// Dst ID
                NULL,                           /// *Hops
                POSITION_FAIL                   /// Hop Counter
            );
        }

        ///=====================================================================
        /// MEMORY DISAMBIGUATION
        /// Control the Memory Dependency - Memory READ/WRITE
        if (this->reorder_buffer[position_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD ||
        this->reorder_buffer[position_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
            /// Make connections between ROB and MOB
            mob_line->rob_ptr = &this->reorder_buffer[position_rob];

            this->make_memory_dependencies(this->reorder_buffer[position_rob].mob_ptr, this->memory_order_buffer_write, this->memory_order_buffer_write_size);
            this->make_memory_dependencies(this->reorder_buffer[position_rob].mob_ptr, this->memory_order_buffer_read, this->memory_order_buffer_read_size);
        }

        ///=====================================================================
        /// REGISTER RENAME
        /// Control the Register Dependency - Register READ
        for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
            if (this->reorder_buffer[position_rob].uop.read_regs[k] < 0) {
                break;
            }
            uint32_t read_register = this->reorder_buffer[position_rob].uop.read_regs[k];
            ERROR_ASSERT_PRINTF(read_register < this->register_alias_table_size, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, this->register_alias_table_size);
            /// If there is a dependency
            if (this->register_alias_table[read_register] != NULL) {
                for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
                    if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL) {
                        this->register_alias_table[read_register]->reg_deps_ptr_array[j] = &this->reorder_buffer[position_rob];
                        this->reorder_buffer[position_rob].wait_reg_deps_number++;
                        break;
                    }
                }
            }
        }

        /// Control the Register Dependency - Register WRITE
        for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
            if (this->reorder_buffer[position_rob].uop.write_regs[k] < 0) {
                break;
            }
            uint32_t write_register = this->reorder_buffer[position_rob].uop.write_regs[k];
            ERROR_ASSERT_PRINTF(write_register < this->register_alias_table_size, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, this->register_alias_table_size);

            this->register_alias_table[write_register] = &this->reorder_buffer[position_rob];
        }
    }
};

/// ============================================================================
void processor_t::stage_dispatch() {
    PROCESSOR_DEBUG_PRINTF("stage_dispatch()\n");

    /// Reset the number of instructions dispatched in this cycle
    static uint32_t total_dispatched;
    total_dispatched = 0;

    /// Control the total dispatched
    static uint32_t fu_int_alu;
    static uint32_t fu_int_mul;
    static uint32_t fu_int_div;
    fu_int_alu = 0;
    fu_int_mul = 0;
    fu_int_div = 0;

    static uint32_t fu_fp_alu;
    static uint32_t fu_fp_mul;
    static uint32_t fu_fp_div;
    fu_fp_alu = 0;
    fu_fp_mul = 0;
    fu_fp_div = 0;

    static uint32_t fu_mem_load;
    static uint32_t fu_mem_store;
    fu_mem_load = 0;
    fu_mem_store = 0;

    for (uint32_t k = 0; k < this->unified_reservation_station.size() && k < unified_reservation_station_window_size; k++) {
        reorder_buffer_line_t* reorder_buffer_line = this->unified_reservation_station[k];

        /// No other Functional Unit is available
        if (total_dispatched >= this->stage_dispatch_width) {
            break;
        }

        if (reorder_buffer_line->wait_reg_deps_number == 0 &&
        reorder_buffer_line->uop.ready_cycle <= sinuca_engine.get_global_cycle()) {
            ERROR_ASSERT_PRINTF(reorder_buffer_line->stage == PROCESSOR_STAGE_RENAME, "Reservation Station with package not in Rename Stage.\n")
            ERROR_ASSERT_PRINTF(reorder_buffer_line->uop.state == PACKAGE_STATE_READY, "Reservation Station with package not Ready.\n")
            PROCESSOR_DEBUG_PRINTF("\t Dispatching package\n");

            bool dispatched = false;

            switch (reorder_buffer_line->uop.uop_operation) {
            ///=============================================================
                /// BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    /// Solve the Branch Prediction
                    this->solve_branch(reorder_buffer_line->uop.opcode_number, PROCESSOR_STAGE_DISPATCH, reorder_buffer_line->uop.uop_operation);
                /// INTEGERS ALU ===============================================
                case INSTRUCTION_OPERATION_INT_ALU:
                /// NOP
                case INSTRUCTION_OPERATION_NOP:
                /// NOT INDENTIFIED
                case INSTRUCTION_OPERATION_OTHER:
                    if (fu_int_alu < this->number_fu_int_alu) {
                        for (uint32_t i = 0; i < this->number_fu_int_alu; i++) {
                            if (this->ready_cycle_fu_int_alu[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_int_alu[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_int_alu;

                                fu_int_alu++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_int_alu += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_int_alu);
                                break;
                            }
                        }
                    }
                break;
                /// INTEGERS MUL ===============================================
                case INSTRUCTION_OPERATION_INT_MUL:
                    if (fu_int_mul < this->number_fu_int_mul) {
                        for (uint32_t i = 0; i < this->number_fu_int_mul; i++) {
                            if (this->ready_cycle_fu_int_mul[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_int_mul[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_int_mul;

                                fu_int_mul++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_int_mul += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_int_mul);
                                break;
                            }
                        }
                    }
                break;
                /// INTEGERS DIV ===============================================
                case INSTRUCTION_OPERATION_INT_DIV:
                    if (fu_int_div < this->number_fu_int_div) {
                        for (uint32_t i = 0; i < this->number_fu_int_div; i++) {
                            if (this->ready_cycle_fu_int_div[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_int_div[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_int_div;

                                fu_int_div++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_int_div += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_int_div);
                                break;
                            }
                        }
                    }
                break;

                ///=============================================================
                /// FLOAT POINT ALU ============================================
                case INSTRUCTION_OPERATION_FP_ALU:
                    if (fu_fp_alu < this->number_fu_fp_alu) {
                        for (uint32_t i = 0; i < this->number_fu_fp_alu; i++) {
                            if (this->ready_cycle_fu_fp_alu[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_fp_alu[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_fp_alu;

                                fu_fp_alu++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_fp_alu += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_fp_alu);
                                break;
                            }
                        }
                    }
                break;
                /// FLOAT POINT MUL ============================================
                case INSTRUCTION_OPERATION_FP_MUL:
                    if (fu_fp_mul < this->number_fu_fp_mul) {
                        for (uint32_t i = 0; i < this->number_fu_fp_mul; i++) {
                            if (this->ready_cycle_fu_fp_mul[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_fp_mul[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_fp_mul;

                                fu_fp_mul++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_fp_mul += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_fp_mul);
                                break;
                            }
                        }
                    }
                break;
                /// FLOAT POINT DIV ============================================
                case INSTRUCTION_OPERATION_FP_DIV:
                    if (fu_fp_div < this->number_fu_fp_div) {
                        for (uint32_t i = 0; i < this->number_fu_fp_div; i++) {
                            if (this->ready_cycle_fu_fp_div[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_fp_div[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_fp_div;

                                fu_fp_div++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_fp_div += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_fp_div);
                                break;
                            }
                        }
                    }
                break;

                ///=============================================================
                /// MEMORY OPERATIONS ==========================================
                case INSTRUCTION_OPERATION_MEM_LOAD:
                    if (fu_mem_load < this->number_fu_mem_load) {
                        for (uint32_t i = 0; i < this->number_fu_mem_load; i++) {
                            if (this->ready_cycle_fu_mem_load[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_mem_load[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_mem_load;

                                fu_mem_load++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_mem_load += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_mem_load);
                                break;
                            }
                        }
                    }
                break;
                case INSTRUCTION_OPERATION_MEM_STORE:
                    if (fu_mem_store < this->number_fu_mem_store) {
                        for (uint32_t i = 0; i < this->number_fu_mem_store; i++) {
                            if (this->ready_cycle_fu_mem_store[i] <= sinuca_engine.get_global_cycle()) {
                                this->ready_cycle_fu_mem_store[i] = sinuca_engine.get_global_cycle() + this->wait_between_fu_mem_store;

                                fu_mem_store++;
                                dispatched = true;
                                reorder_buffer_line->stage = PROCESSOR_STAGE_EXECUTION;
                                this->stat_dispatch_cycles_fu_mem_store += sinuca_engine.get_global_cycle() - reorder_buffer_line->uop.ready_cycle;
                                reorder_buffer_line->uop.package_ready(this->latency_fu_mem_store);
                                break;
                            }
                        }
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

/// ============================================================================
void processor_t::stage_execution() {
    PROCESSOR_DEBUG_PRINTF("stage_execution()\n");
    int32_t position_mem;

    /// ========================================================================
    /// REMOVE MEMORY PACKAGE READS which are ready!
    for (uint32_t slot = 0; slot < this->memory_order_buffer_read_size; slot++) {
        if (this->memory_order_buffer_read_used == 0){
            break;
        }

        if (this->memory_order_buffer_read[slot].memory_request.state == PACKAGE_STATE_READY) {
            ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[slot].uop_executed == true, "Removing memory read before being executed.\n")
            ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[slot].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n")
            ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[slot].memory_request.is_answer == true, "Removing a package from MOB which is not answer.\n")
            // Solve ROB, and send to COMMIT
            this->memory_order_buffer_read[slot].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
            this->memory_order_buffer_read[slot].rob_ptr->uop.package_ready(this->stage_commit_cycles);
            this->memory_order_buffer_read[slot].rob_ptr->mob_ptr = NULL;
            this->solve_register_dependency(this->memory_order_buffer_read[slot].rob_ptr);
            this->solve_memory_dependency(&this->memory_order_buffer_read[slot]);
            /// Clean MOB line.
            this->memory_order_buffer_read[slot].package_clean();
            this->memory_order_buffer_read_used--;
            break;
        }
    }

    /// ========================================================================
    /// READ_BUFFER(PACKAGE_STATE_TRANSMIT) =>  send_package()
    position_mem = POSITION_FAIL;
    if (this->memory_order_buffer_read_used != 0){
        position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_read, this->memory_order_buffer_read_size, PACKAGE_STATE_TRANSMIT);
    }
    if (position_mem != POSITION_FAIL) {
        int32_t transmission_latency = this->send_package(&this->memory_order_buffer_read[position_mem].memory_request);
        if (transmission_latency != POSITION_FAIL) {  /// Try to send to the Data Cache.
            /// Wait answer.
            this->memory_order_buffer_read[position_mem].memory_request.package_wait(transmission_latency);
        }
    }

    /// ========================================================================
    /// WRITE_BUFFER(PACKAGE_STATE_TRANSMIT) =>  send_package()
    position_mem = POSITION_FAIL;
    if (this->memory_order_buffer_write_used != 0){
        position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_write, this->memory_order_buffer_write_size, PACKAGE_STATE_TRANSMIT);
    }
    if (position_mem != POSITION_FAIL) {
        int32_t transmission_latency = this->send_package(&this->memory_order_buffer_write[position_mem].memory_request);
        if (transmission_latency != POSITION_FAIL) {  /// Try to send to the DC.
            if (this->wait_write_complete) {
                this->memory_order_buffer_write[position_mem].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
                this->memory_order_buffer_write[position_mem].rob_ptr->uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
                this->memory_order_buffer_write[position_mem].rob_ptr->mob_ptr = NULL;
                /// Solve dependencies(forwarding data) before the store be removed from the MOB
                this->solve_register_dependency(this->memory_order_buffer_write[position_mem].rob_ptr);
                this->solve_memory_dependency(&this->memory_order_buffer_write[position_mem]);
                /// Clean MOB line.
                this->memory_order_buffer_write[position_mem].package_clean();
                this->memory_order_buffer_write_used--;
            }
            else {
                /// Solve dependencies(forwarding data) before the store be removed from the MOB
                this->solve_memory_dependency(&this->memory_order_buffer_write[position_mem]);
                /// Clean MOB line.
                this->memory_order_buffer_write[position_mem].package_clean();
                this->memory_order_buffer_write_used--;
            }
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
            ERROR_ASSERT_PRINTF(reorder_buffer_line->stage == PROCESSOR_STAGE_EXECUTION, "Functional Unit with package not in Execution Stage.\n%s\n",
                                                                                        reorder_buffer_line->content_to_string().c_str());
            ERROR_ASSERT_PRINTF(reorder_buffer_line->uop.state == PACKAGE_STATE_READY, "Functional Unit with package not Ready.\n")

            switch (reorder_buffer_line->uop.uop_operation) {
            ///=============================================================
                /// BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    /// Solve the Branch Prediction
                    this->solve_branch(reorder_buffer_line->uop.opcode_number, PROCESSOR_STAGE_EXECUTION, reorder_buffer_line->uop.uop_operation);
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
                    PROCESSOR_DEBUG_PRINTF("\t Executing package:%s\n", reorder_buffer_line->uop.content_to_string().c_str());
                    reorder_buffer_line->stage = PROCESSOR_STAGE_COMMIT;
                    reorder_buffer_line->uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
                    this->solve_register_dependency(reorder_buffer_line);
                    total_executed++;
                    /// Remove from the Functional Units
                    this->unified_functional_units.erase(this->unified_functional_units.begin() + k);
                    k--;
                break;


                /// FUNC_UNIT_MEM_LOAD => READ_BUFFER
                case INSTRUCTION_OPERATION_MEM_LOAD: {
                    reorder_buffer_line->mob_ptr->uop_executed = true;
                    /// Waits for the cache
                    reorder_buffer_line->uop.state = PACKAGE_STATE_TRANSMIT;
                    total_executed++;
                    /// Remove from the Functional Units
                    this->unified_functional_units.erase(this->unified_functional_units.begin() + k);
                    k--;
                }
                break;


                /// FUNC_UNIT_MEM_STORE => WRITE_BUFFER
                case INSTRUCTION_OPERATION_MEM_STORE: {
                    reorder_buffer_line->mob_ptr->uop_executed = true;

                    if (this->wait_write_complete) {
                        /// Waits for the cache
                        reorder_buffer_line->uop.state = PACKAGE_STATE_TRANSMIT;
                    }
                    else {
                        reorder_buffer_line->mob_ptr->rob_ptr = NULL;
                        reorder_buffer_line->mob_ptr = NULL;
                        /// Never waits for the cache
                        reorder_buffer_line->stage = PROCESSOR_STAGE_COMMIT;
                        reorder_buffer_line->uop.package_ready(this->stage_execution_cycles + this->stage_commit_cycles);
                        this->solve_register_dependency(reorder_buffer_line);
                    }

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


/// ============================================================================
void processor_t::solve_register_dependency(reorder_buffer_line_t *rob_line) {

    /// Remove pointers from Register Alias Table (RAT)
    for (uint32_t j = 0; j < MAX_REGISTERS; j++) {
        if (rob_line->uop.write_regs[j] < 0 ) {
            break;
        }
        uint32_t write_register = rob_line->uop.write_regs[j];
        ERROR_ASSERT_PRINTF(write_register < this->register_alias_table_size, "Read Register (%d) > Register Alias Table Size (%d)\n", write_register, this->register_alias_table_size);
        if (this->register_alias_table[write_register] != NULL && this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number) {
            this->register_alias_table[write_register] = NULL;
        }
    }

    ///=========================================================================
    /// SOLVE REGISTER DEPENDENCIES - RAT
    ///=========================================================================
    /// Send message to acknowledge the dependency is over
    for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
        /// There is an unsolved dependency
        if (rob_line->reg_deps_ptr_array[j] != NULL) {
            rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            rob_line->reg_deps_ptr_array[j]->uop.ready_cycle = sinuca_engine.get_global_cycle();
            rob_line->reg_deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
    }
}

/// ============================================================================
void processor_t::solve_memory_dependency(memory_order_buffer_line_t *mob_line) {
    ///=========================================================================
    /// SOLVE MEMORY DEPENDENCIES - MOB
    ///=========================================================================
    /// Send message to acknowledge the dependency is over
    for (uint32_t j = 0; j < this->reorder_buffer_size; j++) {
        /// There is an unsolved dependency
        if (mob_line->mem_deps_ptr_array[j] != NULL) {
            mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number--;

            switch (this->disambiguation_type) {
                case DISAMBIGUATION_PERFECT:
                    if (mob_line->mem_deps_ptr_array[j]->uop_executed == true &&
                    mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number == 0 &&
                    mob_line->mem_deps_ptr_array[j]->memory_request.memory_operation == MEMORY_OPERATION_READ) {
                        /// Check if address and size matches
                        if (mob_line->mem_deps_ptr_array[j]->memory_request.memory_address == mob_line->memory_request.memory_address &&
                        mob_line->mem_deps_ptr_array[j]->memory_request.memory_size == mob_line->memory_request.memory_size) {
                            /// Solve the LOAD->LOAD and STORE->LOAD
                            mob_line->mem_deps_ptr_array[j]->memory_request.state = PACKAGE_STATE_READY;
                            mob_line->mem_deps_ptr_array[j]->memory_request.is_answer = true;
                        }
                    }
                break;

                case DISAMBIGUATION_DISABLE:
                break;
            }

            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            mob_line->mem_deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
    }
};

/// ============================================================================
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
            PROCESSOR_DEBUG_PRINTF("\t Commiting package:%s\n",  this->reorder_buffer[position_buffer].content_to_string().c_str());
            // ~ this->add_stat_instruction_read_completed(this->reorder_buffer[position_buffer].uop.born_cycle);

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
        /// Could not commit the older, then stop looking for ready uops
        else {
            break;
        }
    }
};

/// ============================================================================
void processor_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    PROCESSOR_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    PROCESSOR_DEBUG_PRINTF("====================\n");
    PROCESSOR_DEBUG_PRINTF("cycle() \n");

    this->branch_predictor->clock(subcycle);

    /// Something to be done this cycle. -- Improve the performance
    if (this->reorder_buffer_position_used != 0) {
        /// Read from FU
        /// Mark ROB instructions as DONE
        /// Remove ISSUE WIDE oldest instructions
        this->stage_commit();
    }

    /// Something to be done this cycle. -- Improve the performance
    if (this->memory_order_buffer_read_used != 0 ||
    this->memory_order_buffer_write_used != 0 ||
    this->reorder_buffer_position_used != 0){
        /// Read each FU pipeline
        /// Creates latency for each instruction
        /// LOAD/STORES ready, are send to READ/WRITE buffer
        /// After allocate to READ/WRITE buffer, wait only for LOADS
        this->stage_execution();
    }

    if (this->reorder_buffer_position_used != 0) {
        /// Read Ready instructions from ROB
        /// Send to free Functional Units
        this->stage_dispatch();
    }

    /// Something to be done this cycle. -- Improve the performance
    if (decode_buffer_position_used != 0) {
        /// Read from the Decode Buffer
        /// Solve the dependencies
        /// Store on ROB
        this->stage_rename();
    }

    /// Something to be done this cycle. -- Improve the performance
    if (fetch_buffer_position_used != 0) {
        /// Read from the Fetch Buffer
        /// Split Load_Load into 2xLoads
        /// Split Load_Store into 1xLoad + 1xStore
        /// Store on Decode Buffer
        this->stage_decode();
    }

    /// Read from the trace
    /// Send request to ICache
    /// Store on Fetch Buffer, Waiting for ICache
    this->stage_fetch();
};

/// ============================================================================
int32_t processor_t::send_package(memory_package_t *package) {
    ERROR_ASSERT_PRINTF(!package->is_answer, "Processor is trying to send an answer.\n")

    /// Check if DESTINATION has FREE SPACE available.
    if (sinuca_engine.interconnection_interface_array[package->id_dst]->check_token_list(package) == false) {
        PROCESSOR_DEBUG_PRINTF("\tSEND FAIL (NO TOKENS)\n");
        return POSITION_FAIL;
    }

    sinuca_engine.interconnection_controller->find_package_route(package);
    ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
    uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
    ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
    package->hop_count--;  /// Consume its own port

    uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package, this, this->get_interface_output_component(output_port));
    bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port), transmission_latency);
    if (sent) {
        PROCESSOR_DEBUG_PRINTF("\tSEND OK\n");
        return transmission_latency;
    }
    else {
        PROCESSOR_DEBUG_PRINTF("\tSEND FAIL\n");
        package->hop_count++;  /// Do not Consume its own port
        return POSITION_FAIL;
    }

    PROCESSOR_DEBUG_PRINTF("\tSEND READ DATA FAIL (BUSY)\n");
    return POSITION_FAIL;
};

/// ============================================================================
bool processor_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {

    if (this->recv_ready_cycle[input_port] <= sinuca_engine.get_global_cycle()) {
        PROCESSOR_DEBUG_PRINTF("receive_package() port:%u, package:%s\n", input_port, package->content_to_string().c_str());
        ERROR_ASSERT_PRINTF(package->id_owner == this->get_id(), "Received some package for a different owner.\n");
        ERROR_ASSERT_PRINTF(package->id_dst == this->get_id(), "Received some package for a different id_dst.\n");
        ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");
        ERROR_ASSERT_PRINTF(package->is_answer == true, "Only answers are expected.\n");

        int32_t slot = POSITION_FAIL;
        switch (package->memory_operation) {
            case MEMORY_OPERATION_INST: {
                ERROR_ASSERT_PRINTF(input_port == PROCESSOR_PORT_INST_CACHE, "Receiving instruction package from a wrong port.\n");

                /// Add to the buffer the whole line fetched
                this->fetch_opcode_address_line_buffer = package->memory_address;

                /// Find packages WAITING
                for (uint32_t k = 0; k < this->fetch_buffer_position_used; k++) {
                    /// Update the FETCH BUFFER position
                    uint32_t j = this->fetch_buffer_position_start + k;
                    if (j >= this->fetch_buffer_size) {
                        j -= this->fetch_buffer_size;
                    }

                    /// Wake up ALL instructions waiting
                    if (this->fetch_buffer[j].state == PACKAGE_STATE_WAIT &&
                    this->cmp_fetch_block(package->memory_address, this->fetch_buffer[j].opcode_address)) {
                        this->add_stat_instruction_read_completed(this->fetch_buffer[j].born_cycle);
                        PROCESSOR_DEBUG_PRINTF("\t WANTED INSTRUCTION\n");
                        this->fetch_buffer[j].package_ready(transmission_latency);
                        slot = j;
                    }
                }
                ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Processor Read Instruction done, but it is not on the fetch-buffer anymore.\n")
                this->recv_ready_cycle[input_port] = sinuca_engine.get_global_cycle() + transmission_latency;
                return OK;
            }
            break;

            case MEMORY_OPERATION_READ:
                ERROR_ASSERT_PRINTF(input_port == PROCESSOR_PORT_DATA_CACHE, "Receiving read package from a wrong port.\n");

                slot = memory_order_buffer_line_t::find_uop_number(this->memory_order_buffer_read, this->memory_order_buffer_read_size, package->uop_number);
                ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Processor Read done, but no request in the read-buffer found.\npackage:%s\n",
                                                                    package->content_to_string().c_str());
                if (slot != POSITION_FAIL) {
                    PROCESSOR_DEBUG_PRINTF("\t WANTED READ.\n");
                    this->memory_order_buffer_read[slot].memory_request.package_ready(sinuca_engine.get_global_cycle() + transmission_latency);
                    this->memory_order_buffer_read[slot].memory_request.is_answer = true;
                }
                ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Processor Read Request done, but it is not on the memory_order_buffer anymore.\n")
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

/// ============================================================================
/// Token Controller Methods
/// ============================================================================
bool processor_t::check_token_list(memory_package_t *package) {
    ERROR_PRINTF("check_token_list %s.\n", package->content_to_string().c_str())
    return FAIL;
};

/// ============================================================================
uint32_t processor_t::check_token_space(memory_package_t *package) {
    ERROR_PRINTF("check_token_space %s.\n", package->content_to_string().c_str())
    return 0;
};

/// ============================================================================
void processor_t::remove_token_list(memory_package_t *package) {
    ERROR_PRINTF("remove_token_list %s.\n", package->content_to_string().c_str())
};


/// ============================================================================
void processor_t::print_structures() {
    SINUCA_PRINTF("%s BRANCH_SOLVE_STAGE:%s  BRANCH_OPCODE_NUMBER:%"PRIu64"\n", this->get_label(), get_enum_processor_stage_char(this->branch_solve_stage), this->branch_opcode_number);
    SINUCA_PRINTF("%s SYNC_STATUS:%s\n", this->get_label(), get_enum_sync_char(this->sync_status));

    SINUCA_PRINTF("%s FETCH_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->fetch_buffer_position_start, this->fetch_buffer_position_end, this->fetch_buffer_position_used);
    SINUCA_PRINTF("%s FETCH_BUFFER:\n%s", this->get_label(), opcode_package_t::print_all(this->fetch_buffer, this->fetch_buffer_size).c_str());

    SINUCA_PRINTF("%s DECODE_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->decode_buffer_position_start, this->decode_buffer_position_end, this->decode_buffer_position_used);
    SINUCA_PRINTF("%s DECODE_BUFFER:\n%s", this->get_label(), uop_package_t::print_all(this->decode_buffer, this->decode_buffer_size).c_str());

    SINUCA_PRINTF("%s REORDER_BUFFER START:%d  END:%d  SIZE:%d\n", this->get_label(), this->reorder_buffer_position_start, this->reorder_buffer_position_end, this->reorder_buffer_position_used);
    SINUCA_PRINTF("%s REORDER_BUFFER:\n%s",  this->get_label(), reorder_buffer_line_t::print_all(this->reorder_buffer, this->reorder_buffer_size).c_str());

    SINUCA_PRINTF("%s MEMORY_ORDER_BUFFER_READ:\n%s",  this->get_label(), memory_order_buffer_line_t::print_all(this->memory_order_buffer_read, this->memory_order_buffer_read_size).c_str());
    SINUCA_PRINTF("%s MEMORY_ORDER_BUFFER_WRITE:\n%s",  this->get_label(), memory_order_buffer_line_t::print_all(this->memory_order_buffer_write, this->memory_order_buffer_write_size).c_str());
};

// =============================================================================
void processor_t::panic() {
    this->print_structures();
    this->branch_predictor->panic();
};

/// ============================================================================
void processor_t::periodic_check(){
    #ifdef PROCESSOR_DEBUG
        PROCESSOR_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(opcode_package_t::check_age(this->fetch_buffer, this->fetch_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(uop_package_t::check_age(this->decode_buffer, this->decode_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(reorder_buffer_line_t::check_age(this->reorder_buffer, this->reorder_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(memory_order_buffer_line_t::check_age(this->memory_order_buffer_read, this->memory_order_buffer_read_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(memory_order_buffer_line_t::check_age(this->memory_order_buffer_write, this->memory_order_buffer_write_size) == OK, "Check_age failed.\n");

    this->branch_predictor->periodic_check();
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void processor_t::reset_statistics() {

    this->set_stat_reset_fetch_opcode_counter(this->fetch_opcode_counter);
    this->set_stat_reset_decode_uop_counter(this->decode_uop_counter);

    this->set_stat_branch_stall_cycles(0);
    this->set_stat_sync_stall_cycles(0);

    this->stat_full_fetch_buffer = 0;
    this->stat_full_decode_buffer = 0;
    this->stat_full_reorder_buffer = 0;
    this->stat_full_memory_order_buffer_read = 0;
    this->stat_full_memory_order_buffer_write = 0;

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

    this->branch_predictor->reset_statistics();
    return;
};

/// ============================================================================
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

    /// ROB and MOB Statistics
    sinuca_engine.write_statistics_small_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_fetch_buffer", stat_full_fetch_buffer);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_decode_buffer", stat_full_decode_buffer);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_reorder_buffer", stat_full_reorder_buffer);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_memory_order_buffer_read", stat_full_memory_order_buffer_read);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_memory_order_buffer_write", stat_full_memory_order_buffer_write);

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

    this->branch_predictor->print_statistics();
};

/// ============================================================================
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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "unified_reservation_station_window_size", unified_reservation_station_window_size);

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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_int_alu", wait_between_fu_int_alu);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_int_mul", number_fu_int_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_int_mul", latency_fu_int_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_int_mul", wait_between_fu_int_mul);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_int_div", number_fu_int_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_int_div", latency_fu_int_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_int_div", wait_between_fu_int_div);

    /// Floating Point Functional Units
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_alu", number_fu_fp_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_alu", latency_fu_fp_alu);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_fp_alu", wait_between_fu_fp_alu);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_mul", number_fu_fp_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_mul", latency_fu_fp_mul);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_fp_mul", wait_between_fu_fp_mul);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_fp_div", number_fu_fp_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_fp_div", latency_fu_fp_div);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_fp_div", wait_between_fu_fp_div);

    /// Memory Functional Units
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_mem_load", number_fu_mem_load);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_mem_load", latency_fu_mem_load);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_mem_load", wait_between_fu_mem_load);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "number_fu_mem_store", number_fu_mem_store);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "latency_fu_mem_store", latency_fu_mem_store);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "wait_between_fu_mem_store", wait_between_fu_mem_store);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "memory_order_buffer_read_size", memory_order_buffer_read_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "memory_order_buffer_write_size", memory_order_buffer_write_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fetch_block_size", this->fetch_block_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fetch_offset_bits_mask", utils_t::address_to_binary(this->fetch_offset_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "not_fetch_offset_bits_mask", utils_t::address_to_binary(this->not_fetch_offset_bits_mask).c_str());


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "offset_bits_mask", utils_t::address_to_binary(this->offset_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "not_offset_bits_mask", utils_t::address_to_binary(this->not_offset_bits_mask).c_str());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "branch_per_fetch", branch_per_fetch);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "inflight_branches_size", inflight_branches_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "branch_flush_latency", branch_flush_latency);


    this->branch_predictor->print_configuration();
};

/// ============================================================================
/// Fetch Buffer Methods
/// ============================================================================
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

/// ============================================================================
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

/// ============================================================================
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

/// ============================================================================
/// Decode Buffer Methods
/// ============================================================================
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

/// ============================================================================
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


/// ============================================================================
/// Reorder Buffer Methods
/// ============================================================================
/*! Should make all the verifications before call this method, because it will
 * update the position_end and position_used for the decode_buffer
 */
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

/// ============================================================================
/*! Make sure that you want to remove the first element before call this method
 * because it will remove the buffer[position_start] and update the controls
 */
void processor_t::rob_remove() {
    ERROR_ASSERT_PRINTF(this->reorder_buffer_position_used > 0, "Trying to remove from ROB with no used position.\n");
    ERROR_ASSERT_PRINTF(this->reorder_buffer[this->reorder_buffer_position_start].reg_deps_ptr_array[0] == NULL, "Removing from ROB without solve the dependencies.\n");

    this->reorder_buffer[this->reorder_buffer_position_start].package_clean();

    this->reorder_buffer_position_used--;
    this->reorder_buffer_position_start++;
    if (this->reorder_buffer_position_start >= this->reorder_buffer_size) {
        this->reorder_buffer_position_start = 0;
    }
};

