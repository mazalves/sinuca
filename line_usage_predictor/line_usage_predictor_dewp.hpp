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

class line_usage_predictor_dewp_t : public line_usage_predictor_t {
    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t access_counter_bits_read;
        uint32_t access_counter_bits_writeback;

        bool early_eviction;
        bool early_writeback;
        bool turnoff_dead_lines;

        /// metadata
        uint32_t metadata_line_number;          /// Cache Metadata
        uint32_t metadata_associativity;        /// Cache Metadata

        /// aht
        uint32_t aht_line_number;
        uint32_t aht_associativity;
        replacement_t aht_replacement_policy;


        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        uint32_t access_counter_max_read;
        uint32_t access_counter_max_writeback;

        /// metadata
        dewp_metadata_set_t *metadata_sets;
        uint32_t metadata_total_sets;

        /// aht
        aht_set_t *aht_sets;
        uint32_t aht_total_sets;
        uint64_t aht_index_bits_mask;


         // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_line_hit;
        uint64_t stat_line_miss;
        uint64_t stat_sub_block_miss;
        uint64_t stat_send_writeback;
        uint64_t stat_recv_writeback;
        uint64_t stat_eviction;
        uint64_t stat_invalidation;

        uint64_t stat_aht_access;
        uint64_t stat_aht_hit;
        uint64_t stat_aht_miss;

        /// prediction accuracy
        uint64_t stat_dead_read_learn;
        uint64_t stat_dead_read_normal_over;
        uint64_t stat_dead_read_normal_correct;
        uint64_t stat_dead_read_disable_correct;
        uint64_t stat_dead_read_disable_under;

        uint64_t stat_dead_writeback_learn;
        uint64_t stat_dead_writeback_notsent_over;
        uint64_t stat_dead_writeback_notsent_correct;
        uint64_t stat_dead_writeback_sent_correct;
        uint64_t stat_dead_writeback_sent_under;

        uint64_t stat_line_read_0;
        uint64_t stat_line_read_1;
        uint64_t stat_line_read_2_3;
        uint64_t stat_line_read_4_7;
        uint64_t stat_line_read_8_15;
        uint64_t stat_line_read_16_127;
        uint64_t stat_line_read_128_bigger;

        uint64_t stat_line_writeback_0;
        uint64_t stat_line_writeback_1;
        uint64_t stat_line_writeback_2_3;
        uint64_t stat_line_writeback_4_7;
        uint64_t stat_line_writeback_8_15;
        uint64_t stat_line_writeback_16_127;
        uint64_t stat_line_writeback_128_bigger;

        uint64_t stat_cycles_turned_on_whole_line;
        uint64_t stat_cycles_turned_off_whole_line;

        uint64_t stat_cycles_turned_off_whole_line_since_begin;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        line_usage_predictor_dewp_t();
        ~line_usage_predictor_dewp_t();

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        // ~ int32_t send_package(memory_package_t *package);
        // ~ bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        // ~ bool pop_token_credit(uint32_t src_id, memory_operation_t memory_operation);
        // ~ uint32_t check_token_space(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        // ====================================================================


        // ====================================================================
        /// Inheritance from line_usage_predictor_t
        // ====================================================================
        /// Inspections
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
        // ====================================================================

        INSTANTIATE_GET_SET(uint32_t, access_counter_bits_read);
        INSTANTIATE_GET_SET(uint32_t, access_counter_bits_writeback);
        INSTANTIATE_GET_SET(uint32_t, access_counter_max_read);
        INSTANTIATE_GET_SET(uint32_t, access_counter_max_writeback);

        INSTANTIATE_GET_SET(bool, early_eviction);
        INSTANTIATE_GET_SET(bool, early_writeback);
        INSTANTIATE_GET_SET(bool, turnoff_dead_lines);

        /// metadata
        INSTANTIATE_GET_SET(dewp_metadata_set_t*, metadata_sets);
        INSTANTIATE_GET_SET(uint32_t, metadata_line_number);
        INSTANTIATE_GET_SET(uint32_t, metadata_associativity);
        INSTANTIATE_GET_SET(uint32_t, metadata_total_sets);

        /// aht
        aht_line_t* aht_find_line(uint64_t opcode_address, uint64_t memory_address);
        aht_line_t* aht_evict_address(uint64_t opcode_address, uint64_t memory_address);

        INSTANTIATE_GET_SET(uint32_t, aht_line_number);
        INSTANTIATE_GET_SET(uint32_t, aht_associativity);
        INSTANTIATE_GET_SET(replacement_t, aht_replacement_policy);
        INSTANTIATE_GET_SET(uint32_t, aht_total_sets);



        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_miss);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_send_writeback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_recv_writeback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_aht_access);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_aht_hit);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_aht_miss);

        /// Prediction Accuracy
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_learn);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_normal_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_normal_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_disable_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_read_disable_under);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_writeback_learn);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_writeback_notsent_over);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_writeback_notsent_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_writeback_sent_correct);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dead_writeback_sent_under);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_read_128_bigger);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_0);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_1);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_2_3);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_4_7);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_8_15);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_16_127);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_writeback_128_bigger);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_cycles_turned_on_whole_line);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_cycles_turned_off_whole_line);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_cycles_turned_off_whole_line_since_begin);
};
