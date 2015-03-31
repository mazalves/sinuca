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

class directory_controller_t : public interconnection_interface_t {
    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        coherence_protocol_t coherence_protocol_type;
        inclusiveness_t inclusiveness_type;
        bool generate_llc_writeback;
        bool generate_non_llc_writeback;
        bool final_writeback_all;

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        uint64_t not_offset_bits_mask;
        container_ptr_cache_memory_t llc_caches;
        container_ptr_directory_line_t directory_lines;
        uint32_t max_cache_level;
        // MVX
        uint64_t total_row_buffer_size;
        uint64_t total_row_buffer_size_bits_mask;
        uint64_t not_total_row_buffer_size_bits_mask;
        uint32_t mvx_operation_size;
        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_instruction_hit;
        uint64_t stat_read_hit;
        uint64_t stat_prefetch_hit;
        uint64_t stat_write_hit;
        uint64_t stat_writeback_recv;

        uint64_t stat_instruction_miss;
        uint64_t stat_read_miss;
        uint64_t stat_prefetch_miss;
        uint64_t stat_write_miss;
        uint64_t stat_writeback_send;

        uint64_t stat_cache_to_cache;
        uint64_t stat_final_writeback_all_cycles;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        directory_controller_t();
        ~directory_controller_t();

        inline const char* get_label() {
            return "DIRECTORY_CTRL";
        };
        inline const char* get_type_component_label() {
            return "DIRECTORY_CTRL";
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

        int32_t find_directory_line(memory_package_t *package);

        package_state_t treat_cache_request(uint32_t obj_id, memory_package_t *package);
        package_state_t treat_cache_answer(uint32_t obj_id, memory_package_t *package);
        package_state_t treat_cache_request_sent(uint32_t obj_id, memory_package_t *package);

        memory_package_t* create_cache_writeback(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);

        uint32_t find_next_obj_id(cache_memory_t *cache_memory, uint64_t memory_address);
        bool is_locked(uint64_t memory_address);

        bool coherence_is_read(memory_operation_t memory_operation);
        bool coherence_is_dirty(protocol_status_t line_status);
        inline bool coherence_is_hit(protocol_status_t line_status);
        inline bool coherence_need_writeback(cache_memory_t *cache_memory, cache_line_t *cache_line);

        protocol_status_t find_writeback_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address);
        protocol_status_t find_cache_line_higher_levels(uint32_t *sum_latency, cache_memory_t *cache_memory, uint64_t memory_address, bool check_llc);

        void coherence_invalidate_all(cache_memory_t *cache_memory, uint64_t memory_address);
        bool coherence_evict_all();
        void coherence_evict_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address);

        void new_statistics(cache_memory_t *cache, memory_operation_t memory_operation, bool is_hit);
        void coherence_new_operation(cache_memory_t *cache_memory, cache_line_t *cache_line,  memory_package_t *package, bool is_hit);
        bool inclusiveness_new_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way, memory_package_t *package);

        inline bool cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & not_offset_bits_mask) == (memory_addressB & not_offset_bits_mask);
        };

        inline bool cmp_total_row_buffer_size(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_total_row_buffer_size_bits_mask) == (memory_addressB & this->not_total_row_buffer_size_bits_mask);
        }


        inline uint32_t get_directory_lines_size() {
            return this->directory_lines.size();
        };

        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)
        INSTANTIATE_GET_SET(coherence_protocol_t, coherence_protocol_type)
        INSTANTIATE_GET_SET(inclusiveness_t, inclusiveness_type)
        INSTANTIATE_GET_SET(bool, generate_llc_writeback)
        INSTANTIATE_GET_SET(bool, generate_non_llc_writeback)
        INSTANTIATE_GET_SET(bool, final_writeback_all)


        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_prefetch_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writeback_recv)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_prefetch_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writeback_send)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_cache_to_cache)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_final_writeback_all_cycles)
};
