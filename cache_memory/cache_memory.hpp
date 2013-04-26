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

/// Cache Memory.
class cache_memory_t : public interconnection_interface_t {
    public:
        prefetch_t *prefetcher;                          /// Prefetcher
        line_usage_predictor_t *line_usage_predictor;    /// Line_Usage_Predictor

    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
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

        uint32_t mshr_buffer_request_reserved_size;
        uint32_t mshr_buffer_copyback_reserved_size;
        uint32_t mshr_buffer_prefetch_reserved_size;

        uint32_t mshr_request_different_lines_size;
        uint32_t mshr_request_token_window_size;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
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

        cache_line_t *mshr_request_different_lines;
        uint32_t mshr_request_different_lines_used;

        uint64_t send_ans_ready_cycle;
        uint64_t send_rqst_ready_cycle;

        uint64_t recv_ans_ready_cycle;            /// Ready to receive new READ
        uint64_t recv_rqst_read_ready_cycle;            /// Ready to receive new READ
        uint64_t recv_rqst_write_ready_cycle;           /// Ready to receive new WRITE

        container_ptr_cache_memory_t *higher_level_cache;    /// Higher Level Caches
        container_ptr_cache_memory_t *lower_level_cache;     /// Lower Level Caches

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_accesses;
        uint64_t stat_invalidation;
        uint64_t stat_invalidation_copyback;
        uint64_t stat_eviction;
        uint64_t stat_eviction_copyback;

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

        uint64_t stat_full_mshr_buffer_request;
        uint64_t stat_full_mshr_buffer_copyback;
        uint64_t stat_full_mshr_buffer_prefetch;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_memory_t();
        ~cache_memory_t();

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
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

        /// MASKS
        void set_masks();
        uint64_t get_fake_address(uint32_t index, uint32_t way);
        /// Check if MSHR supports the higher levels MSHR
        // ~ void check_mshr_size();

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
        int32_t allocate_copyback(memory_package_t* package);
        int32_t allocate_prefetch(memory_package_t* package);

        cache_line_t* find_line(uint64_t memory_address, uint32_t& index, uint32_t& way);
        cache_line_t* evict_address(uint64_t memory_address, uint32_t& index, uint32_t& way);
        void change_address(cache_line_t *line, uint64_t new_memory_address);
        void change_status(cache_line_t *line, protocol_status_t status);
        void update_last_access(cache_line_t *line);

        /// Methods called by the directory to add statistics and others
        void cache_hit(memory_package_t *package);
        void cache_miss(memory_package_t *package);
        void cache_invalidate(bool is_copyback);
        void cache_evict(bool is_copyback);

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
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_request_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_copyback_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_prefetch_reserved_size)

        INSTANTIATE_GET_SET(uint32_t, mshr_request_different_lines_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_request_token_window_size)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation_copyback)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction_copyback)

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

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_request);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_copyback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_prefetch);


        inline void add_stat_instruction_miss(uint64_t born_cycle) {
            this->stat_instruction_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_miss(uint64_t born_cycle) {
            this->stat_read_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_miss(uint64_t born_cycle) {
            this->stat_prefetch_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };

        inline void add_stat_write_miss(uint64_t born_cycle) {
            this->stat_write_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_miss(uint64_t born_cycle) {
            this->stat_copyback_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };
};
