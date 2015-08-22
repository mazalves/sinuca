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

#include "../circular_buffer.hpp"

#ifndef _PROCESSOR_PROCESSOR_HPP_
#define _PROCESSOR_PROCESSOR_HPP_

class processor_t : public interconnection_interface_t {
    public:
        /// Branch Predictor
        branch_predictor_t *branch_predictor;    /// Branch Predictor
        uint64_t branch_opcode_number;          /// Opcode which will solve the branch
        processor_stage_t branch_solve_stage;   /// Stage which will solve the branch

    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t core_id;

        /// Buffers' Size
        uint32_t fetch_buffer_size;
        uint32_t decode_buffer_size;
        uint32_t reorder_buffer_size;

        /// Stages' Latency
        uint32_t stage_fetch_cycles;
        uint32_t stage_decode_cycles;
        uint32_t stage_rename_cycles;
        uint32_t stage_dispatch_cycles;
        uint32_t stage_execution_cycles;
        uint32_t stage_commit_cycles;

        /// Stages' Width
        uint32_t stage_fetch_width;
        uint32_t stage_decode_width;
        uint32_t stage_rename_width;
        uint32_t stage_dispatch_width;
        uint32_t stage_execution_width;
        uint32_t stage_commit_width;

        // ====================================================================
        /// Integer Functional Units
        uint32_t number_fu_int_alu;
        uint32_t latency_fu_int_alu;
        uint32_t wait_between_fu_int_alu;

        uint32_t number_fu_int_mul;
        uint32_t latency_fu_int_mul;
        uint32_t wait_between_fu_int_mul;

        uint32_t number_fu_int_div;
        uint32_t latency_fu_int_div;
        uint32_t wait_between_fu_int_div;

        // ====================================================================
        /// Floating Point Functional Units
        uint32_t number_fu_fp_alu;
        uint32_t latency_fu_fp_alu;
        uint32_t wait_between_fu_fp_alu;

        uint32_t number_fu_fp_mul;
        uint32_t latency_fu_fp_mul;
        uint32_t wait_between_fu_fp_mul;

        uint32_t number_fu_fp_div;
        uint32_t latency_fu_fp_div;
        uint32_t wait_between_fu_fp_div;

        // ====================================================================
        /// Memory Functional Units
        uint32_t number_fu_mem_load;
        uint32_t latency_fu_mem_load;
        uint32_t wait_between_fu_mem_load;

        uint32_t number_fu_mem_store;
        uint32_t latency_fu_mem_store;
        uint32_t wait_between_fu_mem_store;

        uint32_t memory_order_buffer_read_size;
        uint32_t memory_order_buffer_write_size;

        uint32_t disambiguation_block_size;
        disambiguation_t disambiguation_type;
        uint32_t register_forward_latency;
        bool solve_address_to_address;

        uint32_t fetch_block_size;
        bool wait_write_complete;

        /// Branch Latency to flush on wrong prediction
        uint32_t branch_per_fetch;
        uint32_t branch_flush_latency;
        uint32_t inflight_branches;
        uint32_t inflight_branches_size;
        uint64_t branch_flush_cycle_ready;

        uint32_t unified_reservation_station_window_size;

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        uint64_t offset_bits_mask;      /// Offset mask
        uint64_t not_offset_bits_mask;  /// Offset mask

        uint64_t fetch_offset_bits_mask;      /// Offset mask
        uint64_t not_fetch_offset_bits_mask;  /// Offset mask

        uint64_t disambiguation_offset_bits_mask;      /// Offset mask
        uint64_t not_disambiguation_offset_bits_mask;  /// Offset mask

        uint64_t *recv_ready_cycle;

        /// Synchronization Control Variables
        sync_t sync_status;                    /// Used to sync or request sync
        uint64_t sync_status_time;              /// Last time the sync changed
        bool trace_over;

        /// Stages Control Variables
        opcode_package_t trace_next_opcode;
        uint64_t fetch_opcode_address;
        uint64_t fetch_opcode_address_line_buffer;

        uint64_t fetch_opcode_counter;
        uint64_t decode_opcode_counter;

        uint64_t decode_uop_counter;
        uint64_t rename_uop_counter;
        uint64_t commit_uop_counter;

        /// Fetch buffer
        circular_buffer_t<opcode_package_t> fetch_buffer;
        /// Decode buffer
        circular_buffer_t<uop_package_t> decode_buffer;

        /// Reorder buffer
        reorder_buffer_line_t *reorder_buffer;
        uint32_t reorder_buffer_position_start;
        uint32_t reorder_buffer_position_end;
        uint32_t reorder_buffer_position_used;

        /// Register Alias Table for Renaming
        uint32_t register_alias_table_size;
        reorder_buffer_line_t* *register_alias_table;

        /// Store the last request (r/w) the processor tryed to send.
        memory_order_buffer_line_t *oldest_read_to_send;
        memory_order_buffer_line_t *oldest_write_to_send;


        /// Containers to fast the execution, with pointers of UOPs ready.
        container_ptr_reorder_buffer_line_t unified_reservation_station;    /// dispatch->execute
        container_ptr_reorder_buffer_line_t unified_functional_units;       /// execute->commit

        // ====================================================================
        /// Integer Functional Units
        uint64_t *ready_cycle_fu_int_alu;
        uint64_t *ready_cycle_fu_int_mul;
        uint64_t *ready_cycle_fu_int_div;

        // ====================================================================
        /// Floating Point Functional Units
        uint64_t *ready_cycle_fu_fp_alu;
        uint64_t *ready_cycle_fu_fp_mul;
        uint64_t *ready_cycle_fu_fp_div;

        // ====================================================================
        /// Memory Functional Units
        uint64_t *ready_cycle_fu_mem_load;
        uint64_t *ready_cycle_fu_mem_store;

        memory_order_buffer_line_t *memory_order_buffer_read;
        memory_order_buffer_line_t *memory_order_buffer_write;
        /// Number of positions used inside MOB
        uint32_t memory_order_buffer_read_executed;
        uint32_t memory_order_buffer_write_executed;

        cache_memory_t *data_cache;
        cache_memory_t *inst_cache;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_active_cycles;
        uint64_t stat_idle_cycles;

        uint64_t stat_branch_stall_cycles;
        uint64_t stat_sync_stall_cycles;

        uint64_t stat_reset_fetch_opcode_counter;
        uint64_t stat_reset_decode_uop_counter;

        /// Full ROB and MOB statistics
        uint64_t stat_full_fetch_buffer;
        uint64_t stat_full_decode_buffer;
        uint64_t stat_full_reorder_buffer;
        uint64_t stat_full_memory_order_buffer_read;
        uint64_t stat_full_memory_order_buffer_write;

        /// Executed Instructions
        uint64_t stat_nop_completed;
        uint64_t stat_branch_completed;
        uint64_t stat_other_completed;

        uint64_t stat_int_alu_completed;
        uint64_t stat_int_mul_completed;
        uint64_t stat_int_div_completed;

        uint64_t stat_fp_alu_completed;
        uint64_t stat_fp_mul_completed;
        uint64_t stat_fp_div_completed;

        uint64_t stat_instruction_read_completed;
        uint64_t stat_memory_read_completed;
        uint64_t stat_memory_write_completed;
        uint64_t stat_address_to_address;

        // Executed HMC
        uint64_t stat_hmc_completed;

        /// Dispatch Cycles Stall
        uint64_t stat_dispatch_cycles_fu_int_alu;
        uint64_t stat_dispatch_cycles_fu_int_mul;
        uint64_t stat_dispatch_cycles_fu_int_div;

        uint64_t stat_dispatch_cycles_fu_fp_alu;
        uint64_t stat_dispatch_cycles_fu_fp_mul;
        uint64_t stat_dispatch_cycles_fu_fp_div;

        uint64_t stat_dispatch_cycles_fu_mem_load;
        uint64_t stat_dispatch_cycles_fu_mem_store;

        /// Memory Cycles Stall
        uint64_t stat_min_instruction_read_wait_time;
        uint64_t stat_max_instruction_read_wait_time;
        uint64_t stat_accumulated_instruction_read_wait_time;

        uint64_t stat_min_memory_read_wait_time;
        uint64_t stat_max_memory_read_wait_time;
        uint64_t stat_accumulated_memory_read_wait_time;

        uint64_t stat_min_memory_write_wait_time;
        uint64_t stat_max_memory_write_wait_time;
        uint64_t stat_accumulated_memory_write_wait_time;

        // HMC Cycles Stall
        uint64_t stat_min_hmc_wait_time;
        uint64_t stat_max_hmc_wait_time;
        uint64_t stat_accumulated_hmc_wait_time;



    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        processor_t();
        ~processor_t();
        int32_t send_instruction_package(opcode_package_t *is);
        int32_t send_data_package(memory_package_t *ms);

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        bool check_token_list(memory_package_t *package);
        void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        // ====================================================================

        void synchronize(sync_t new_sync);
        void solve_branch(uint64_t opcode_number, processor_stage_t processor_stage, instruction_operation_t operation);
        void stage_fetch();
        void stage_decode();
        void stage_rename();
        void stage_dispatch();
        void stage_execution();
        void stage_commit();

        inline bool cmp_fetch_block(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_fetch_offset_bits_mask) == (memory_addressB & this->not_fetch_offset_bits_mask);
        }

        inline bool cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }

        bool is_busy();

        bool check_if_memory_overlaps(uint64_t memory_address1, uint32_t size1, uint64_t memory_address2, uint32_t size2);
        void make_memory_dependencies(memory_order_buffer_line_t *new_mob_line, memory_order_buffer_line_t *input_array, uint32_t size_array);
        void solve_register_dependency(reorder_buffer_line_t *rob_line);
        void solve_memory_dependency(memory_order_buffer_line_t *mob_line);

        /// Reorder Buffer =====================================================
        int32_t rob_insert();
        void rob_remove();
        // ====================================================================


        INSTANTIATE_GET_SET(uint32_t, core_id)
        INSTANTIATE_GET_SET(uint64_t, offset_bits_mask)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)

        INSTANTIATE_GET_SET(uint32_t, fetch_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, decode_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, reorder_buffer_size)

        /// Stages' Latency
        INSTANTIATE_GET_SET(uint32_t, stage_fetch_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_decode_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_rename_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_dispatch_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_execution_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_commit_cycles)

        /// Stages' Width
        INSTANTIATE_GET_SET(uint32_t, stage_fetch_width)
        INSTANTIATE_GET_SET(uint32_t, stage_decode_width)
        INSTANTIATE_GET_SET(uint32_t, stage_rename_width)
        INSTANTIATE_GET_SET(uint32_t, stage_dispatch_width)
        INSTANTIATE_GET_SET(uint32_t, stage_execution_width)
        INSTANTIATE_GET_SET(uint32_t, stage_commit_width)

        INSTANTIATE_GET_SET(uint64_t, fetch_opcode_counter)
        INSTANTIATE_GET_SET(uint64_t, decode_opcode_counter)

        INSTANTIATE_GET_SET(uint64_t, decode_uop_counter)
        INSTANTIATE_GET_SET(uint64_t, rename_uop_counter)
        INSTANTIATE_GET_SET(uint64_t, commit_uop_counter)

        /// Integer Funcional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_int_alu)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_alu)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_int_alu)

        INSTANTIATE_GET_SET(uint32_t, number_fu_int_mul)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_mul)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_int_mul)

        INSTANTIATE_GET_SET(uint32_t, number_fu_int_div)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_div)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_int_div)

        /// Floating Point Functional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_alu)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_alu)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_fp_alu)

        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_mul)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_mul)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_fp_mul)

        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_div)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_div)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_fp_div)

        /// Memory Functional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_mem_load)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_mem_load)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_mem_load)

        INSTANTIATE_GET_SET(uint32_t, number_fu_mem_store)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_mem_store)
        INSTANTIATE_GET_SET(uint32_t, wait_between_fu_mem_store)

        INSTANTIATE_GET_SET(uint32_t, memory_order_buffer_read_size)
        INSTANTIATE_GET_SET(uint32_t, memory_order_buffer_write_size)

        INSTANTIATE_GET_SET(uint32_t, memory_order_buffer_read_executed)
        INSTANTIATE_GET_SET(uint32_t, memory_order_buffer_write_executed)

        INSTANTIATE_GET_SET(disambiguation_t, disambiguation_type)
        INSTANTIATE_GET_SET(uint32_t, disambiguation_block_size)
        INSTANTIATE_GET_SET(uint32_t, register_forward_latency)
        INSTANTIATE_GET_SET(bool, solve_address_to_address)

        INSTANTIATE_GET_SET(cache_memory_t*, data_cache)
        INSTANTIATE_GET_SET(cache_memory_t*, inst_cache)

        INSTANTIATE_GET_SET(uint32_t, fetch_block_size)
        INSTANTIATE_GET_SET(bool, wait_write_complete)
        INSTANTIATE_GET_SET(uint32_t, unified_reservation_station_window_size)
        INSTANTIATE_GET_SET(uint32_t, branch_per_fetch)

        /// Branch Latency to flush on wrong prediction
        INSTANTIATE_GET_SET(uint32_t, branch_flush_latency)
        INSTANTIATE_GET_SET(uint32_t, inflight_branches)
        INSTANTIATE_GET_SET(uint32_t, inflight_branches_size)
        INSTANTIATE_GET_SET(uint64_t, branch_flush_cycle_ready)

        /// Processor Synchronization
        INSTANTIATE_GET_SET(sync_t, sync_status);
        INSTANTIATE_GET_SET(uint64_t, sync_status_time);

        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_active_cycles)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_idle_cycles)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_stall_cycles)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sync_stall_cycles)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_fetch_opcode_counter)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_decode_uop_counter)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_fetch_buffer)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_decode_buffer)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_reorder_buffer)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_memory_order_buffer_read)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_memory_order_buffer_write)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_nop_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_other_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_alu_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_mul_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_div_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_alu_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_mul_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_div_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_read_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_memory_read_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_memory_write_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_int_alu)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_int_mul)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_int_div)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_fp_alu)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_fp_mul)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_fp_div)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_mem_load)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dispatch_cycles_fu_mem_store)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_address_to_address)

        inline void add_stat_instruction_read_completed(uint64_t born_cycle) {
            this->stat_instruction_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            stat_accumulated_instruction_read_wait_time += new_time;
            if (this->stat_min_instruction_read_wait_time > new_time) this->stat_min_instruction_read_wait_time = new_time;
            if (this->stat_max_instruction_read_wait_time < new_time) this->stat_max_instruction_read_wait_time = new_time;
        };

        inline void add_stat_memory_read_completed(uint64_t born_cycle) {
            this->stat_memory_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_memory_read_wait_time += new_time;
            if (this->stat_min_memory_read_wait_time > new_time) this->stat_min_memory_read_wait_time = new_time;
            if (this->stat_max_memory_read_wait_time < new_time) this->stat_max_memory_read_wait_time = new_time;
        };

        inline void add_stat_memory_write_completed(uint64_t born_cycle) {
            this->stat_memory_write_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_memory_write_wait_time += new_time;
            if (this->stat_min_memory_write_wait_time > new_time) this->stat_min_memory_write_wait_time = new_time;
            if (this->stat_max_memory_write_wait_time < new_time) this->stat_max_memory_write_wait_time = new_time;
        };

        // HMC
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_hmc_completed)
        inline void add_stat_hmc_completed(uint64_t born_cycle) {
            this->stat_hmc_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_hmc_wait_time += new_time;
            if (this->stat_min_hmc_wait_time > new_time) this->stat_min_hmc_wait_time = new_time;
            if (this->stat_max_hmc_wait_time < new_time) this->stat_max_hmc_wait_time = new_time;
        };


};
#endif  // _PROCESSOR_PROCESSOR_HPP_
