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

class memory_channel_t : public interconnection_interface_t {
    public:
        /// Comes from memory controller
        uint32_t bank_per_channel;
        uint32_t bank_buffer_size;
        selection_t bank_selection_policy;
        uint32_t bank_row_buffer_size;

        request_priority_t request_priority_policy;
        write_priority_t write_priority_policy;

        uint64_t not_column_bits_mask;

        uint64_t bank_bits_mask;
        uint64_t bank_bits_shift;

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
        uint32_t timing_burst;

        /// Set by allocate
        container_ptr_memory_package_t *bank_buffer;
        int32_t *bank_buffer_actual_position;   /// Position inside BankBuffer of the actual request being treated

        bool *bank_is_drain_write;              /// Buffer is in drain_write mode
        uint32_t *bank_number_drain_write;      /// Number of writes treated in drain_write mode
        uint64_t *bank_open_row_address;        /// Address of the actual request being treated

        /// Keep track of the last command in each bank
        memory_controller_command_t *bank_last_command; /// Last command sent to each bank

        uint64_t **bank_last_command_cycle;      /// Cycle of the Last command sent to each bank
        uint64_t *channel_last_command_cycle;      /// Cycle of the last command type

        uint32_t last_bank_selected;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_row_buffer_hit;
        uint64_t stat_row_buffer_miss;

        uint64_t stat_read_forward;
        uint64_t stat_write_forward;
        // ====================================================================
        /// Methods
        // ====================================================================
        memory_channel_t();
        ~memory_channel_t();
        inline const char* get_type_component_label() {
            return "MEMORY_CHANNEL";
        };

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
        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }

        inline bool cmp_row_bank_channel(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_column_bits_mask) == (memory_addressB & this->not_column_bits_mask);
        }


        int32_t find_next_read_operation(uint32_t bank);
        int32_t find_next_write_operation(uint32_t bank);
        int32_t find_next_package(uint32_t bank);

        bool check_if_minimum_latency(uint32_t bank, memory_controller_command_t next_command);

        package_state_t treat_memory_request(memory_package_t *package);


        /// Selection strategies
        uint32_t selection_bank_random();
        uint32_t selection_bank_round_robin();

        INSTANTIATE_GET_SET(uint32_t, bank_per_channel)
        INSTANTIATE_GET_SET(uint32_t, bank_buffer_size)
        INSTANTIATE_GET_SET(selection_t, bank_selection_policy)
        INSTANTIATE_GET_SET(uint32_t, bank_row_buffer_size)
        INSTANTIATE_GET_SET(request_priority_t, request_priority_policy)
        INSTANTIATE_GET_SET(write_priority_t, write_priority_policy)

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

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_row_buffer_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_row_buffer_miss);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_forward);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_forward);
};
