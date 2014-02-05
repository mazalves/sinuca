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
line_usage_predictor_dsbp_oracle_t::line_usage_predictor_dsbp_oracle_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DSBP_ORACLE;

    this->bytes_per_subblock = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;
};

/// ============================================================================
line_usage_predictor_dsbp_oracle_t::~line_usage_predictor_dsbp_oracle_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dsbp_metadata_set_t>(metadata_sets);

    utils_t::template_delete_array<uint64_t>(cycles_turned_on_whole_line);
    utils_t::template_delete_array<uint64_t>(stat_touched_sub_blocks_per_line);
    utils_t::template_delete_array<uint64_t>(stat_turned_on_sub_blocks_per_access);
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::allocate() {
    line_usage_predictor_t::allocate();

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

            this->metadata_sets[i].ways[j].clock_last_access_subblock = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
        }
    }

    /// STATISTICS
    this->cycles_turned_on_whole_line = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_touched_sub_blocks_per_line = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_turned_on_sub_blocks_per_access = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");

};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::fill_package_sub_blocks(memory_package_t *package) {
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

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Package %s", package->sub_blocks_to_string().c_str())

};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;

    // If writeback send the whole line, otherwise request only what the package needs (don't change the package).
    if (package->memory_operation == MEMORY_OPERATION_WRITEBACK) {
        package->memory_size = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            package->sub_blocks[i] = (this->metadata_sets[index].ways[way].sub_blocks[i] ||
                                    (this->metadata_sets[index].ways[way].access_counter_read[i] != 0));

            package->memory_size += package->sub_blocks[i];
        }
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Package AFTER line_to_subblock %s\n", package->content_to_string().c_str())
    }
};

/// ============================================================================
bool line_usage_predictor_dsbp_oracle_t::check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    if (!this->bring_whole_line) {
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (package->sub_blocks[i] == true &&
            this->metadata_sets[index].ways[way].sub_blocks[i] == false) {
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
                return false;
            }
        }
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
    return true;

};

/// ============================================================================
bool line_usage_predictor_dsbp_oracle_t::check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_disabled()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
bool line_usage_predictor_dsbp_oracle_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
bool line_usage_predictor_dsbp_oracle_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_write()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};


/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_line_hit();

    /// Statistics
    this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].real_access_counter_read[i]++ ;
            this->metadata_sets[index].ways[way].clock_last_access_subblock[i] = sinuca_engine.get_global_cycle();

        }
    }
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();

    if (package->memory_operation == MEMORY_OPERATION_WRITEBACK ||
    package->memory_operation == MEMORY_OPERATION_WRITE) {
        this->metadata_sets[index].ways[way].is_dirty = true;
    }

    this->metadata_sets[index].ways[way].cycle_access_list.push_back(sinuca_engine.get_global_cycle());


    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
};


/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_line_miss();

    /// Statistics
    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    // Activate the sub-blocks requested.
    this->metadata_sets[index].ways[way].active_sub_blocks = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].sub_blocks[i] = true;
        }
        this->metadata_sets[index].ways[way].active_sub_blocks += this->metadata_sets[index].ways[way].sub_blocks[i];
    }


    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
};


/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_sub_block_miss();

    /// Statistics
    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    // Activate the sub-blocks requested.
    this->metadata_sets[index].ways[way].active_sub_blocks = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].sub_blocks[i] = true;
        }
        this->metadata_sets[index].ways[way].active_sub_blocks += this->metadata_sets[index].ways[way].sub_blocks[i];
    }


    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_send_writeback();

    /// Statistics
    this->cycles_turned_on_whole_line[this->metadata_sets[index].ways[way].active_sub_blocks] +=
                sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].real_access_counter_read[i]++ ;
            this->metadata_sets[index].ways[way].clock_last_access_subblock[i] = sinuca_engine.get_global_cycle();
        }
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_recv_writeback();

    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    this->metadata_sets[index].ways[way].is_dirty = true;
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;

    // Activate the sub-blocks requested.
    this->metadata_sets[index].ways[way].active_sub_blocks = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i]) {
            this->metadata_sets[index].ways[way].sub_blocks[i] = true;
            this->metadata_sets[index].ways[way].real_access_counter_read[i]++ ;
            this->metadata_sets[index].ways[way].clock_last_access_subblock[i] = sinuca_engine.get_global_cycle();
        }
        this->metadata_sets[index].ways[way].active_sub_blocks += this->metadata_sets[index].ways[way].sub_blocks[i];
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};



/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_eviction();

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

    // Compute the number of sub-blocks active in the moment of each access
    uint32_t active_blocks;
    while( !this->metadata_sets[index].ways[way].cycle_access_list.empty() ){
        active_blocks = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_access_counter_read[i] > 0 &&
            this->metadata_sets[index].ways[way].clock_last_access_subblock[i] >= this->metadata_sets[index].ways[way].cycle_access_list.back() ) {
                active_blocks++;
            }
        }
        this->stat_turned_on_sub_blocks_per_access[active_blocks]++;
        this->metadata_sets[index].ways[way].cycle_access_list.pop_back();
    }

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

    this->metadata_sets[index].ways[way].clean();

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].clock_last_access_subblock[i] = sinuca_engine.get_global_cycle();
    }
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

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

    // Compute the number of sub-blocks active in the moment of each access
    uint32_t active_blocks;
    while( !this->metadata_sets[index].ways[way].cycle_access_list.empty() ){
        active_blocks = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_access_counter_read[i] > 0 &&
            this->metadata_sets[index].ways[way].clock_last_access_subblock[i] >= this->metadata_sets[index].ways[way].cycle_access_list.back() ) {
                active_blocks++;
            }
        }
        this->stat_turned_on_sub_blocks_per_access[active_blocks]++;
        this->metadata_sets[index].ways[way].cycle_access_list.pop_back();
    }

    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    this->metadata_sets[index].ways[way].is_dirty = false;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].clock_last_access_subblock[i] = sinuca_engine.get_global_cycle();
    }
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_writeback = 0;
    this->stat_recv_writeback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

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
void line_usage_predictor_dsbp_oracle_t::print_statistics() {
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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_since_begin_whole_cache", cycles_turned_off_whole_line_since_begin/ metadata_line_number);

    if ((total_cycles_turned_on_whole_line + total_cycles_turned_off_whole_line) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }
};

/// ============================================================================
void line_usage_predictor_dsbp_oracle_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bytes_per_subblock", bytes_per_subblock);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);
};

