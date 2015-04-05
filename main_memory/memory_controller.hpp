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

class memory_controller_t : public interconnection_interface_t {
    private:

    #ifdef BURST_TRACE
        char *burst_rqst_name;
        char *burst_wback_name;
        char *burst_pftch_name;

        std::ofstream burst_rqst_file;
        std::ofstream burst_wback_file;
        std::ofstream burst_pftch_file;
    #endif

        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        memory_controller_mask_t address_mask_type;
        uint32_t line_size;

        uint32_t controller_number;
        uint32_t total_controllers;

        uint32_t mshr_buffer_request_reserved_size;
        uint32_t mshr_buffer_writeback_reserved_size;
        uint32_t mshr_buffer_prefetch_reserved_size;

        uint32_t channels_per_controller;
        uint32_t bank_per_channel;
        uint32_t bank_buffer_size;
        selection_t bank_selection_policy;
        uint32_t bank_row_buffer_size;

        request_priority_t request_priority_policy;
        write_priority_t write_priority_policy;

        /// DRAM configuration
        uint32_t bus_frequency; // DRAM to core bus frequency
        uint32_t burst_length;  // DDR1 has a BL=2, DDR2 has a BL=4, DDR3 has a BL=8
        float core_to_bus_clock_ratio;  // Ratio between PROCESSOR and BUS frequency

        /// All parameters given in nCK
        /// 1 nCK = (IO Bus Cycle)
        uint32_t timing_al;     // added latency for column accesses
        uint32_t timing_cas;    // column access strobe (cl) latency
        uint32_t timing_ccd;    // column to column delay
        uint32_t timing_cwd;    // column write delay (cwl)
        uint32_t timing_faw;    // four (row) activation window
        uint32_t timing_ras;    // row access strobe
        uint32_t timing_rc;     // row cycle
        uint32_t timing_rcd;    // row to column comand delay
        uint32_t timing_rp;     // row precharge
        uint32_t timing_rrd;    // row activation to row activation delay
        uint32_t timing_rtp;    // read to precharge
        uint32_t timing_wr;     // write recovery time
        uint32_t timing_wtr;    // write to read delay time

        // MVX
        uint32_t mvx_operation_size;

        uint32_t mvx_latency_int_alu;
        uint32_t mvx_latency_int_mul;
        uint32_t mvx_latency_int_div;

        uint32_t mvx_latency_fp_alu;
        uint32_t mvx_latency_fp_mul;
        uint32_t mvx_latency_fp_div;


        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        uint64_t not_column_bits_mask;

        uint64_t colrow_bits_mask;
        uint64_t colrow_bits_shift;

        uint64_t colbyte_bits_mask;
        uint64_t colbyte_bits_shift;

        uint64_t row_bits_mask;
        uint64_t row_bits_shift;

        uint64_t bank_bits_mask;
        uint64_t bank_bits_shift;

        uint64_t channel_bits_mask;
        uint64_t channel_bits_shift;

        uint64_t controller_bits_mask;
        uint64_t controller_bits_shift;


        uint64_t send_ready_cycle;         /// Ready to send new Answer
        uint64_t recv_ready_cycle;         /// Ready to receive new Request

        uint32_t mshr_buffer_size;
        memory_package_t *mshr_buffer;
        container_ptr_memory_package_t mshr_born_ordered;
        uint32_t timing_burst;
        memory_channel_t *channels;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_accesses;

        uint64_t stat_instruction_completed;
        uint64_t stat_read_completed;
        uint64_t stat_prefetch_completed;
        uint64_t stat_write_completed;
        uint64_t stat_writeback_completed;

        uint64_t stat_min_instruction_wait_time;
        uint64_t stat_max_instruction_wait_time;
        uint64_t stat_accumulated_instruction_wait_time;

        uint64_t stat_min_read_wait_time;
        uint64_t stat_max_read_wait_time;
        uint64_t stat_accumulated_read_wait_time;

        uint64_t stat_min_prefetch_wait_time;
        uint64_t stat_max_prefetch_wait_time;
        uint64_t stat_accumulated_prefetch_wait_time;

        uint64_t stat_min_write_wait_time;
        uint64_t stat_max_write_wait_time;
        uint64_t stat_accumulated_write_wait_time;

        uint64_t stat_min_writeback_wait_time;
        uint64_t stat_max_writeback_wait_time;
        uint64_t stat_accumulated_writeback_wait_time;

        // MVX

        uint64_t stat_mvx_lock_completed;
        uint64_t stat_mvx_unlock_completed;
        uint64_t stat_mvx_load_completed;
        uint64_t stat_mvx_store_completed;
        uint64_t stat_mvx_int_alu_completed;
        uint64_t stat_mvx_int_mul_completed;
        uint64_t stat_mvx_int_div_completed;
        uint64_t stat_mvx_fp_alu_completed;
        uint64_t stat_mvx_fp_mul_completed;
        uint64_t stat_mvx_fp_div_completed;

        uint64_t stat_min_mvx_lock_wait_time;
        uint64_t stat_max_mvx_lock_wait_time;
        uint64_t stat_accumulated_mvx_lock_wait_time;

        uint64_t stat_min_mvx_unlock_wait_time;
        uint64_t stat_max_mvx_unlock_wait_time;
        uint64_t stat_accumulated_mvx_unlock_wait_time;

        uint64_t stat_min_mvx_load_wait_time;
        uint64_t stat_max_mvx_load_wait_time;
        uint64_t stat_accumulated_mvx_load_wait_time;

        uint64_t stat_min_mvx_store_wait_time;
        uint64_t stat_max_mvx_store_wait_time;
        uint64_t stat_accumulated_mvx_store_wait_time;

        uint64_t stat_min_mvx_int_alu_wait_time;
        uint64_t stat_max_mvx_int_alu_wait_time;
        uint64_t stat_accumulated_mvx_int_alu_wait_time;

        uint64_t stat_min_mvx_int_mul_wait_time;
        uint64_t stat_max_mvx_int_mul_wait_time;
        uint64_t stat_accumulated_mvx_int_mul_wait_time;

        uint64_t stat_min_mvx_int_div_wait_time;
        uint64_t stat_max_mvx_int_div_wait_time;
        uint64_t stat_accumulated_mvx_int_div_wait_time;

        uint64_t stat_min_mvx_fp_alu_wait_time;
        uint64_t stat_max_mvx_fp_alu_wait_time;
        uint64_t stat_accumulated_mvx_fp_alu_wait_time;

        uint64_t stat_min_mvx_fp_mul_wait_time;
        uint64_t stat_max_mvx_fp_mul_wait_time;
        uint64_t stat_accumulated_mvx_fp_mul_wait_time;

        uint64_t stat_min_mvx_fp_div_wait_time;
        uint64_t stat_max_mvx_fp_div_wait_time;
        uint64_t stat_accumulated_mvx_fp_div_wait_time;


        uint64_t stat_full_mshr_buffer_request;
        uint64_t stat_full_mshr_buffer_writeback;
        uint64_t stat_full_mshr_buffer_prefetch;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        memory_controller_t();
        ~memory_controller_t();

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

        /// MASKS
        void set_masks();

        inline uint64_t get_controller(uint64_t addr) {
            return (addr & this->controller_bits_mask) >> this->controller_bits_shift;
        }

        inline uint64_t get_channel(uint64_t addr) {
            return (addr & this->channel_bits_mask) >> this->channel_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->channels[0].bank_bits_mask) >> this->channels[0].bank_bits_shift;
        }


        void insert_mshr_born_ordered(memory_package_t* package);
        int32_t allocate_request(memory_package_t* package);
        int32_t allocate_writeback(memory_package_t* package);
        int32_t allocate_prefetch(memory_package_t* package);

        INSTANTIATE_GET_SET(memory_controller_mask_t, address_mask_type)
        INSTANTIATE_GET_SET(uint32_t, line_size)

        INSTANTIATE_GET_SET(uint32_t, controller_number)
        INSTANTIATE_GET_SET(uint32_t, total_controllers)

        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_request_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_writeback_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_prefetch_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_size)

        INSTANTIATE_GET_SET(uint32_t, channels_per_controller)
        INSTANTIATE_GET_SET(uint32_t, bank_per_channel)

        INSTANTIATE_GET_SET(uint32_t, bank_buffer_size)
        INSTANTIATE_GET_SET(selection_t, bank_selection_policy)
        INSTANTIATE_GET_SET(uint32_t, bank_row_buffer_size)

        INSTANTIATE_GET_SET(request_priority_t, request_priority_policy)
        INSTANTIATE_GET_SET(write_priority_t, write_priority_policy)

        /// DRAM configuration
        INSTANTIATE_GET_SET(uint32_t, bus_frequency)
        INSTANTIATE_GET_SET(uint32_t, burst_length)
        INSTANTIATE_GET_SET(float, core_to_bus_clock_ratio)

        /// All parameters given in nCK
        /// 1 nCK = (IO Bus Cycle)
        INSTANTIATE_GET_SET(uint32_t, timing_burst)
        INSTANTIATE_GET_SET(uint32_t, timing_al)
        INSTANTIATE_GET_SET(uint32_t, timing_cas)
        INSTANTIATE_GET_SET(uint32_t, timing_ccd)
        INSTANTIATE_GET_SET(uint32_t, timing_cwd)
        INSTANTIATE_GET_SET(uint32_t, timing_faw)
        INSTANTIATE_GET_SET(uint32_t, timing_ras)
        INSTANTIATE_GET_SET(uint32_t, timing_rc)
        INSTANTIATE_GET_SET(uint32_t, timing_rcd)
        INSTANTIATE_GET_SET(uint32_t, timing_rp)
        INSTANTIATE_GET_SET(uint32_t, timing_rrd)
        INSTANTIATE_GET_SET(uint32_t, timing_rtp)
        INSTANTIATE_GET_SET(uint32_t, timing_wr)
        INSTANTIATE_GET_SET(uint32_t, timing_wtr)

        //MVX
        INSTANTIATE_GET_SET(uint32_t, mvx_operation_size)

        INSTANTIATE_GET_SET(uint32_t, mvx_latency_int_alu)
        INSTANTIATE_GET_SET(uint32_t, mvx_latency_int_mul)
        INSTANTIATE_GET_SET(uint32_t, mvx_latency_int_div)

        INSTANTIATE_GET_SET(uint32_t, mvx_latency_fp_alu)
        INSTANTIATE_GET_SET(uint32_t, mvx_latency_fp_mul)
        INSTANTIATE_GET_SET(uint32_t, mvx_latency_fp_div)


        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_request);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_writeback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_prefetch);

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_read_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_write_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_writeback_completed)

        inline void add_stat_instruction_completed(uint64_t born_cycle) {
            this->stat_instruction_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_completed(uint64_t born_cycle) {
            this->stat_read_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_completed(uint64_t born_cycle) {
            this->stat_prefetch_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };


        inline void add_stat_write_completed(uint64_t born_cycle) {
            this->stat_write_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_writeback_completed(uint64_t born_cycle) {
            this->stat_writeback_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_writeback_wait_time += new_time;
            if (this->stat_min_writeback_wait_time > new_time) this->stat_min_writeback_wait_time = new_time;
            if (this->stat_max_writeback_wait_time < new_time) this->stat_max_writeback_wait_time = new_time;
        };

        // MVX
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_lock_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_unlock_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_load_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_store_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_int_alu_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_int_mul_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_int_div_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_fp_alu_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_fp_mul_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_mvx_fp_div_completed)

        inline void add_stat_mvx_lock_completed(uint64_t born_cycle) {
            this->stat_mvx_lock_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_lock_wait_time += new_time;
            if (this->stat_min_mvx_lock_wait_time > new_time) this->stat_min_mvx_lock_wait_time = new_time;
            if (this->stat_max_mvx_lock_wait_time < new_time) this->stat_max_mvx_lock_wait_time = new_time;
        };
        inline void add_stat_mvx_unlock_completed(uint64_t born_cycle) {
            this->stat_mvx_unlock_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_unlock_wait_time += new_time;
            if (this->stat_min_mvx_unlock_wait_time > new_time) this->stat_min_mvx_unlock_wait_time = new_time;
            if (this->stat_max_mvx_unlock_wait_time < new_time) this->stat_max_mvx_unlock_wait_time = new_time;
        };


        inline void add_stat_mvx_load_completed(uint64_t born_cycle) {
            this->stat_mvx_load_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_load_wait_time += new_time;
            if (this->stat_min_mvx_load_wait_time > new_time) this->stat_min_mvx_load_wait_time = new_time;
            if (this->stat_max_mvx_load_wait_time < new_time) this->stat_max_mvx_load_wait_time = new_time;
        };
        inline void add_stat_mvx_store_completed(uint64_t born_cycle) {
            this->stat_mvx_store_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_store_wait_time += new_time;
            if (this->stat_min_mvx_store_wait_time > new_time) this->stat_min_mvx_store_wait_time = new_time;
            if (this->stat_max_mvx_store_wait_time < new_time) this->stat_max_mvx_store_wait_time = new_time;
        };

        inline void add_stat_mvx_int_alu_completed(uint64_t born_cycle) {
            this->stat_mvx_int_alu_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_int_alu_wait_time += new_time;
            if (this->stat_min_mvx_int_alu_wait_time > new_time) this->stat_min_mvx_int_alu_wait_time = new_time;
            if (this->stat_max_mvx_int_alu_wait_time < new_time) this->stat_max_mvx_int_alu_wait_time = new_time;
        };
        inline void add_stat_mvx_int_mul_completed(uint64_t born_cycle) {
            this->stat_mvx_int_mul_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_int_mul_wait_time += new_time;
            if (this->stat_min_mvx_int_mul_wait_time > new_time) this->stat_min_mvx_int_mul_wait_time = new_time;
            if (this->stat_max_mvx_int_mul_wait_time < new_time) this->stat_max_mvx_int_mul_wait_time = new_time;
        };
        inline void add_stat_mvx_int_div_completed(uint64_t born_cycle) {
            this->stat_mvx_int_div_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_int_div_wait_time += new_time;
            if (this->stat_min_mvx_int_div_wait_time > new_time) this->stat_min_mvx_int_div_wait_time = new_time;
            if (this->stat_max_mvx_int_div_wait_time < new_time) this->stat_max_mvx_int_div_wait_time = new_time;
        };

        inline void add_stat_mvx_fp_alu_completed(uint64_t born_cycle) {
            this->stat_mvx_fp_alu_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_fp_alu_wait_time += new_time;
            if (this->stat_min_mvx_fp_alu_wait_time > new_time) this->stat_min_mvx_fp_alu_wait_time = new_time;
            if (this->stat_max_mvx_fp_alu_wait_time < new_time) this->stat_max_mvx_fp_alu_wait_time = new_time;
        };
        inline void add_stat_mvx_fp_mul_completed(uint64_t born_cycle) {
            this->stat_mvx_fp_mul_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_fp_mul_wait_time += new_time;
            if (this->stat_min_mvx_fp_mul_wait_time > new_time) this->stat_min_mvx_fp_mul_wait_time = new_time;
            if (this->stat_max_mvx_fp_mul_wait_time < new_time) this->stat_max_mvx_fp_mul_wait_time = new_time;
        };
        inline void add_stat_mvx_fp_div_completed(uint64_t born_cycle) {
            this->stat_mvx_fp_div_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_accumulated_mvx_fp_div_wait_time += new_time;
            if (this->stat_min_mvx_fp_div_wait_time > new_time) this->stat_min_mvx_fp_div_wait_time = new_time;
            if (this->stat_max_mvx_fp_div_wait_time < new_time) this->stat_max_mvx_fp_div_wait_time = new_time;
        };


};
