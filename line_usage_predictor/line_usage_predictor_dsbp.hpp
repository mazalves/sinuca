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
class line_usage_predictor_dsbp_t : public line_usage_predictor_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t access_counter_bits_read;
        uint32_t bytes_per_subblock;

        bool early_eviction;
        bool early_writeback;
        bool turnoff_dead_lines;

        /// metadata
        uint32_t metadata_line_number;          /// Cache Metadata
        uint32_t metadata_associativity;        /// Cache Metadata

        /// pht
        uint32_t pht_line_number;
        uint32_t pht_associativity;
        replacement_t pht_replacement_policy;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint32_t access_counter_max_read;

        /// metadata
        dsbp_metadata_set_t *metadata_sets;
        uint32_t metadata_total_sets;

        /// pht - misses
        pht_set_t *pht_sets;
        uint32_t pht_total_sets;
        uint64_t pht_index_bits_mask;

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

        uint64_t stat_pht_access;
        uint64_t stat_pht_hit;
        uint64_t stat_pht_miss;

        /// prediction accuracy
        uint64_t stat_dead_read_learn;
        uint64_t stat_dead_read_normal_over;
        uint64_t stat_dead_read_normal_correct;
        uint64_t stat_dead_read_disable_correct;
        uint64_t stat_dead_read_disable_under;

        uint64_t stat_line_read_0;
        uint64_t stat_line_read_1;
        uint64_t stat_line_read_2_3;
        uint64_t stat_line_read_4_7;
        uint64_t stat_line_read_8_15;
        uint64_t stat_line_read_16_127;
        uint64_t stat_line_read_128_bigger;

        uint64_t *stat_subblock_read_per_line;
        uint64_t *stat_subblock_turned_on_during_read;

        uint64_t *cycles_turned_on;
        uint64_t cycles_turned_off;
        uint64_t cycles_turned_off_since_begin;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        line_usage_predictor_dsbp_t();
        ~line_usage_predictor_dsbp_t();

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

        bool check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way);
        bool check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);
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

        INSTANTIATE_GET_SET(uint32_t, access_counter_bits_read);
        INSTANTIATE_GET_SET(uint32_t, access_counter_max_read);

        INSTANTIATE_GET_SET(uint32_t, bytes_per_subblock);

        INSTANTIATE_GET_SET(bool, early_eviction);
        INSTANTIATE_GET_SET(bool, early_writeback);
        INSTANTIATE_GET_SET(bool, turnoff_dead_lines);

        /// metadata
        INSTANTIATE_GET_SET(dsbp_metadata_set_t*, metadata_sets);
        INSTANTIATE_GET_SET(uint32_t, metadata_line_number);
        INSTANTIATE_GET_SET(uint32_t, metadata_associativity);
        INSTANTIATE_GET_SET(uint32_t, metadata_total_sets);

        /// pht misses
        pht_line_t* pht_find_line(uint64_t opcode_address, uint64_t memory_address);
        pht_line_t* pht_evict_address(uint64_t opcode_address, uint64_t memory_address);

        INSTANTIATE_GET_SET(uint32_t, pht_line_number);
        INSTANTIATE_GET_SET(uint32_t, pht_associativity);
        INSTANTIATE_GET_SET(replacement_t, pht_replacement_policy);
        INSTANTIATE_GET_SET(uint32_t, pht_total_sets);

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

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_access);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_miss);

        /// Prediction Accuracy
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_learn);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_normal_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_normal_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_disable_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_disable_under);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_128_bigger);

        INSTANTIATE_GET_SET_ADD(uint64_t, cycles_turned_off);
        INSTANTIATE_GET_SET_ADD(uint64_t, cycles_turned_off_since_begin);
};
