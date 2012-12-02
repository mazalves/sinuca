/// ============================================================================
//
// Copyright (C) 2010, 2011
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
class directory_controller_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        coherence_protocol_t coherence_protocol_type;
        inclusiveness_t inclusiveness_type;
        bool generate_llc_copyback;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t not_offset_bits_mask;
        container_ptr_cache_memory_t *llc_caches;
        container_ptr_directory_line_t *directory_lines;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_instruction_hit;
        uint64_t stat_read_hit;
        uint64_t stat_prefetch_hit;
        uint64_t stat_write_hit;
        uint64_t stat_copyback_hit;

        uint64_t stat_instruction_miss;
        uint64_t stat_read_miss;
        uint64_t stat_prefetch_miss;
        uint64_t stat_write_miss;
        uint64_t stat_copyback_miss;

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
        directory_controller_t();
        ~directory_controller_t();

        inline const char* get_label() {
            return "DIRECTORY_CTRL";
        };
        inline const char* get_type_component_label() {
            return "DIRECTORY_CTRL";
        };

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

        int32_t find_directory_line(memory_package_t *package);

        package_state_t treat_cache_request(uint32_t obj_id, memory_package_t *package);
        package_state_t treat_cache_answer(uint32_t obj_id, memory_package_t *package);
        package_state_t treat_cache_request_sent(uint32_t obj_id, memory_package_t *package);

        bool create_cache_copyback(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);

        uint32_t find_next_obj_id(cache_memory_t *cache_memory, uint64_t memory_address);
        bool is_locked(uint64_t memory_address);

        bool coherence_is_read(memory_operation_t memory_operation);
        bool coherence_is_hit(cache_line_t *cache_line, memory_operation_t memory_operation);
        bool coherence_need_copyback(cache_memory_t *cache_memory, cache_line_t *cache_line);

        protocol_status_t find_copyback_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address);
        protocol_status_t find_cache_line_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address, bool check_llc);

        void coherence_invalidate_all(cache_memory_t *cache_memory, uint64_t memory_address);
        void coherence_evict_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address);

        void new_statistics(cache_memory_t *cache, memory_package_t *package, bool is_hit);
        void coherence_new_operation(cache_memory_t *cache_memory, cache_line_t *cache_line,  memory_package_t *package, bool is_hit);
        bool inclusiveness_new_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way, memory_package_t *package);

        inline bool cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & not_offset_bits_mask) == (memory_addressB & not_offset_bits_mask);
        };

        inline uint32_t get_directory_lines_size() {
            return this->directory_lines->size();
        };

        INSTANTIATE_GET_SET(container_ptr_directory_line_t*, directory_lines)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)
        INSTANTIATE_GET_SET(coherence_protocol_t, coherence_protocol_type)
        INSTANTIATE_GET_SET(inclusiveness_t, inclusiveness_type)
        INSTANTIATE_GET_SET(bool, generate_llc_copyback)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_prefetch_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback_hit)

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_read_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_write_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_copyback_miss)

        inline void add_stat_instruction_miss(uint64_t born_cycle) {
            this->stat_instruction_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time || this->stat_min_instruction_wait_time == 0) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_miss(uint64_t born_cycle) {
            this->stat_read_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time || this->stat_min_read_wait_time == 0) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_miss(uint64_t born_cycle) {
            this->stat_prefetch_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time || this->stat_min_prefetch_wait_time == 0) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };

        inline void add_stat_write_miss(uint64_t born_cycle) {
            this->stat_write_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time || this->stat_min_write_wait_time == 0) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_miss(uint64_t born_cycle) {
            this->stat_copyback_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time || this->stat_min_copyback_wait_time == 0) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };
};
