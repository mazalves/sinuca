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
line_usage_predictor_subblock_stats_t::line_usage_predictor_subblock_stats_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_SUBBLOCK_STATS;

    this->sub_block_size = 0;
    this->sub_block_total = 0;

    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;

    /// STATISTICS
    this->stat_accessed_sub_blocks = NULL;     /// Number of sub_blocks accessed
    this->stat_real_write_counter = NULL;     /// Number of sub_blocks written
};

/// ============================================================================
line_usage_predictor_subblock_stats_t::~line_usage_predictor_subblock_stats_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dsbp_metadata_set_t>(metadata_sets);

    utils_t::template_delete_array<uint64_t>(stat_accessed_sub_blocks);
    utils_t::template_delete_array<uint64_t>(stat_real_write_counter);
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::allocate() {
    line_usage_predictor_t::allocate();

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size() / this->get_sub_block_size()),
                        "Wrong line_size(%u) or sub_block_size(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_sub_block_total(sinuca_engine.get_global_line_size() / this->get_sub_block_size());
    
    // Cache Metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()),
                        "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n",
                                        this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(),
                                        this->get_metadata_total_sets(), this->get_sub_block_total());

    this->metadata_sets = utils_t::template_allocate_array<dsbp_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dsbp_metadata_line_t>(this->get_metadata_associativity());

        for (uint32_t j = 0; j < this->get_metadata_associativity(); j++) {
            this->metadata_sets[i].ways[j].valid_sub_blocks = utils_t::template_allocate_initialize_array<line_sub_block_t>(sinuca_engine.get_global_line_size(), LINE_SUB_BLOCK_DISABLE);
            this->metadata_sets[i].ways[j].real_access_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].access_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);

            this->metadata_sets[i].ways[j].clock_become_alive = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].clock_become_dead = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->metadata_sets[i].ways[j].real_write_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->metadata_sets[i].ways[j].active_sub_blocks = 0;
            this->metadata_sets[i].ways[j].is_dead = true;
        }
    }

    /// ================================================================
    /// Statistics
    /// ================================================================
    this->stat_accessed_sub_blocks = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_real_write_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");

};

/// ============================================================================
// Input:   base_address, size
// Output:  start_sub_block, end_Sub_block
void line_usage_predictor_subblock_stats_t::get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end) {
    ERROR_ASSERT_PRINTF(size > 0, "Received a request with invalid size.\n")
    uint64_t address_offset = base_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t address_size = address_offset + size;
    if (address_size >= sinuca_engine.get_global_line_size()){
        address_size = sinuca_engine.get_global_line_size();
    }

    sub_block_ini = uint32_t(address_offset / this->sub_block_size) * this->sub_block_size;
    sub_block_end = uint32_t( (address_size / this->sub_block_size) +
                                        uint32_t((address_size % this->sub_block_size) != 0) ) * this->sub_block_size;
}

/// ============================================================================
void line_usage_predictor_subblock_stats_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    /// Compute the START and END sub_blocks
    uint32_t sub_block_ini, sub_block_end;
    this->get_start_end_sub_blocks(package->memory_address, package->memory_size, sub_block_ini, sub_block_end);

    /// Generates the Request Vector
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        package->sub_blocks[i] = ( i >= sub_block_ini && i < sub_block_end );
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t package->sub_blocks:%s", package->sub_blocks_to_string().c_str())
};

/// ============================================================================
bool line_usage_predictor_subblock_stats_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t HIT\n")
    return true;
};

/// ============================================================================
bool line_usage_predictor_subblock_stats_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    (void)index;
    (void)way;

    return false;
};


/// ============================================================================
// Mechanism Operations
/// ============================================================================
void line_usage_predictor_subblock_stats_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_hit();

    // Update the METADATA real_access_counter
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].real_access_counter[i] += package->sub_blocks[i];
        if (package->sub_blocks[i] == true) {
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        }
    }
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_subblock_stats_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_miss();

    (void)package;
    // Clean the metadata entry
    this->metadata_sets[index].ways[way].learn_mode = true;
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].pht_pointer = NULL;
    this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].access_counter[i] = 0;
        this->metadata_sets[index].ways[way].overflow[i] = true;

        this->metadata_sets[index].ways[way].real_access_counter[i] = 0;
        this->metadata_sets[index].ways[way].real_write_counter[i] = 0;

        this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
    }

    // Modify the package->sub_blocks (next level request)
    // ~ package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_subblock_stats_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_sub_block_miss();

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_subblock_stats_t::line_recv_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_recv_copyback();

    // Clean the metadata entry
    this->metadata_sets[index].ways[way].clean();
    this->metadata_sets[index].ways[way].is_dirty = true;
    this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->metadata_sets[index].ways[way].real_write_counter[i] += package->sub_blocks[i];
        if (package->sub_blocks[i] == true) {
            this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        }
    }
};


/// ============================================================================
void line_usage_predictor_subblock_stats_t::line_send_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_send_copyback();

    package->memory_size = sinuca_engine.get_global_line_size();

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_eviction();

    //==================================================================
    // Statistics
    uint32_t sub_blocks_accessed = 0;
    uint32_t sub_blocks_written = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {

        // Accesses before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter[i] == 0) {
            this->add_stat_sub_block_access_0();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] == 1) {
            this->add_stat_sub_block_access_1();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 2 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 3) {
            this->add_stat_sub_block_access_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 4 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 7) {
            this->add_stat_sub_block_access_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 8 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 15) {
            this->add_stat_sub_block_access_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 16 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 127) {
            this->add_stat_sub_block_access_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >=128) {
            this->add_stat_sub_block_access_128_bigger();
        }

        // Writes before eviction
        if (this->metadata_sets[index].ways[way].real_write_counter[i] == 0) {
            this->add_stat_sub_block_write_0();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] == 1) {
            this->add_stat_sub_block_write_1();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 2 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 3) {
            this->add_stat_sub_block_write_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 4 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 7) {
            this->add_stat_sub_block_write_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 8 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 15) {
            this->add_stat_sub_block_write_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 16 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 127) {
            this->add_stat_sub_block_write_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >=128) {
            this->add_stat_sub_block_write_128_bigger();
        }

        // Sub-Blocks accessed before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter[i] != 0) {
            sub_blocks_accessed++;
        }

        // Written Sub-blocks before eviction
        if (this->metadata_sets[index].ways[way].real_write_counter[i] != 0) {
            sub_blocks_written++;
        }
    }
    this->stat_accessed_sub_blocks[sub_blocks_accessed]++;
    this->stat_real_write_counter[sub_blocks_written]++;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Stats %u %"PRIu64"\n", sub_blocks_accessed, this->stat_accessed_sub_block[sub_blocks_accessed] )
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_invalidation();

    //==================================================================
    // Statistics
    uint32_t sub_blocks_accessed = 0;
    uint32_t sub_blocks_written = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {

        // Accesses before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter[i] == 0) {
            this->add_stat_sub_block_access_0();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] == 1) {
            this->add_stat_sub_block_access_1();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 2 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 3) {
            this->add_stat_sub_block_access_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 4 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 7) {
            this->add_stat_sub_block_access_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 8 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 15) {
            this->add_stat_sub_block_access_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >= 16 && this->metadata_sets[index].ways[way].real_access_counter[i] <= 127) {
            this->add_stat_sub_block_access_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter[i] >=128) {
            this->add_stat_sub_block_access_128_bigger();
        }

        // Writes before eviction
        if (this->metadata_sets[index].ways[way].real_write_counter[i] == 0) {
            this->add_stat_sub_block_write_0();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] == 1) {
            this->add_stat_sub_block_write_1();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 2 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 3) {
            this->add_stat_sub_block_write_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 4 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 7) {
            this->add_stat_sub_block_write_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 8 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 15) {
            this->add_stat_sub_block_write_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >= 16 && this->metadata_sets[index].ways[way].real_write_counter[i] <= 127) {
            this->add_stat_sub_block_write_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_write_counter[i] >=128) {
            this->add_stat_sub_block_write_128_bigger();
        }

        // Sub-Blocks accessed before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter[i] != 0) {
            sub_blocks_accessed++;
        }

        // Written Sub-blocks before eviction
        if (this->metadata_sets[index].ways[way].real_write_counter[i] != 0) {
            sub_blocks_written++;
        }
    }
    this->stat_accessed_sub_blocks[sub_blocks_accessed]++;
    this->stat_real_write_counter[sub_blocks_written]++;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Stats %u %"PRIu64"\n", sub_blocks_accessed, this->stat_accessed_sub_block[sub_blocks_accessed] )
};



/// ============================================================================
void line_usage_predictor_subblock_stats_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_subblock_stats_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_copyback = 0;
    this->stat_recv_copyback = 0;    
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    this->stat_sub_block_access_0 = 0;
    this->stat_sub_block_access_1 = 0;
    this->stat_sub_block_access_2_3 = 0;
    this->stat_sub_block_access_4_7 = 0;
    this->stat_sub_block_access_8_15 = 0;
    this->stat_sub_block_access_16_127 = 0;
    this->stat_sub_block_access_128_bigger = 0;

    this->stat_sub_block_write_0 = 0;
    this->stat_sub_block_write_1 = 0;
    this->stat_sub_block_write_2_3 = 0;
    this->stat_sub_block_write_4_7 = 0;
    this->stat_sub_block_write_8_15 = 0;
    this->stat_sub_block_write_16_127 = 0;
    this->stat_sub_block_write_128_bigger = 0;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        this->stat_accessed_sub_blocks[i] = 0;
        this->stat_real_write_counter[i] = 0;
    }
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    for (uint32_t index = 0; index < this->get_metadata_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_metadata_associativity(); way++) {
            this->line_eviction(index, way);
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_hit", stat_line_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_send_copyback", stat_send_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_recv_copyback", stat_recv_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_0", stat_sub_block_access_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_1", stat_sub_block_access_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_2_3", stat_sub_block_access_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_4_7", stat_sub_block_access_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_8_15", stat_sub_block_access_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_16_127", stat_sub_block_access_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_access_128_bigger", stat_sub_block_access_128_bigger);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_0", stat_sub_block_write_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_1", stat_sub_block_write_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_2_3", stat_sub_block_write_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_4_7", stat_sub_block_write_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_8_15", stat_sub_block_write_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_16_127", stat_sub_block_write_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_write_128_bigger", stat_sub_block_write_128_bigger);

    char name[100];
    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_accessed_sub_blocks_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_accessed_sub_blocks[i]);
    }

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_real_write_counter_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_real_write_counter[i]);
    }
};

/// ============================================================================
void line_usage_predictor_subblock_stats_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "sub_block_size", sub_block_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "sub_block_total", sub_block_total);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);
};

