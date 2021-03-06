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


/// Cache Memory.
class cache_memory_t : public interconnection_interface_t {
    public:
        prefetch_t *prefetcher;                          /// Prefetcher
        line_usage_predictor_t *line_usage_predictor;    /// Line_Usage_Predictor

    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t cache_id;      /// Id corresponds to the cache_memory_array[Id]
        uint32_t bank_number;
        uint32_t total_banks;
        cache_mask_t address_mask_type;

        uint32_t hierarchy_level;
        uint32_t line_size;
        uint32_t line_number;
        uint32_t associativity;
        replacement_t replacement_policy;

        uint32_t penalty_read;
        uint32_t penalty_write;

        uint32_t mshr_request_buffer_size;
        uint32_t mshr_prefetch_buffer_size;
        uint32_t mshr_write_buffer_size;
        uint32_t mshr_eviction_buffer_size;

        uint32_t higher_level_request_tokens;
        uint32_t higher_level_prefetch_tokens;
        uint32_t higher_level_write_tokens;

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        uint64_t bank_bits_mask;        /// NUCA bank mask
        uint64_t bank_bits_shift;

        uint64_t offset_bits_mask;      /// Offset mask
        uint64_t not_offset_bits_mask;  /// Offset mask
        uint64_t offset_bits_shift;

        uint64_t index_bits_mask;       /// Index mask
        uint64_t index_bits_shift;

        uint64_t tag_bits_mask;         /// Tag mask
        uint64_t tag_bits_shift;

        uint32_t total_sets;
        cache_set_t *sets;              /// Internal Memory Storage

        memory_package_t *mshr_buffer;  /// Buffer of Missed Requests
        uint32_t mshr_buffer_size;

        container_ptr_memory_package_t mshr_born_ordered;

        int32_t *mshr_tokens_request;
        int32_t *mshr_tokens_write;
        int32_t *mshr_tokens_prefetch;

        bool got_request_token;
        bool got_prefetch_token;
        bool got_write_token;

        uint64_t send_ans_ready_cycle;
        uint64_t send_rqst_ready_cycle;

        uint64_t recv_ans_ready_cycle;                  /// Ready to receive new READ ANS
        uint64_t recv_rqst_read_ready_cycle;            /// Ready to receive new READ RQST
        uint64_t recv_rqst_write_ready_cycle;           /// Ready to receive new WRITE RQST

        container_ptr_cache_memory_t *higher_level_cache;    /// Higher Level Caches
        container_ptr_cache_memory_t *lower_level_cache;     /// Lower Level Caches

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_accesses;
        uint64_t stat_invalidation;
        uint64_t stat_eviction;
        uint64_t stat_writeback;

        uint64_t stat_final_eviction;
        uint64_t stat_final_writeback;

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

        // HMC
        uint64_t stat_min_hmc_wait_time;
        uint64_t stat_max_hmc_wait_time;
        uint64_t stat_accumulated_hmc_wait_time;

        uint64_t stat_full_mshr_request_buffer;
        uint64_t stat_full_mshr_prefetch_buffer;
        uint64_t stat_full_mshr_write_buffer;
        uint64_t stat_full_mshr_eviction_buffer;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        cache_memory_t();
        ~cache_memory_t();

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        bool pop_token_credit(uint32_t src_id, memory_operation_t memory_operation);
        void push_token_credit(uint32_t src_id, memory_operation_t memory_operation);
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
        uint64_t get_fake_address(uint32_t index, uint32_t way);

        /// Create the tokens for higher level components
        void set_tokens();

        inline uint64_t get_tag(uint64_t addr) {
            return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }

        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }

        inline uint64_t get_offset(uint64_t addr) {
            return (addr & this->offset_bits_mask) >> this->offset_bits_shift;
        }

        inline bool cmp_tag_index_bank(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }

        inline void add_higher_level_cache(cache_memory_t *cache_memory) {
            this->higher_level_cache->push_back(cache_memory);
        }
        INSTANTIATE_GET_SET(container_ptr_cache_memory_t*, higher_level_cache)


        inline void add_lower_level_cache(cache_memory_t *cache_memory) {
            this->lower_level_cache->push_back(cache_memory);
        }
        INSTANTIATE_GET_SET(container_ptr_cache_memory_t*, lower_level_cache)


        void insert_mshr_born_ordered(memory_package_t* package);
        int32_t allocate_request(memory_package_t* package);
        int32_t allocate_prefetch(memory_package_t* package);
        int32_t allocate_write(memory_package_t* package);
        int32_t allocate_eviction(memory_package_t* package);

        cache_line_t* get_line(uint32_t index, uint32_t way);
        cache_line_t* find_line(uint64_t memory_address, uint32_t& index, uint32_t& way);
        cache_line_t* evict_address(uint64_t memory_address, uint32_t& index, uint32_t& way);
        void change_address(cache_line_t *line, uint64_t new_memory_address);
        void change_status(cache_line_t *line, protocol_status_t status);
        void update_last_access(cache_line_t *line);

        /// Methods called by the directory to add statistics and others
        void cache_stats(memory_operation_t memory_operation, bool is_hit);
        void cache_wait(memory_package_t *package);

        INSTANTIATE_GET_SET(uint32_t, cache_id)
        INSTANTIATE_GET_SET(uint32_t, bank_number)
        INSTANTIATE_GET_SET(uint32_t, total_banks)
        INSTANTIATE_GET_SET(cache_mask_t, address_mask_type)

        INSTANTIATE_GET_SET(uint32_t, hierarchy_level)
        INSTANTIATE_GET_SET(uint32_t, line_size)
        INSTANTIATE_GET_SET(uint32_t, line_number)
        INSTANTIATE_GET_SET(uint32_t, associativity)
        INSTANTIATE_GET_SET(uint32_t, total_sets)
        INSTANTIATE_GET_SET(replacement_t, replacement_policy)
        INSTANTIATE_GET_SET(uint32_t, penalty_read)
        INSTANTIATE_GET_SET(uint32_t, penalty_write)
        INSTANTIATE_GET_SET(memory_package_t*, mshr_buffer)

        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_size)

        INSTANTIATE_GET_SET(uint32_t, mshr_request_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_prefetch_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_write_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_eviction_buffer_size)

        INSTANTIATE_GET_SET(uint32_t, higher_level_request_tokens)
        INSTANTIATE_GET_SET(uint32_t, higher_level_prefetch_tokens)
        INSTANTIATE_GET_SET(uint32_t, higher_level_write_tokens)

        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_writeback)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_final_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_final_writeback)

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

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_request_buffer);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_prefetch_buffer);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_write_buffer);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_eviction_buffer);



        inline void add_stat_instruction_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };

        inline void add_stat_write_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_writeback_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_writeback_wait_time += new_time;
            if (this->stat_min_writeback_wait_time > new_time) this->stat_min_writeback_wait_time = new_time;
            if (this->stat_max_writeback_wait_time < new_time) this->stat_max_writeback_wait_time = new_time;
        };

        // HMC
        inline void add_stat_hmc_wait(uint64_t born_cycle) {
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_accumulated_hmc_wait_time += new_time;
            if (this->stat_min_hmc_wait_time > new_time) this->stat_min_hmc_wait_time = new_time;
            if (this->stat_max_hmc_wait_time < new_time) this->stat_max_hmc_wait_time = new_time;
        };


};
