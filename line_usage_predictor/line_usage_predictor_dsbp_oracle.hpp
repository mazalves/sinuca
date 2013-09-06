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
class line_usage_predictor_dsbp_oracle_t : public line_usage_predictor_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t sub_block_size;

        uint32_t metadata_line_number;          /// Cache Metadata
        uint32_t metadata_associativity;        /// Cache Metadata

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint32_t sub_block_total;

        dsbp_metadata_set_t *metadata_sets;
        uint32_t metadata_total_sets;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_line_hit;
        uint64_t stat_line_miss;
        uint64_t stat_sub_block_miss;
        uint64_t stat_send_writeback;
        uint64_t stat_recv_writeback;
        uint64_t stat_eviction;
        uint64_t stat_invalidation;

        uint64_t stat_sub_block_access_0;
        uint64_t stat_sub_block_access_1;
        uint64_t stat_sub_block_access_2_3;
        uint64_t stat_sub_block_access_4_7;
        uint64_t stat_sub_block_access_8_15;
        uint64_t stat_sub_block_access_16_127;
        uint64_t stat_sub_block_access_128_bigger;

        uint64_t stat_sub_block_write_0;
        uint64_t stat_sub_block_write_1;
        uint64_t stat_sub_block_write_2_3;
        uint64_t stat_sub_block_write_4_7;
        uint64_t stat_sub_block_write_8_15;
        uint64_t stat_sub_block_write_16_127;
        uint64_t stat_sub_block_write_128_bigger;

        uint64_t *stat_accessed_sub_blocks;     /// Number of sub_blocks accessed
        uint64_t *stat_real_write_counter;     /// Number of sub_blocks written

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        line_usage_predictor_dsbp_oracle_t();
        ~line_usage_predictor_dsbp_oracle_t();
        inline const char* get_type_component_label() {
            return "LINE_USAGE_PREDICTOR";
        };

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        // ~ int32_t send_package(memory_package_t *package);
        // ~ bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        // ~ bool check_token_list(memory_package_t *package);
        // ~ uint32_t check_token_space(memory_package_t *package);
        // ~ void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================


        /// ====================================================================
        /// Inheritance from line_usage_predictor_t
        /// ====================================================================
        /// Inspections
        void fill_package_sub_blocks(memory_package_t *package);
        void line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void predict_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);

        bool check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way);
        bool check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);
        bool check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);

        /// Cache Operations
        void line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way);
        void line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);
        void line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);
        /// ====================================================================

        void get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end);

        INSTANTIATE_GET_SET(uint32_t, sub_block_size);
        INSTANTIATE_GET_SET(uint32_t, sub_block_total);

        /// metadata
        INSTANTIATE_GET_SET(dsbp_metadata_set_t*, metadata_sets);
        INSTANTIATE_GET_SET(uint32_t, metadata_line_number);
        INSTANTIATE_GET_SET(uint32_t, metadata_associativity);
        INSTANTIATE_GET_SET(uint32_t, metadata_total_sets);


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_send_writeback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_recv_writeback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_access_128_bigger);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_write_128_bigger);
};
