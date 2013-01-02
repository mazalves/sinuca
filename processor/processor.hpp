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
#ifndef _PROCESSOR_PROCESSOR_HPP_
#define _PROCESSOR_PROCESSOR_HPP_

class processor_t : public interconnection_interface_t {
    public:
        /// Branch Predictor
        branch_predictor_t *branch_predictor;    /// Branch Predictor
        uint64_t branch_opcode_number;          /// Opcode which will solve the branch
        processor_stage_t branch_solve_stage;   /// Stage which will solve the branch

    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
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

        /// ====================================================================
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

        /// ====================================================================
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

        /// ====================================================================
        /// Memory Functional Units
        uint32_t number_fu_mem_load;
        uint32_t latency_fu_mem_load;
        uint32_t wait_between_fu_mem_load;

        uint32_t number_fu_mem_store;
        uint32_t latency_fu_mem_store;
        uint32_t wait_between_fu_mem_store;

        uint32_t read_buffer_size;
        uint32_t write_buffer_size;

        uint32_t fetch_block_size;
        uint32_t branch_per_fetch;
        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t offset_bits_mask;      /// Offset mask
        uint64_t not_offset_bits_mask;  /// Offset mask

        uint64_t fetch_offset_bits_mask;      /// Offset mask
        uint64_t not_fetch_offset_bits_mask;  /// Offset mask

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
        opcode_package_t *fetch_buffer;
        uint32_t fetch_buffer_position_start;
        uint32_t fetch_buffer_position_end;
        uint32_t fetch_buffer_position_used;

        /// Decode buffer
        uop_package_t *decode_buffer;
        uint32_t decode_buffer_position_start;
        uint32_t decode_buffer_position_end;
        uint32_t decode_buffer_position_used;

        /// Reorder buffer
        reorder_buffer_line_t *reorder_buffer;
        uint32_t reorder_buffer_position_start;
        uint32_t reorder_buffer_position_end;
        uint32_t reorder_buffer_position_used;

        /// Register Alias Table for Renaming
        uint32_t register_alias_table_size;
        reorder_buffer_line_t* *register_alias_table;
        reorder_buffer_line_t* *memory_map_table;

        /// Containers to fast the execution, with pointers of UOPs ready.
        container_ptr_reorder_buffer_line_t unified_reservation_station;    /// dispatch->execute
        container_ptr_reorder_buffer_line_t unified_functional_units;       /// execute->commit

        /// ====================================================================
        /// Integer Functional Units
        uint64_t *ready_cycle_fu_int_alu;
        uint64_t *ready_cycle_fu_int_mul;
        uint64_t *ready_cycle_fu_int_div;

        /// ====================================================================
        /// Floating Point Functional Units
        uint64_t *ready_cycle_fu_fp_alu;
        uint64_t *ready_cycle_fu_fp_mul;
        uint64_t *ready_cycle_fu_fp_div;

        /// ====================================================================
        /// Memory Functional Units
        uint64_t *ready_cycle_fu_mem_load;
        uint64_t *ready_cycle_fu_mem_store;

        memory_package_t *read_buffer;
        memory_package_t *write_buffer;

        cache_memory_t *data_cache;
        cache_memory_t *inst_cache;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_branch_stall_cycles;
        uint64_t stat_sync_stall_cycles;

        uint64_t stat_reset_fetch_opcode_counter;
        uint64_t stat_reset_decode_uop_counter;

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
        uint64_t stat_acumulated_instruction_read_wait_time;

        uint64_t stat_min_memory_read_wait_time;
        uint64_t stat_max_memory_read_wait_time;
        uint64_t stat_acumulated_memory_read_wait_time;

        uint64_t stat_min_memory_write_wait_time;
        uint64_t stat_max_memory_write_wait_time;
        uint64_t stat_acumulated_memory_write_wait_time;


    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        processor_t();
        ~processor_t();
        int32_t send_instruction_package(opcode_package_t *is);
        int32_t send_data_package(memory_package_t *ms);

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        void allocate_token_list();
        bool check_token_list(memory_package_t *package);
        uint32_t check_token_space(memory_package_t *package);
        void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void synchronize(sync_t new_sync);
        void solve_branch(uint64_t opcode_number, processor_stage_t processor_stage);
        void stage_fetch();
        void stage_decode();
        void stage_rename();
        void stage_dispatch();
        void stage_execution();
        void solve_data_forward(reorder_buffer_line_t *rob_line);
        void stage_commit();

        inline bool cmp_fetch_block(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_fetch_offset_bits_mask) == (memory_addressB & this->not_fetch_offset_bits_mask);
        }


        inline bool cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }

        inline bool is_busy() {
            return (trace_over == false ||
                    fetch_buffer_position_used != 0 ||
                    decode_buffer_position_used != 0 ||
                    reorder_buffer_position_used != 0);
        }

        /// Fetch Buffer =======================================================
        int32_t fetch_buffer_insert();
        void fetch_buffer_remove();
        int32_t fetch_buffer_find_opcode_number(uint64_t opcode_number);
        /// ====================================================================

        /// Decode Buffer ======================================================
        int32_t decode_buffer_insert();
        void decode_buffer_remove();
        /// ====================================================================

        /// Reorder Buffer =====================================================
        void rob_line_clean(uint32_t reorder_buffer_line);
        std::string rob_line_to_string(uint32_t reorder_buffer_line);
        std::string rob_print_all();
        int32_t rob_insert();
        void rob_remove();
        int32_t rob_find_uop_number(uint64_t uop_number);
        bool rob_check_age();
        /// ====================================================================

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


        INSTANTIATE_GET_SET(uint32_t, read_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, write_buffer_size)

        INSTANTIATE_GET_SET(cache_memory_t*, data_cache)
        INSTANTIATE_GET_SET(cache_memory_t*, inst_cache)

        INSTANTIATE_GET_SET(uint32_t, fetch_block_size)
        INSTANTIATE_GET_SET(uint32_t, branch_per_fetch)

        /// Processor Synchronization
        INSTANTIATE_GET_SET(sync_t, sync_status);
        INSTANTIATE_GET_SET(uint64_t, sync_status_time);

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_stall_cycles)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sync_stall_cycles)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_fetch_opcode_counter)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_decode_uop_counter)

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

        inline void add_stat_instruction_read_completed(uint64_t born_cycle) {
            this->stat_instruction_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            stat_acumulated_instruction_read_wait_time += new_time;
            if (stat_min_instruction_read_wait_time > new_time) stat_min_instruction_read_wait_time = new_time;
            if (stat_max_instruction_read_wait_time < new_time) stat_max_instruction_read_wait_time = new_time;
        };

        inline void add_stat_memory_read_completed(uint64_t born_cycle) {
            this->stat_memory_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_memory_read_wait_time += new_time;
            if (stat_min_memory_read_wait_time > new_time) stat_min_memory_read_wait_time = new_time;
            if (stat_max_memory_read_wait_time < new_time) stat_max_memory_read_wait_time = new_time;
        };

        inline void add_stat_memory_write_completed(uint64_t born_cycle) {
            this->stat_memory_write_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_memory_write_wait_time += new_time;
            if (stat_min_memory_write_wait_time > new_time) stat_min_memory_write_wait_time = new_time;
            if (stat_max_memory_write_wait_time < new_time) stat_max_memory_write_wait_time = new_time;
        };
};
#endif  // _PROCESSOR_PROCESSOR_HPP_
