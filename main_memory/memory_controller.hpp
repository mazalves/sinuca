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
class memory_controller_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        memory_controller_mask_t address_mask_type;
        uint32_t line_size;

        uint32_t bus_width;
        uint32_t bus_frequency;
        uint32_t core_to_bus_clock_ratio;

        uint32_t controller_number;
        uint32_t total_controllers;
        uint32_t channels_per_controller;
        uint32_t banks_per_channel;
        uint32_t read_buffer_size;
        uint32_t write_buffer_size;
        uint32_t row_buffer_size;

        request_priority_t request_priority_policy;
        write_priority_t write_priority_policy;

        selection_t bank_selection_policy;
        selection_t channel_selection_policy;

        uint32_t RP_latency;
        uint32_t RCD_latency;
        uint32_t CAS_latency;
        uint32_t RAS_latency;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t row_bits_mask;
        uint64_t row_bits_shift;

        uint64_t bank_bits_mask;
        uint64_t bank_bits_shift;

        uint64_t channel_bits_mask;
        uint64_t channel_bits_shift;

        uint64_t controller_bits_mask;
        uint64_t controller_bits_shift;

        uint64_t column_bits_mask;
        uint64_t not_column_bits_mask;
        uint64_t column_bits_shift;

        uint32_t bus_latency;          /// Ready to send new CAS signal
        uint64_t *bus_ready_cycle;          /// Ready to send new CAS signal
        uint64_t *send_ready_cycle;         /// Ready to send new READ
        uint64_t *recv_read_ready_cycle;    /// Ready to receive new READ
        uint64_t *recv_write_ready_cycle;   /// Ready to receive new WRITE

        uint32_t *last_bank_selected;
        uint32_t last_channel_selected;
        memory_channel_t *channels;

        memory_package_t *fill_buffer;
        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_accesses;
        uint64_t stat_open_new_row;
        uint64_t stat_full_read_buffer;
        uint64_t stat_full_write_buffer;

        uint64_t stat_instruction_completed;
        uint64_t stat_read_completed;
        uint64_t stat_prefetch_completed;
        uint64_t stat_write_completed;
        uint64_t stat_copyback_completed;

        uint64_t stat_min_instruction_wait_time;
        uint64_t stat_max_instruction_wait_time;
        uint64_t stat_acumulated_instruction_wait_time;

        uint64_t stat_min_read_wait_time;
        uint64_t stat_max_read_wait_time;
        uint64_t stat_acumulated_read_wait_time;

        uint64_t stat_min_prefetch_wait_time;
        uint64_t stat_max_prefetch_wait_time;
        uint64_t stat_acumulated_prefetch_wait_time;

        uint64_t stat_min_write_wait_time;
        uint64_t stat_max_write_wait_time;
        uint64_t stat_acumulated_write_wait_time;

        uint64_t stat_min_copyback_wait_time;
        uint64_t stat_max_copyback_wait_time;
        uint64_t stat_acumulated_copyback_wait_time;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        memory_controller_t();
        ~memory_controller_t();

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

        /// Selection strategies
        uint32_t selection_bank_random(uint32_t total_buffers);
        uint32_t selection_bank_round_robin(uint32_t channel, uint32_t total_buffers);

        uint32_t selection_channel_random(uint32_t total_channels);
        uint32_t selection_channel_round_robin(uint32_t total_channels);

        /// MASKS
        void set_masks();

        inline uint64_t get_row(uint64_t addr) {
            return (addr & this->row_bits_mask) >> this->row_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }

        inline uint64_t get_channel(uint64_t addr) {
            return (addr & this->channel_bits_mask) >> this->channel_bits_shift;
        }

        inline uint64_t get_controller(uint64_t addr) {
            return (addr & this->controller_bits_mask) >> this->controller_bits_shift;
        }

        inline uint64_t get_column(uint64_t addr) {
            return (addr & this->column_bits_mask) >> this->column_bits_shift;
        }

        inline bool cmp_row_bank_channel(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_column_bits_mask) == (memory_addressB & this->not_column_bits_mask);
        }

        void find_cas_and_ras(memory_package_t *input_buffer, uint32_t input_buffer_size, memory_package_t *row_buffer, int32_t& cas_position, int32_t& ras_position);

        INSTANTIATE_GET_SET(memory_controller_mask_t, address_mask_type)
        INSTANTIATE_GET_SET(uint32_t, line_size)

        INSTANTIATE_GET_SET(uint32_t, bus_width)
        INSTANTIATE_GET_SET(uint32_t, bus_frequency)
        INSTANTIATE_GET_SET(uint32_t, core_to_bus_clock_ratio)

        INSTANTIATE_GET_SET(uint32_t, controller_number)
        INSTANTIATE_GET_SET(uint32_t, total_controllers)
        INSTANTIATE_GET_SET(uint32_t, channels_per_controller)
        INSTANTIATE_GET_SET(uint32_t, banks_per_channel)
        INSTANTIATE_GET_SET(uint32_t, read_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, write_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, row_buffer_size)

        INSTANTIATE_GET_SET(selection_t, bank_selection_policy)
        INSTANTIATE_GET_SET(selection_t, channel_selection_policy)

        INSTANTIATE_GET_SET(request_priority_t, request_priority_policy)
        INSTANTIATE_GET_SET(write_priority_t, write_priority_policy)

        INSTANTIATE_GET_SET(uint32_t, bus_latency)

        INSTANTIATE_GET_SET(uint32_t, RP_latency)
        INSTANTIATE_GET_SET(uint32_t, RCD_latency)
        INSTANTIATE_GET_SET(uint32_t, CAS_latency)
        INSTANTIATE_GET_SET(uint32_t, RAS_latency)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_open_new_row)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_read_buffer)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_write_buffer)

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_read_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_write_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_copyback_completed)

        inline void add_stat_instruction_completed(uint64_t born_cycle) {
            this->stat_instruction_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_completed(uint64_t born_cycle) {
            this->stat_read_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_completed(uint64_t born_cycle) {
            this->stat_prefetch_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };


        inline void add_stat_write_completed(uint64_t born_cycle) {
            this->stat_write_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_completed(uint64_t born_cycle) {
            this->stat_copyback_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };

};
