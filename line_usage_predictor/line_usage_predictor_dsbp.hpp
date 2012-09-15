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

        uint32_t sub_block_size;
        uint32_t usage_counter_bits;

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

        uint32_t sub_block_total;
        uint32_t usage_counter_max;

        /// metadata
        dsbp_metadata_set_t *metadata_sets;
        uint32_t metadata_total_sets;

        /// pht
        pht_set_t *pht_sets;
        uint32_t pht_total_sets;
        uint64_t pht_index_bits_mask;


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_dsbp_line_sub_block_disable_always;
        uint64_t stat_dsbp_line_sub_block_disable_turnoff;
        uint64_t stat_dsbp_line_sub_block_normal_correct;
        uint64_t stat_dsbp_line_sub_block_normal_over;
        uint64_t stat_dsbp_line_sub_block_learn;
        uint64_t stat_dsbp_line_sub_block_wrong_first;
        uint64_t stat_dsbp_line_sub_block_copyback;

        uint64_t stat_line_miss;
        uint64_t stat_sub_block_miss;
        uint64_t stat_copyback;
        uint64_t stat_eviction;

        uint64_t stat_pht_access;
        uint64_t stat_pht_hit;
        uint64_t stat_pht_miss;

        uint64_t *stat_accessed_sub_block;
        uint64_t *stat_active_sub_block_per_access;     /// Number of active sub_blocks on the line during one access
        uint64_t *stat_active_sub_block_per_cycle;      /// Number of cycles with a set of sub_blocks enabled

        uint64_t stat_sub_block_touch_0;
        uint64_t stat_sub_block_touch_1;
        uint64_t stat_sub_block_touch_2_3;
        uint64_t stat_sub_block_touch_4_7;
        uint64_t stat_sub_block_touch_8_15;
        uint64_t stat_sub_block_touch_16_127;
        uint64_t stat_sub_block_touch_128_bigger;

        /// Number of sub_blocks written
        uint64_t *stat_written_sub_blocks_per_line;
        uint64_t stat_dirty_lines_predicted_dead;
        uint64_t stat_clean_lines_predicted_dead;
        uint64_t stat_written_lines_miss_predicted;

        /// Number of times each sub_block was written before eviction
        uint64_t stat_writes_per_sub_blocks_1;
        uint64_t stat_writes_per_sub_blocks_2;
        uint64_t stat_writes_per_sub_blocks_3;
        uint64_t stat_writes_per_sub_blocks_4;
        uint64_t stat_writes_per_sub_blocks_5;
        uint64_t stat_writes_per_sub_blocks_10;
        uint64_t stat_writes_per_sub_blocks_100;
        uint64_t stat_writes_per_sub_blocks_bigger;

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
        // ~ void allocate_token_list();
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
        bool check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way);
        bool check_line_is_dead(uint32_t index, uint32_t way);

        /// Cache Operations
        void line_hit(memory_package_t *package, uint32_t index, uint32_t way);
        void line_miss(memory_package_t *package, uint32_t index, uint32_t way);
        void sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way);
        void line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way);
        void line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way);
        void line_eviction(uint32_t index, uint32_t way);
        void line_invalidation(uint32_t index, uint32_t way);
        /// ====================================================================

        /// DSBP methdos
        void compute_static_energy(uint32_t index, uint32_t way);
        void get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end);

        INSTANTIATE_GET_SET(uint32_t, sub_block_size);
        INSTANTIATE_GET_SET(uint32_t, sub_block_total);
        INSTANTIATE_GET_SET(uint32_t, usage_counter_bits);
        INSTANTIATE_GET_SET(uint32_t, usage_counter_max);

        /// metadata
        INSTANTIATE_GET_SET(dsbp_metadata_set_t*, metadata_sets);
        INSTANTIATE_GET_SET(uint32_t, metadata_line_number);
        INSTANTIATE_GET_SET(uint32_t, metadata_associativity);
        INSTANTIATE_GET_SET(uint32_t, metadata_total_sets);

        /// pht
        pht_line_t* pht_find_line(uint64_t opcode_address, uint64_t memory_address);
        pht_line_t* pht_evict_address(uint64_t opcode_address, uint64_t memory_address);

        INSTANTIATE_GET_SET(uint32_t, pht_line_number);
        INSTANTIATE_GET_SET(uint32_t, pht_associativity);
        INSTANTIATE_GET_SET(replacement_t, pht_replacement_policy);
        INSTANTIATE_GET_SET(uint32_t, pht_total_sets);

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_disable_always);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_disable_turnoff);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_normal_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_normal_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_learn);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_wrong_first);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dsbp_line_sub_block_copyback);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_access);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_pht_miss);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_128_bigger);


        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dirty_lines_predicted_dead);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_clean_lines_predicted_dead);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_written_lines_miss_predicted);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_2);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_4);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_5);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_10);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_100);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_bigger);
};
