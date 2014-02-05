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
#include "../sinuca.hpp"

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

/// ============================================================================
line_usage_predictor_dsbp_t::line_usage_predictor_dsbp_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DSBP;

    this->early_eviction = true;
    this->early_writeback = true;
    this->turnoff_dead_lines = true;

    this->access_counter_bits_read = 0;
    this->access_counter_max_read = 0;

    this->bytes_per_subblock = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;

    /// pht
    this->pht_sets = NULL;
    this->pht_line_number = 0;
    this->pht_associativity = 0;
    this->pht_total_sets = 0;
    this->pht_replacement_policy = REPLACEMENT_LRU;

};

/// ============================================================================
line_usage_predictor_dsbp_t::~line_usage_predictor_dsbp_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dsbp_metadata_set_t>(metadata_sets);
    utils_t::template_delete_array<pht_set_t>(pht_sets);

    utils_t::template_delete_array<uint64_t>(cycles_turned_on_whole_line);
    utils_t::template_delete_array<uint64_t>(stat_touched_sub_blocks_per_line);
    utils_t::template_delete_array<uint64_t>(stat_turned_on_sub_blocks_per_access);
};

/// ============================================================================
void line_usage_predictor_dsbp_t::allocate() {
    line_usage_predictor_t::allocate();

    this->access_counter_max_read = pow(2, this->access_counter_bits_read) - 1;

    ///=========================================================================
    /// metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s metadata %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());
    this->metadata_sets = utils_t::template_allocate_array<dsbp_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dsbp_metadata_line_t>(this->get_metadata_associativity());
        for (uint32_t j = 0; j < this->get_metadata_associativity(); j++) {
            this->metadata_sets[i].ways[j].sub_blocks = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);

            this->metadata_sets[i].ways[j].real_access_counter_read = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].access_counter_read = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->metadata_sets[i].ways[j].overflow_read = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
        }
    }

    ///=========================================================================
    /// pht misses
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_pht_line_number() / this->get_pht_associativity()),
                        "Wrong line number(%u) or associativity(%u).\n", this->get_pht_line_number(), this->get_pht_associativity());
    this->set_pht_total_sets(this->get_pht_line_number() / this->get_pht_associativity());
    this->pht_sets = utils_t::template_allocate_array<pht_set_t>(this->get_pht_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s pht %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_pht_line_number(), this->get_pht_associativity(), this->get_pht_total_sets());

    /// INDEX MASK
    this->pht_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_pht_total_sets()); i++) {
        this->pht_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_pht_total_sets(); i++) {
        this->pht_sets[i].ways = utils_t::template_allocate_array<pht_line_t>(this->get_pht_associativity());
        for (uint32_t j = 0; j < this->get_pht_associativity(); j++) {
            this->pht_sets[i].ways[j].access_counter_read = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->pht_sets[i].ways[j].overflow_read = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
        }

    }

    /// STATISTICS
    this->cycles_turned_on_whole_line = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_touched_sub_blocks_per_line = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_turned_on_sub_blocks_per_access = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
};

/// ============================================================================
void line_usage_predictor_dsbp_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


/// ============================================================================
void line_usage_predictor_dsbp_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(package->memory_size > 0, "Received a request with invalid size.\n")

    /// Compute the START and END sub_blocks
    uint32_t sub_block_ini, sub_block_end;

    uint64_t address_offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t address_size = address_offset + package->memory_size;
    if (address_size >= sinuca_engine.get_global_line_size()){
        address_size = sinuca_engine.get_global_line_size();
    }

    sub_block_ini = uint32_t(address_offset / this->bytes_per_subblock) * this->bytes_per_subblock;
    sub_block_end = uint32_t( (address_size / this->bytes_per_subblock) +
                                        uint32_t((address_size % this->bytes_per_subblock) != 0) ) * this->bytes_per_subblock;


    /// Generates the Request Vector
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        package->sub_blocks[i] = ( i >= sub_block_ini && i < sub_block_end );
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Package AFTER fill_package %s\n", package->content_to_string().c_str())

};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;

    package->memory_size = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        package->sub_blocks[i] = (this->metadata_sets[index].ways[way].sub_blocks[i] ||
                                (this->metadata_sets[index].ways[way].access_counter_read[i] != 0));

        package->memory_size += package->sub_blocks[i];
    }
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Package AFTER line_to_subblock %s\n", package->content_to_string().c_str())
};

/// ============================================================================
bool line_usage_predictor_dsbp_t::check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;

    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        return false;
    }

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i] == true &&
        this->metadata_sets[index].ways[way].sub_blocks[i] == false) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
            return false;
        }
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
    return true;

};

/// ============================================================================
bool line_usage_predictor_dsbp_t::check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
        return true;
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
        return false;
    }
};


/// ============================================================================
bool line_usage_predictor_dsbp_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return (this->metadata_sets[index].ways[way].learn_mode == false &&
            this->metadata_sets[index].ways[way].is_dead_read &&
            this->early_eviction);
};

/// ============================================================================
bool line_usage_predictor_dsbp_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_write()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return (this->metadata_sets[index].ways[way].learn_mode == false &&
            this->metadata_sets[index].ways[way].is_dead_read &&
            this->metadata_sets[index].ways[way].is_dirty &&
            this->early_writeback);
};

/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_dsbp_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].active_sub_blocks <= sinuca_engine.get_global_line_size() &&
                        this->metadata_sets[index].ways[way].active_sub_blocks > 0,
                        "Wrong number of active_sub_blocks %d", this->metadata_sets[index].ways[way].active_sub_blocks);

    (void)package;
    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_line_hit();

    /// Statistics
    this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    this->stat_turned_on_sub_blocks_per_access[this->metadata_sets[index].ways[way].active_sub_blocks]++;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].real_access_counter_read[i] += package->sub_blocks[i];
    }

    if (package->memory_operation == MEMORY_OPERATION_WRITEBACK ||
    package->memory_operation == MEMORY_OPERATION_WRITE) {
        this->metadata_sets[index].ways[way].is_dirty = true;
    }

    /// ================================================================
    /// METADATA Learn Mode
    /// ================================================================
    if (this->metadata_sets[index].ways[way].learn_mode == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE ON\n");

        // pht UPDATE
        if (this->metadata_sets[index].ways[way].pht_pointer != NULL &&
        this->metadata_sets[index].ways[way].pht_pointer->pointer == true) {
            // pht Statistics
            this->add_stat_pht_access();
            this->add_stat_pht_hit();

            // Update the pht entry
            this->metadata_sets[index].ways[way].pht_pointer->last_access = sinuca_engine.get_global_cycle();

            // Check the pht Overflow
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] += package->sub_blocks[i];

                if (this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] > this->access_counter_max_read) {
                    this->metadata_sets[index].ways[way].pht_pointer->overflow_read[i] = true;
                    this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] = this->access_counter_max_read;
                }
            }
        }
    }
    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE OFF\n");

        bool all_off = true;
        bool all_dead = true;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            //Predicted as DEAD-READ
            if (this->metadata_sets[index].ways[way].overflow_read[i] == false &&
            this->metadata_sets[index].ways[way].real_access_counter_read[i] == this->metadata_sets[index].ways[way].access_counter_read[i]) {
                // Can Turn off the sub_blocks
                if (this->metadata_sets[index].ways[way].is_dirty == false &&
                this->turnoff_dead_lines == true) {
                    this->metadata_sets[index].ways[way].active_sub_blocks -= this->metadata_sets[index].ways[way].sub_blocks[i];
                    this->metadata_sets[index].ways[way].sub_blocks[i] = false;
                }
                else {
                    all_off = false;
                }
            }
            else {
                all_off = false;
                all_dead = false;
            }
        }
        if (all_off) {
            this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
        }
        if (all_dead) {
            this->metadata_sets[index].ways[way].is_dead_read = true;
        }
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
};



/// ============================================================================
void line_usage_predictor_dsbp_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE, "Metadata Line should be clean\n");

    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_line_miss();

    /// Statistics
    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        this->cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    pht_line_t *pht_line = this->pht_find_line(package->opcode_address, package->memory_address);
    ///=================================================================
    /// pht HIT
    ///=================================================================
    if (pht_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t %s", pht_line->content_to_string().c_str())
        // Statistics
        this->add_stat_pht_access();
        this->add_stat_pht_hit();

        // Clean the entries
        this->metadata_sets[index].ways[way].clean();

        pht_line->last_access = sinuca_engine.get_global_cycle();

        // If no pht_pointer
        if (pht_line->pointer == false) {
            pht_line->pointer = true;
            this->metadata_sets[index].ways[way].pht_pointer = pht_line;
        }
        else {
            this->metadata_sets[index].ways[way].pht_pointer = NULL;
        }

        // Copy pht prediction
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;

        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->metadata_sets[index].ways[way].access_counter_read[i] = pht_line->access_counter_read[i];
            this->metadata_sets[index].ways[way].overflow_read[i] = pht_line->overflow_read[i];
            this->metadata_sets[index].ways[way].sub_blocks[i] = pht_line->access_counter_read[i] != 0;
            this->metadata_sets[index].ways[way].active_sub_blocks += pht_line->access_counter_read[i] != 0;
        }

        this->metadata_sets[index].ways[way].is_dead_read = false;
    }
    ///=================================================================
    /// pht MISS
    ///=================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht MISS\n")
        // Statistics
        this->add_stat_pht_access();
        this->add_stat_pht_miss();

        // New pht entry
        pht_line = pht_evict_address(package->opcode_address, package->memory_address);

        // Clean the entries
        pht_line->clean();
        this->metadata_sets[index].ways[way].clean();

        pht_line->opcode_address = package->opcode_address;
        pht_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
        pht_line->last_access = sinuca_engine.get_global_cycle();

        pht_line->pointer = true;
        this->metadata_sets[index].ways[way].pht_pointer = pht_line;

        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_LEARN;
        this->metadata_sets[index].ways[way].learn_mode = true;

        this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->metadata_sets[index].ways[way].sub_blocks[i] = true;
        }
    }
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_dsbp_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].active_sub_blocks <= sinuca_engine.get_global_line_size(),
                        "Wrong number of active_sub_blocks %d", this->metadata_sets[index].ways[way].active_sub_blocks);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_sub_block_miss();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            this->cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                    sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }


    // Enable Learn_mode
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRONG_FIRST;

    this->metadata_sets[index].ways[way].learn_mode = true;
    this->metadata_sets[index].ways[way].is_dead_read = false;
    this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].sub_blocks[i] = true;

        this->metadata_sets[index].ways[way].access_counter_read[i] = 0;
        this->metadata_sets[index].ways[way].overflow_read[i] = false;
    }


    ///=================================================================
    /// pht HIT
    ///=================================================================
    if (this->metadata_sets[index].ways[way].pht_pointer != NULL &&
    this->metadata_sets[index].ways[way].pht_pointer->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].pht_pointer->content_to_string().c_str())
        // Statistics
        this->add_stat_pht_access();
        this->add_stat_pht_hit();

        // Update the pht entry
        this->metadata_sets[index].ways[way].pht_pointer->last_access = sinuca_engine.get_global_cycle();

        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_access_counter_read[i] <= this->access_counter_max_read) {
                this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] = this->metadata_sets[index].ways[way].real_access_counter_read[i];
                this->metadata_sets[index].ways[way].pht_pointer->overflow_read[i] = false;
            }
            else {
                this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] = this->access_counter_max_read;
                this->metadata_sets[index].ways[way].pht_pointer->overflow_read[i] = true;
            }
        }
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_writeback() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_DISABLE, "Metadata Line should be enabled\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].is_dirty == true, "Metadata Line should be dirty\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].active_sub_blocks <= sinuca_engine.get_global_line_size() &&
                        this->metadata_sets[index].ways[way].active_sub_blocks  > 0,
                        "Wrong number of active_sub_blocks %d", this->metadata_sets[index].ways[way].active_sub_blocks);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_send_writeback();

    /// Statistics
    this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    this->metadata_sets[index].ways[way].is_dirty = false;

    if (this->metadata_sets[index].ways[way].learn_mode == false) {
        bool all_off = true;
        bool all_dead = true;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            //Predicted as DEAD-READ
            if (this->metadata_sets[index].ways[way].overflow_read[i] == false &&
            this->metadata_sets[index].ways[way].real_access_counter_read[i] == this->metadata_sets[index].ways[way].access_counter_read[i]) {
                // Can Turn off the sub_blocks
                if (this->metadata_sets[index].ways[way].is_dirty == false &&
                this->turnoff_dead_lines == true) {
                    this->metadata_sets[index].ways[way].active_sub_blocks -= this->metadata_sets[index].ways[way].sub_blocks[i];
                    this->metadata_sets[index].ways[way].sub_blocks[i] = false;
                }
                else {
                    all_off = false;
                }
            }
            else {
                all_off = false;
                all_dead = false;
            }
        }
        if (all_off) {
            this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
        }
        if (all_dead) {
            this->metadata_sets[index].ways[way].is_dead_read = true;
        }
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_writeback() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    // Can be a higher level cache-to-cache ???????????????????????
    // ~ ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE, "Metadata Line should be invalid\n");

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_recv_writeback();

    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    if (this->metadata_sets[index].ways[way].learn_mode == true) {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_LEARN;
    }
    else {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
    }

    /// Activate the received sub-blocks
    this->metadata_sets[index].ways[way].active_sub_blocks = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].sub_blocks[i] = true;
        }

        this->metadata_sets[index].ways[way].active_sub_blocks += (this->metadata_sets[index].ways[way].sub_blocks[i] == true);
    }


    this->metadata_sets[index].ways[way].is_dirty = true;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Recv %s", this->metadata_sets[index].ways[way].content_to_string().c_str())

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].active_sub_blocks <= sinuca_engine.get_global_line_size(),
                        "Wrong number of active_sub_blocks %d", this->metadata_sets[index].ways[way].active_sub_blocks);

    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_eviction();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                    sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }


    uint32_t touched_sub_blocks = sinuca_engine.get_global_line_size();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        // Reads before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter_read[i] == 0) {
            this->add_stat_line_read_0();
            touched_sub_blocks--;
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] == 1) {
            this->add_stat_line_read_1();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] >= 2 &&
                this->metadata_sets[index].ways[way].real_access_counter_read[i] <= 3) {
            this->add_stat_line_read_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] >= 4 &&
                this->metadata_sets[index].ways[way].real_access_counter_read[i] <= 7) {
            this->add_stat_line_read_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] >= 8 &&
                this->metadata_sets[index].ways[way].real_access_counter_read[i] <= 15) {
            this->add_stat_line_read_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] >= 16 &&
                this->metadata_sets[index].ways[way].real_access_counter_read[i] <= 127) {
            this->add_stat_line_read_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read[i] >=128) {
            this->add_stat_line_read_128_bigger();
        }
    }

    this->stat_touched_sub_blocks_per_line[touched_sub_blocks]++;

    //==================================================================
    // Prediction Accuracy Statistics
    bool overflow_flag = false;

    switch (this->metadata_sets[index].ways[way].line_status) {
        case LINE_SUB_BLOCK_LEARN:
            this->add_stat_dead_read_learn();
        break;

        case LINE_SUB_BLOCK_NORMAL:
            if (this->metadata_sets[index].ways[way].is_dirty) {
                overflow_flag = true;
            }
            else {
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->metadata_sets[index].ways[way].overflow_read[i]) {
                        overflow_flag = true;
                        break;
                    }
                }
            }

            if (overflow_flag) {
                this->add_stat_dead_read_normal_correct();
            }
            else {
                this->add_stat_dead_read_normal_over();
            }
        break;

        case LINE_SUB_BLOCK_DISABLE:
            this->add_stat_dead_read_disable_correct();
        break;

        case LINE_SUB_BLOCK_WRONG_FIRST:
            this->add_stat_dead_read_disable_under();
        break;

        case LINE_SUB_BLOCK_WRITEBACK:
            ERROR_PRINTF("Should not be in LINE_SUB_BLOCK_WRITEBACK status\n")
        break;
    }

    ///=================================================================
    /// pht HIT
    ///=================================================================
    if (//this->metadata_sets[index].ways[way].learn_mode == false &&
    this->metadata_sets[index].ways[way].pht_pointer != NULL &&
    this->metadata_sets[index].ways[way].pht_pointer->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].pht_pointer->content_to_string().c_str())
        // Statistics
        this->add_stat_pht_access();
        this->add_stat_pht_hit();

        // Update the pht entry
        this->metadata_sets[index].ways[way].pht_pointer->last_access = sinuca_engine.get_global_cycle();

        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_access_counter_read[i] <= this->access_counter_max_read) {
                this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] = this->metadata_sets[index].ways[way].real_access_counter_read[i];
                this->metadata_sets[index].ways[way].pht_pointer->overflow_read[i] = false;
            }
            else {
                this->metadata_sets[index].ways[way].pht_pointer->access_counter_read[i] = this->access_counter_max_read;
                this->metadata_sets[index].ways[way].pht_pointer->overflow_read[i] = true;
            }
        }

        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", this->metadata_sets[index].ways[way].pht_pointer->content_to_string().c_str())
        this->metadata_sets[index].ways[way].pht_pointer->pointer = false;
        this->metadata_sets[index].ways[way].pht_pointer = NULL;
    }

    this->metadata_sets[index].ways[way].clean();
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].active_sub_blocks <= sinuca_engine.get_global_line_size(),
                        "Wrong number of active_sub_blocks %d", this->metadata_sets[index].ways[way].active_sub_blocks);
    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_invalidation();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                    sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
/// pht Miss
/// ============================================================================
pht_line_t* line_usage_predictor_dsbp_t::pht_find_line(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("pht_find_line()\n")
    uint32_t pht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t pht_index = opcode_address & this->pht_index_bits_mask;

    ERROR_ASSERT_PRINTF(pht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", pht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(pht_index < this->pht_total_sets, "Wrong index %d > total_sets %d", pht_index, this->pht_total_sets);

    for (uint32_t pht_way = 0; pht_way < this->get_pht_associativity(); pht_way++) {
        if (this->pht_sets[pht_index].ways[pht_way].opcode_address == opcode_address && this->pht_sets[pht_index].ways[pht_way].offset == pht_offset) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Found pht Index %u - Way %u\n", pht_index, pht_way )
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht %s \n", this->pht_sets[pht_index].ways[pht_way].content_to_string().c_str())
            return &this->pht_sets[pht_index].ways[pht_way];
        }
    }
    return NULL;
}

/// ============================================================================
pht_line_t* line_usage_predictor_dsbp_t::pht_evict_address(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("pht_evict_address()\n")
    uint32_t pht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t pht_index = opcode_address & this->pht_index_bits_mask;

    ERROR_ASSERT_PRINTF(pht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", pht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(pht_index < this->pht_total_sets, "Wrong index %d > total_sets %d", pht_index, this->pht_total_sets);

    pht_line_t *choosen_line = NULL;

    switch (this->pht_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t pht_way = 0; pht_way < this->get_pht_associativity(); pht_way++) {
                /// If the line is LRU
                if (this->pht_sets[pht_index].ways[pht_way].last_access <= last_access) {
                    choosen_line = &this->pht_sets[pht_index].ways[pht_way];
                    last_access = this->pht_sets[pht_index].ways[pht_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t pht_way = (rand_r(&seed) % this->get_pht_associativity());
            choosen_line = &this->pht_sets[pht_index].ways[pht_way];
        }
        break;

        case REPLACEMENT_INVALID_OR_LRU:
        case REPLACEMENT_DEAD_OR_LRU:
        case REPLACEMENT_FIFO:
        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: %s not implemented.\n",  get_enum_replacement_char(this->pht_replacement_policy));
        break;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht %s \n", choosen_line->content_to_string().c_str())
    return choosen_line;
};


/// ============================================================================
void line_usage_predictor_dsbp_t::print_structures() {
    line_usage_predictor_t::print_structures();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_dsbp_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_dsbp_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_writeback = 0;
    this->stat_recv_writeback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    this->stat_pht_access = 0;
    this->stat_pht_hit = 0;
    this->stat_pht_miss = 0;

    /// Prediction accuracy
    this->stat_dead_read_learn = 0;
    this->stat_dead_read_normal_over = 0;
    this->stat_dead_read_normal_correct = 0;
    this->stat_dead_read_disable_correct = 0;
    this->stat_dead_read_disable_under = 0;

    this->stat_line_read_0 = 0;
    this->stat_line_read_1 = 0;
    this->stat_line_read_2_3 = 0;
    this->stat_line_read_4_7 = 0;
    this->stat_line_read_8_15 = 0;
    this->stat_line_read_16_127 = 0;
    this->stat_line_read_128_bigger = 0;


    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        this->cycles_turned_on_whole_line[i] = 0;
        this->stat_touched_sub_blocks_per_line[i] = 0;
        this->stat_turned_on_sub_blocks_per_access[i] = 0;
    }
    this->cycles_turned_off_whole_line = 0;
    this->cycles_turned_off_whole_line_since_begin = 0;
};

/// ============================================================================
void line_usage_predictor_dsbp_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    for (uint32_t index = 0; index < this->get_metadata_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_metadata_associativity(); way++) {
            this->line_eviction(NULL, NULL, index, way);
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_hit", stat_line_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_send_writeback", stat_send_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_recv_writeback", stat_recv_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_pht_access", stat_pht_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_pht_hit", stat_pht_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_pht_miss", stat_pht_miss);

    /// prediction accuracy
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_learn", stat_dead_read_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_over", stat_dead_read_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_correct", stat_dead_read_normal_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_correct", stat_dead_read_disable_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_under", stat_dead_read_disable_under);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_0", stat_line_read_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_1", stat_line_read_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_2_3", stat_line_read_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_4_7", stat_line_read_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_8_15", stat_line_read_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_16_127", stat_line_read_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_128_bigger", stat_line_read_128_bigger);

    uint64_t total_cycles_turned_on_whole_line = 0;
    uint64_t total_cycles_turned_off_whole_line = 0;

    char name[100];
    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_touched_sub_blocks_per_line_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_touched_sub_blocks_per_line[i]);
    }

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_turned_on_sub_blocks_per_access_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_turned_on_sub_blocks_per_access[i]);
    }

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "cycles_turned_on_whole_line_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, cycles_turned_on_whole_line[i]);
        total_cycles_turned_on_whole_line += cycles_turned_on_whole_line[i];
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_line", cycles_turned_off_whole_line);
    total_cycles_turned_off_whole_line = cycles_turned_off_whole_line;

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_on_whole_cache",
                                                                                    total_cycles_turned_on_whole_line/ metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_cache",
                                                                                    total_cycles_turned_off_whole_line/ metadata_line_number);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_line_since_begin", cycles_turned_off_whole_line_since_begin);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_since_begin_whole_cache", cycles_turned_off_whole_line_since_begin / metadata_line_number);

    if ((total_cycles_turned_on_whole_line + total_cycles_turned_off_whole_line) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }

};

/// ============================================================================
void line_usage_predictor_dsbp_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bytes_per_subblock", bytes_per_subblock);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_bits_read", access_counter_bits_read);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_max_read", access_counter_max_read);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_eviction", early_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_writeback", early_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "turnoff_dead_lines", turnoff_dead_lines);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// pht
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "pht_line_number", pht_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "pht_associativity", pht_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "pht_total_sets", pht_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "pht_replacement_policy", get_enum_replacement_char(pht_replacement_policy));
};
