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
class line_usage_predictor_dlec_t : public line_usage_predictor_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t usage_counter_bits;

        /// metadata
        uint32_t metadata_line_number;          /// Cache Metadata
        uint32_t metadata_associativity;        /// Cache Metadata

        /// aht - misses
        uint32_t ahtm_line_number;
        uint32_t ahtm_associativity;
        replacement_t ahtm_replacement_policy;

        /// aht - cpyback
        uint32_t ahtc_line_number;
        uint32_t ahtc_associativity;
        replacement_t ahtc_replacement_policy;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint32_t usage_counter_max;

        /// metadata
        dlec_metadata_set_t *metadata_sets;
        uint32_t metadata_total_sets;

        /// aht - misses
        aht_set_t *ahtm_sets;
        uint32_t ahtm_total_sets;
        uint64_t ahtm_index_bits_mask;

        /// aht - copyback
        aht_set_t *ahtc_sets;
        uint32_t ahtc_total_sets;
        uint64_t ahtc_index_bits_mask;

         /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_line_hit;
        uint64_t stat_line_miss;
        uint64_t stat_sub_block_miss;
        uint64_t stat_copyback;
        uint64_t stat_eviction;
        uint64_t stat_invalidation;

        uint64_t stat_ahtm_access;
        uint64_t stat_ahtm_hit;
        uint64_t stat_ahtm_miss;

        uint64_t stat_ahtc_access;
        uint64_t stat_ahtc_hit;
        uint64_t stat_ahtc_miss;

        uint64_t stat_line_sub_block_learn;
        uint64_t stat_line_sub_block_normal_over;
        uint64_t stat_line_sub_block_normal_correct;
        uint64_t stat_line_sub_block_disable_correct;
        uint64_t stat_line_sub_block_disable_under;

        uint64_t stat_line_sub_block_copyback_over;
        uint64_t stat_line_sub_block_copyback_correct;
        uint64_t stat_line_sub_block_copyback_under;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        line_usage_predictor_dlec_t();
        ~line_usage_predictor_dlec_t();

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

        INSTANTIATE_GET_SET(uint32_t, usage_counter_bits);
        INSTANTIATE_GET_SET(uint32_t, usage_counter_max);

        /// metadata
        INSTANTIATE_GET_SET(dlec_metadata_set_t*, metadata_sets);
        INSTANTIATE_GET_SET(uint32_t, metadata_line_number);
        INSTANTIATE_GET_SET(uint32_t, metadata_associativity);
        INSTANTIATE_GET_SET(uint32_t, metadata_total_sets);

        /// aht misses
        aht_line_t* ahtm_find_line(uint64_t opcode_address, uint64_t memory_address);
        aht_line_t* ahtm_evict_address(uint64_t opcode_address, uint64_t memory_address);

        INSTANTIATE_GET_SET(uint32_t, ahtm_line_number);
        INSTANTIATE_GET_SET(uint32_t, ahtm_associativity);
        INSTANTIATE_GET_SET(replacement_t, ahtm_replacement_policy);
        INSTANTIATE_GET_SET(uint32_t, ahtm_total_sets);

        /// aht copyback
        aht_line_t* ahtc_find_line(uint64_t opcode_address, uint64_t memory_address);
        aht_line_t* ahtc_evict_address(uint64_t opcode_address, uint64_t memory_address);

        INSTANTIATE_GET_SET(uint32_t, ahtc_line_number);
        INSTANTIATE_GET_SET(uint32_t, ahtc_associativity);
        INSTANTIATE_GET_SET(replacement_t, ahtc_replacement_policy);
        INSTANTIATE_GET_SET(uint32_t, ahtc_total_sets);

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtm_access);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtm_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtm_miss);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtc_access);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtc_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_ahtc_miss);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_learn);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_normal_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_normal_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_disable_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_disable_under);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_copyback_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_copyback_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_sub_block_copyback_under);
};
