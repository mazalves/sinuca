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

    this->sub_block_size = 0;
    this->sub_block_total = 0;

    this->usage_counter_bits = 0;
    this->usage_counter_max = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;


    /// pht
    this->dsbp_pht_sets = NULL;
    this->dsbp_pht_line_number = 0;
    this->dsbp_pht_associativity = 0;
    this->dsbp_pht_total_sets = 0;
    this->dsbp_pht_replacement_policy = REPLACEMENT_LRU;

    /// STATISTICS
    this->stat_accessed_sub_block = NULL;
    this->stat_active_sub_block_per_access = NULL;
    this->stat_active_sub_block_per_cycle = NULL;
    this->stat_written_sub_blocks_per_line = NULL;
};

/// ============================================================================
line_usage_predictor_dsbp_t::~line_usage_predictor_dsbp_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dsbp_metadata_set_t>(metadata_sets);
    utils_t::template_delete_array<pht_set_t>(dsbp_pht_sets);

    utils_t::template_delete_array<uint64_t>(stat_accessed_sub_block);
    utils_t::template_delete_array<uint64_t>(stat_active_sub_block_per_access);
    utils_t::template_delete_array<uint64_t>(stat_active_sub_block_per_cycle);
    utils_t::template_delete_array<uint64_t>(stat_written_sub_blocks_per_line);
};

/// ============================================================================
void line_usage_predictor_dsbp_t::allocate() {
    line_usage_predictor_t::allocate();

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size() / this->get_sub_block_size()), "Wrong line_size(%u) or sub_block_size(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_sub_block_total(sinuca_engine.get_global_line_size() / this->get_sub_block_size());

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    this->usage_counter_max = pow(2, this->usage_counter_bits) - 1;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s dsbp %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets(), this->get_sub_block_total());
    this->metadata_sets = utils_t::template_allocate_array<dsbp_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dsbp_metadata_line_t>(this->get_metadata_associativity());

        for (uint32_t j = 0; j < this->get_metadata_associativity(); j++) {
            this->metadata_sets[i].ways[j].valid_sub_blocks = utils_t::template_allocate_initialize_array<line_sub_block_t>(sinuca_engine.get_global_line_size(), LINE_SUB_BLOCK_DISABLE);
            this->metadata_sets[i].ways[j].real_usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);

            this->metadata_sets[i].ways[j].clock_become_alive = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->metadata_sets[i].ways[j].clock_become_dead = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->metadata_sets[i].ways[j].written_sub_blocks = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->metadata_sets[i].ways[j].active_sub_blocks = 0;
            this->metadata_sets[i].ways[j].is_dead = true;
        }
    }

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_dsbp_pht_line_number() / this->get_dsbp_pht_associativity()), "Wrong line number(%u) or associativity(%u).\n", this->get_dsbp_pht_line_number(), this->get_dsbp_pht_associativity());
    this->set_dsbp_pht_total_sets(this->get_dsbp_pht_line_number() / this->get_dsbp_pht_associativity());
    this->dsbp_pht_sets = utils_t::template_allocate_array<pht_set_t>(this->get_dsbp_pht_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s dsbp_pht %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_dsbp_pht_line_number(), this->get_dsbp_pht_associativity(), this->get_dsbp_pht_total_sets(), this->get_sub_block_total());

    /// INDEX MASK
    this->dsbp_pht_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_dsbp_pht_total_sets()); i++) {
        this->dsbp_pht_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_dsbp_pht_total_sets(); i++) {
        this->dsbp_pht_sets[i].ways = utils_t::template_allocate_array<pht_line_t>(this->get_dsbp_pht_associativity());

        for (uint32_t j = 0; j < this->get_dsbp_pht_associativity(); j++) {
            this->dsbp_pht_sets[i].ways[j].opcode_address = 0;
            this->dsbp_pht_sets[i].ways[j].offset = 0;
            this->dsbp_pht_sets[i].ways[j].pointer = false;

            this->dsbp_pht_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->dsbp_pht_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
        }
    }

    /// ================================================================
    /// Statistics
    /// ================================================================
    this->stat_accessed_sub_block = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_active_sub_block_per_access = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_active_sub_block_per_cycle = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
    this->stat_written_sub_blocks_per_line = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);

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

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
// STATISTICS
/// ============================================================================
void line_usage_predictor_dsbp_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_dsbp_line_sub_block_disable_always = 0;
    this->stat_dsbp_line_sub_block_disable_turnoff = 0;
    this->stat_dsbp_line_sub_block_normal_correct = 0;
    this->stat_dsbp_line_sub_block_normal_over = 0;
    this->stat_dsbp_line_sub_block_learn = 0;
    this->stat_dsbp_line_sub_block_wrong_first = 0;
    this->stat_dsbp_line_sub_block_copyback = 0;

    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_copyback = 0;
    this->stat_eviction = 0;

    this->stat_dsbp_pht_access = 0;
    this->stat_dsbp_pht_hit = 0;
    this->stat_dsbp_pht_miss = 0;

    this->stat_sub_block_touch_0 = 0;
    this->stat_sub_block_touch_1 = 0;
    this->stat_sub_block_touch_2_3 = 0;
    this->stat_sub_block_touch_4_7 = 0;
    this->stat_sub_block_touch_8_15 = 0;
    this->stat_sub_block_touch_16_127 = 0;
    this->stat_sub_block_touch_128_bigger = 0;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        this->stat_accessed_sub_block[i] = 0;
        this->stat_active_sub_block_per_access[i] = 0;
        this->stat_active_sub_block_per_cycle[i] = 0;
        this->stat_written_sub_blocks_per_line[i] = 0;
    }

    /// Number of dirty lines predicted to be dead
    this->stat_dirty_lines_predicted_dead = 0;
    this->stat_clean_lines_predicted_dead = 0;

    this->stat_written_lines_miss_predicted = 0;

    /// Number of times each sub_block was written before eviction
    this->stat_writes_per_sub_blocks_1 = 0;
    this->stat_writes_per_sub_blocks_2 = 0;
    this->stat_writes_per_sub_blocks_3 = 0;
    this->stat_writes_per_sub_blocks_4 = 0;
    this->stat_writes_per_sub_blocks_5 = 0;
    this->stat_writes_per_sub_blocks_10 = 0;
    this->stat_writes_per_sub_blocks_100 = 0;
    this->stat_writes_per_sub_blocks_bigger = 0;
};

/// ============================================================================
void line_usage_predictor_dsbp_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    uint64_t stat_average_dead_cycles = 0;
    for (uint32_t index = 0; index < this->get_metadata_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_metadata_associativity(); way++) {
            this->line_eviction(index, way);
            stat_average_dead_cycles += this->metadata_sets[index].ways[way].stat_total_dead_cycles / sinuca_engine.get_global_line_size();
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_average_dead_cycles", stat_average_dead_cycles);


    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_disable_always", stat_dsbp_line_sub_block_disable_always);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_disable_turnoff", stat_dsbp_line_sub_block_disable_turnoff);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_normal_correct", stat_dsbp_line_sub_block_normal_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_normal_over", stat_dsbp_line_sub_block_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_learn", stat_dsbp_line_sub_block_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_wrong_first", stat_dsbp_line_sub_block_wrong_first);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_line_sub_block_copyback", stat_dsbp_line_sub_block_copyback);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback", stat_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_pht_access", stat_dsbp_pht_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_pht_hit", stat_dsbp_pht_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dsbp_pht_miss", stat_dsbp_pht_miss);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_0", stat_sub_block_touch_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_1", stat_sub_block_touch_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_2_3", stat_sub_block_touch_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_4_7", stat_sub_block_touch_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_8_15", stat_sub_block_touch_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_16_127", stat_sub_block_touch_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_128_bigger", stat_sub_block_touch_128_bigger);

    char name[100];
    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_accessed_sub_block_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_accessed_sub_block[i]);
    }

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_active_sub_block_per_access_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_active_sub_block_per_access[i]);
    }

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_active_sub_block_per_cycle_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_active_sub_block_per_cycle[i]);
    }

    /// Number of dirty lines predicted to be dead
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dirty_lines_predicted_dead", stat_dirty_lines_predicted_dead);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_clean_lines_predicted_dead", stat_clean_lines_predicted_dead);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_written_lines_miss_predicted", stat_written_lines_miss_predicted);

    /// Number of times each sub_block was written before eviction
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_1", stat_writes_per_sub_blocks_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_2", stat_writes_per_sub_blocks_2);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_3", stat_writes_per_sub_blocks_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_4", stat_writes_per_sub_blocks_4);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_5", stat_writes_per_sub_blocks_5);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_10", stat_writes_per_sub_blocks_10);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_100", stat_writes_per_sub_blocks_100);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writes_per_sub_blocks_bigger", stat_writes_per_sub_blocks_bigger);

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_written_sub_blocks_per_line_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_written_sub_blocks_per_line[i]);
    }
};

/// ============================================================================
void line_usage_predictor_dsbp_t::print_configuration() {
    line_usage_predictor_t::print_configuration();


    /// ====================================================================
    /// dsbp
    /// ====================================================================
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "usage_counter_bits", usage_counter_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "usage_counter_max", usage_counter_max);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "sub_block_size", sub_block_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "sub_block_total", sub_block_total);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// pht
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_pht_line_number", dsbp_pht_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_pht_associativity", dsbp_pht_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_pht_total_sets", dsbp_pht_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_pht_replacement_policy", get_enum_replacement_char(dsbp_pht_replacement_policy));
};


/// ============================================================================
// Input:   base_address, size
// Output:  start_sub_block, end_Sub_block
void line_usage_predictor_dsbp_t::get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end) {
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
void line_usage_predictor_dsbp_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    /// Compute the START and END sub_blocks
    uint32_t sub_block_ini, sub_block_end;
    this->get_start_end_sub_blocks(package->memory_address, package->memory_size, sub_block_ini, sub_block_end);

    /// Generates the Request Vector
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        package->sub_blocks[i] = ( i >= sub_block_ini && i < sub_block_end );
    }

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t package->sub_blocks[")
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (i % this->sub_block_size == 0) {
                SINUCA_PRINTF("|");
            }
            SINUCA_PRINTF("%u ", package->sub_blocks[i])
        }
        SINUCA_PRINTF("]\n")
    #endif
};

/// ============================================================================
bool line_usage_predictor_dsbp_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (package->sub_blocks[i] == true &&
        this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
            return false;
        }
    }

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t valid_sub_blocks[")
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (i % this->sub_block_size == 0) { SINUCA_PRINTF("|"); }
            SINUCA_PRINTF("%u ", this->metadata_sets[index].ways[way].valid_sub_blocks[i])
        }
        SINUCA_PRINTF("]\n")
    #endif

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
    return true;
};

/// ============================================================================
bool line_usage_predictor_dsbp_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    return (this->metadata_sets[index].ways[way].is_dead);
};


/// ============================================================================
// Mechanism Operations
/// ============================================================================
void line_usage_predictor_dsbp_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())

    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t package->sub_blocks[")
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (i % this->sub_block_size == 0) { SINUCA_PRINTF("|"); }
            SINUCA_PRINTF("%u ", package->sub_blocks[i])
        }
        SINUCA_PRINTF("]\n")
    #endif

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Hit %s", dsbp_metadata_line_to_string(&this->metadata_sets[index].ways[way]).c_str())

    // =================================================================
    // Statistics for Dynamic Energy
    uint32_t number_sub_blocks = this->metadata_sets[index].ways[way].active_sub_blocks;
    ERROR_ASSERT_PRINTF(number_sub_blocks > 0, "No active_sub_blocks during an access.\n");
    ERROR_ASSERT_PRINTF(number_sub_blocks <= sinuca_engine.get_global_line_size(), "More active_sub_blocks than the line_size.\n");
    this->stat_active_sub_block_per_access[number_sub_blocks]++;
    // =================================================================

    // Update the METADATA real_usage_counter
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        // Package Requested
        if (package->sub_blocks[i] == true) {
            this->metadata_sets[index].ways[way].real_usage_counter[i]++;
            ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");
        }
    }

    // If it is a write
    if (package->is_answer == false && package->memory_operation == MEMORY_OPERATION_WRITE) {
        this->metadata_sets[index].ways[way].is_dirty = true;

        // Written Sub-Blocks
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (package->sub_blocks[i] == true) {
                this->metadata_sets[index].ways[way].written_sub_blocks[i]++;
            }
        }
    }

    uint32_t predicted_alive = sinuca_engine.get_global_line_size();
    /// ================================================================
    /// METADATA Learn Mode
    /// ================================================================
    if (this->metadata_sets[index].ways[way].learn_mode == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE ON\n");
        // Has pht pointer
        if (this->metadata_sets[index].ways[way].pht_pointer != NULL) {
            // Update the pht
            this->add_stat_dsbp_pht_access();
            this->metadata_sets[index].ways[way].pht_pointer->last_access = sinuca_engine.get_global_cycle();
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                // Package Requested
                if (package->sub_blocks[i] == true) {
                    // Check the pht Overflow
                    if (this->metadata_sets[index].ways[way].pht_pointer->usage_counter[i] >= this->usage_counter_max) {
                        this->metadata_sets[index].ways[way].pht_pointer->overflow[i] = true;
                    }
                    else {
                        this->metadata_sets[index].ways[way].pht_pointer->usage_counter[i]++;
                    }
                }
            }
        }
    }

    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE OFF\n");
        // Turn off dead sub-blocks
        if (this->metadata_sets[index].ways[way].active_sub_blocks > 0 && this->metadata_sets[index].ways[way].is_dirty == false) {
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                // METADATA Not Overflow + METADATA Used Predicted Number of Times
                if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE &&
                this->metadata_sets[index].ways[way].overflow[i] == false &&
                this->metadata_sets[index].ways[way].real_usage_counter[i] == this->metadata_sets[index].ways[way].usage_counter[i]) {
                    // METADATA Turn off the sub_blocks
                    this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
                    this->metadata_sets[index].ways[way].active_sub_blocks--;
                    this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
                }
            }
        }

        // Check if it can be evicted earlier
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            // METADATA Not Overflow + METADATA Used Predicted Number of Times
            if (this->metadata_sets[index].ways[way].overflow[i] == false &&
            this->metadata_sets[index].ways[way].real_usage_counter[i] >= this->metadata_sets[index].ways[way].usage_counter[i]) {
                predicted_alive--;
            }
        }
    }


    // Enable/Disable IS-DEAD flag
    if (predicted_alive > 0) {
        this->metadata_sets[index].ways[way].is_dead = false;
    }
    else {
        if (this->metadata_sets[index].ways[way].is_dirty == true) {
            add_stat_dirty_lines_predicted_dead();
        }
        else {
            add_stat_clean_lines_predicted_dead();
        }
        this->metadata_sets[index].ways[way].is_dead = true;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", dsbp_metadata_line_to_string(&this->metadata_sets[index].ways[way]).c_str())
};


/// ============================================================================
void line_usage_predictor_dsbp_t::compute_static_energy(uint32_t index, uint32_t way) {

    // Statistics for Static Energy
    bool aux_computed_sub_blocks[sinuca_engine.get_global_line_size()];

    // Add the clock_become_dead for the never turned-off sub_blocks
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        }
        aux_computed_sub_blocks[i] = false;
    }

    uint32_t aux_computed_number = sinuca_engine.get_global_line_size();
    uint64_t computed_cycles = 0;

    while (aux_computed_number > 0 ) {
        uint64_t old_sub_block_clock = sinuca_engine.get_global_cycle() + 1;
        uint32_t old_sub_block_position = 0;
        uint32_t sub_blocks_become_dead = 1;

        // Find the (Not computed yet) and (become dead first)
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (aux_computed_sub_blocks[i] == false) {
                if (this->metadata_sets[index].ways[way].clock_become_dead[i] < old_sub_block_clock) {
                    old_sub_block_clock = this->metadata_sets[index].ways[way].clock_become_dead[i];
                    old_sub_block_position = i;
                    sub_blocks_become_dead = 1;
                }
                else if (this->metadata_sets[index].ways[way].clock_become_dead[i] == old_sub_block_clock) {
                    old_sub_block_clock = this->metadata_sets[index].ways[way].clock_become_dead[i];
                    old_sub_block_position = i;
                    sub_blocks_become_dead++;
                }
            }
        }

        // Compute the sub_block time of life
        uint64_t time_of_life = 0;
        if (computed_cycles == 0) {
            ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].clock_become_dead[old_sub_block_position] >= this->metadata_sets[index].ways[way].clock_become_alive[old_sub_block_position], "Buffer underflow\n")
            time_of_life = this->metadata_sets[index].ways[way].clock_become_dead[old_sub_block_position] - this->metadata_sets[index].ways[way].clock_become_alive[old_sub_block_position];
        }
        else {
            ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].clock_become_dead[old_sub_block_position] >= computed_cycles, "Buffer underflow\n")
            time_of_life = this->metadata_sets[index].ways[way].clock_become_dead[old_sub_block_position] - computed_cycles;
        }
        this->stat_active_sub_block_per_cycle[aux_computed_number] += time_of_life;

        // Update the cycles already computed.
        computed_cycles = this->metadata_sets[index].ways[way].clock_become_dead[old_sub_block_position];

        // Reduce the number of sub_blocks to compute
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].clock_become_dead[i] == old_sub_block_clock) {
                aux_computed_sub_blocks[i] = true;
                aux_computed_number--;
            }
        }
    }

    // Compute the sub_block time of life (Zero sub_blocks turned on)
    ERROR_ASSERT_PRINTF(sinuca_engine.get_global_cycle() >= computed_cycles, "Subtracting a smalled integer\n")
    uint64_t time_of_life = sinuca_engine.get_global_cycle() - computed_cycles;
    this->stat_active_sub_block_per_cycle[0] += time_of_life;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dsbp_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    this->add_stat_line_miss();

    this->line_eviction(index, way);

    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    pht_line_t *pht_line = this->dsbp_pht_find_line(package->opcode_address, package->memory_address);
    ///=================================================================
    /// pht HIT
    ///=================================================================
    if (pht_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t %s", pht_line_to_string(pht_line).c_str())

        this->add_stat_dsbp_pht_hit();

        // Update the pht entry
        this->add_stat_dsbp_pht_access();
        pht_line->last_access = sinuca_engine.get_global_cycle();

        // If no pht_pointer
        if (pht_line->pointer == false) {
            // Create a pht pointer
            pht_line->pointer = true;
            this->metadata_sets[index].ways[way].pht_pointer = pht_line;
        }
        else {
            this->metadata_sets[index].ways[way].pht_pointer = NULL;
        }

        // Clean the metadata entry
        this->metadata_sets[index].ways[way].learn_mode = false;
        this->metadata_sets[index].ways[way].is_dirty = false;
        this->metadata_sets[index].ways[way].active_sub_blocks = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            // Copy the pht prediction
            this->metadata_sets[index].ways[way].usage_counter[i] = pht_line->usage_counter[i];
            this->metadata_sets[index].ways[way].overflow[i] = pht_line->overflow[i];

            this->metadata_sets[index].ways[way].real_usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].written_sub_blocks[i] = 0;

            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
            if (package->sub_blocks[i] == true || pht_line->usage_counter[i] > 0) {
                this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
                this->metadata_sets[index].ways[way].active_sub_blocks++;
            }
        }

        // Add the last access information
        this->line_hit(package, index, way);

        // Modify the package->sub_blocks (next level request)
        package->memory_size = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE ||
            this->metadata_sets[index].ways[way].real_usage_counter[i] != 0) { //already turned off due to this access (dead on arrival)
                package->sub_blocks[i] = true;
                package->memory_size++;
            }
            else {
                package->sub_blocks[i] = false;
            }

        }
    }

    ///=================================================================
    /// pht MISS
    ///=================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht MISS\n")
        this->add_stat_dsbp_pht_miss();
        // New pht entry
        pht_line = dsbp_pht_evict_address(package->opcode_address, package->memory_address);
        // Clean the pht entry
        this->add_stat_dsbp_pht_access();
        pht_line->last_access = sinuca_engine.get_global_cycle();
        pht_line->opcode_address = package->opcode_address;
        pht_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
        pht_line->last_access = sinuca_engine.get_global_cycle();
        pht_line->pointer = true;
        this->metadata_sets[index].ways[way].pht_pointer = pht_line;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            pht_line->usage_counter[i] = 0;
            pht_line->overflow[i] = false;
        }

        // Clean the metadata entry
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].is_dirty = false;
        this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->metadata_sets[index].ways[way].usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].overflow[i] = false;

            this->metadata_sets[index].ways[way].real_usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].written_sub_blocks[i] = 0;

            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_LEARN;
        }

        // Add the last access information
        this->line_hit(package, index, way);

        // Modify the package->sub_blocks (next level request)
        package->memory_size = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            package->sub_blocks[i] = true;
            package->memory_size++;
        }
    }

};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dsbp_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    this->add_stat_sub_block_miss();

    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    // =================================================================
    // Statistics for Static Energy
    this->compute_static_energy(index, way);
    // =================================================================

    pht_line_t *pht_line = this->metadata_sets[index].ways[way].pht_pointer;
    ///=================================================================
    /// pht HIT
    ///=================================================================
    if (pht_line != NULL && pht_line->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", dsbp_metadata_line_to_string(&this->metadata_sets[index].ways[way]).c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", pht_line_to_string(pht_line).c_str())

        // Enable Learn_mode
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();

        // Update the pht entry
        this->add_stat_dsbp_pht_access();
        pht_line->last_access = sinuca_engine.get_global_cycle();
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_usage_counter[i] <= this->usage_counter_max) {
                pht_line->usage_counter[i] = this->metadata_sets[index].ways[way].real_usage_counter[i];
                pht_line->overflow[i] = false;
            }
            else {
                pht_line->usage_counter[i] = this->usage_counter_max;
                pht_line->overflow[i] = true;
            }

            // Enable all sub_blocks
            this->metadata_sets[index].ways[way].usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].overflow[i] = true;
            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            // Enable Valid Sub_blocks
            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
                this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_WRONG_FIRST;
            }
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", pht_line_to_string(pht_line).c_str())
        }

        // Add the last access information
        this->line_hit(package, index, way);

        // Enable all sub_blocks missing
        package->memory_size = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                package->sub_blocks[i] = true;
                package->memory_size++;
            }
            else {
                package->sub_blocks[i] = false;
            }
        }
    }

    ///=================================================================
    /// pht MISS
    ///=================================================================
    else {
        // Enable Learn_mode
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();

        // Enable all sub_blocks
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->metadata_sets[index].ways[way].usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].overflow[i] = true;
            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            // Enable Valid Sub_blocks
            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
                this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_WRONG_FIRST;
            }
        }

        // Add the last access information
        this->line_hit(package, index, way);

        // Enable all sub_blocks missing
        package->memory_size = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                package->sub_blocks[i] = true;
                package->memory_size++;
            }
            else {
                package->sub_blocks[i] = false;
            }
        }
    }
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dsbp_t::line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    this->add_stat_line_miss();

    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    ERROR_ASSERT_PRINTF(cache_memory != NULL && cache_line != NULL, "Wrong Cache or Cache Line");

    // Installing the CopyBack in SAME tagged line
    if (cache_memory->cmp_tag_index_bank(cache_line->tag, package->memory_address)) {
        // Clean the metadata entry
        // ~ this->metadata_sets[index].ways[way].learn_mode = true;
        // ~ this->metadata_sets[index].ways[way].is_dead = false;
        // ~ printf("%s\n", this->metadata_sets[index].ways[way].is_dead ? "DEAD": "ALIVE" );
        // Mark as dirty
        this->metadata_sets[index].ways[way].is_dirty = true;
        // ~ this->metadata_sets[index].ways[way].pht_pointer = NULL;
        this->metadata_sets[index].ways[way].active_sub_blocks = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            // ~ this->metadata_sets[index].ways[way].usage_counter[i] = 0;
            // ~ this->metadata_sets[index].ways[way].overflow[i] = false;

            // ~ this->metadata_sets[index].ways[way].real_usage_counter[i] = 0;
            // ~ this->metadata_sets[index].ways[way].written_sub_blocks[i] = 0;

            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            // ~ this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
            if (package->sub_blocks[i] == true) {
                this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_COPYBACK;
                // ~ this->metadata_sets[index].ways[way].active_sub_blocks++;
                this->metadata_sets[index].ways[way].written_sub_blocks[i]++;
            }

            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
                this->metadata_sets[index].ways[way].active_sub_blocks++;
            }
        }
    }
    // Installing the CopyBack in DIFFERENT tagged line
    else {
        this->line_eviction(index, way);
        // Clean the metadata entry
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].is_dead = false;

        // Mark as dirty
        this->metadata_sets[index].ways[way].is_dirty = true;
        this->metadata_sets[index].ways[way].pht_pointer = NULL;
        this->metadata_sets[index].ways[way].active_sub_blocks = 0;
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            this->metadata_sets[index].ways[way].usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].overflow[i] = false;

            this->metadata_sets[index].ways[way].real_usage_counter[i] = 0;
            this->metadata_sets[index].ways[way].written_sub_blocks[i] = 0;

            this->metadata_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
            if (package->sub_blocks[i] == true) {
                this->metadata_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_COPYBACK;
                this->metadata_sets[index].ways[way].active_sub_blocks++;
                this->metadata_sets[index].ways[way].written_sub_blocks[i]++;
            }
        }
    }
    // Modify the package->sub_blocks (next level request)
    package->memory_size = 1;
};


/// ============================================================================
void line_usage_predictor_dsbp_t::line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->content_to_string().c_str())

    this->add_stat_copyback();

    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    // Modify the package->sub_blocks (valid_sub_blocks)
    package->memory_size = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
            package->sub_blocks[i] = true;
            package->memory_size++;
        }
        else {
            package->sub_blocks[i] = false;
        }
    }
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")

    this->add_stat_eviction();

    // =================================================================
    // Statistics for Static Energy
    this->compute_static_energy(index, way);
    // =================================================================


    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);


    pht_line_t *pht_line = this->metadata_sets[index].ways[way].pht_pointer;
    // pht HIT
    if (pht_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", dsbp_metadata_line_to_string(&this->metadata_sets[index].ways[way]).c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", pht_line_to_string(pht_line).c_str())
        // Update the pht entry
        this->add_stat_dsbp_pht_access();
        pht_line->last_access = sinuca_engine.get_global_cycle();
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->metadata_sets[index].ways[way].real_usage_counter[i] <= this->usage_counter_max) {
                pht_line->usage_counter[i] = this->metadata_sets[index].ways[way].real_usage_counter[i];
                pht_line->overflow[i] = false;
            }
            else {
                pht_line->usage_counter[i] = this->usage_counter_max;
                pht_line->overflow[i] = true;
            }
        }
        pht_line->pointer = false;
        this->metadata_sets[index].ways[way].pht_pointer = NULL;
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", pht_line_to_string(pht_line).c_str())
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht MISS\n")
    }


    //==================================================================
    // Statistics
    uint32_t sub_blocks_touched = 0;
    uint32_t sub_blocks_written = 0;
    uint32_t sub_blocks_wrong_first = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        // Average Dead Time
        this->metadata_sets[index].ways[way].stat_total_dead_cycles += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_become_dead[i];

        // Prediction Accuracy
        switch (this->metadata_sets[index].ways[way].valid_sub_blocks[i]) {
            case LINE_SUB_BLOCK_DISABLE:
                if (this->metadata_sets[index].ways[way].real_usage_counter[i] == 0) {
                    this->stat_dsbp_line_sub_block_disable_always++;
                }
                else {
                    this->stat_dsbp_line_sub_block_disable_turnoff++;
                }
            break;

            case LINE_SUB_BLOCK_NORMAL:
                if (this->metadata_sets[index].ways[way].overflow[i] == 1) {
                    this->stat_dsbp_line_sub_block_normal_correct++;
                }
                else {
                    this->stat_dsbp_line_sub_block_normal_over++;
                }
            break;

            case LINE_SUB_BLOCK_LEARN:
                this->stat_dsbp_line_sub_block_learn++;
            break;

            case LINE_SUB_BLOCK_WRONG_FIRST:
                this->stat_dsbp_line_sub_block_wrong_first++;
            break;

            case LINE_SUB_BLOCK_COPYBACK:
                this->stat_dsbp_line_sub_block_copyback++;
            break;

        }

        // Touches before eviction
        if (this->metadata_sets[index].ways[way].real_usage_counter[i] == 0) {
            this->add_stat_sub_block_touch_0();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] == 1) {
            this->add_stat_sub_block_touch_1();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] >= 2 && this->metadata_sets[index].ways[way].real_usage_counter[i] <= 3) {
            this->add_stat_sub_block_touch_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] >= 4 && this->metadata_sets[index].ways[way].real_usage_counter[i] <= 7) {
            this->add_stat_sub_block_touch_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] >= 8 && this->metadata_sets[index].ways[way].real_usage_counter[i] <= 15) {
            this->add_stat_sub_block_touch_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] >= 16 && this->metadata_sets[index].ways[way].real_usage_counter[i] <= 127) {
            this->add_stat_sub_block_touch_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_usage_counter[i] >=128) {
            this->add_stat_sub_block_touch_128_bigger();
        }

        // Sub-Blocks accessed before eviction
        if (this->metadata_sets[index].ways[way].real_usage_counter[i] != 0) {
            sub_blocks_touched++;
        }

        if (this->metadata_sets[index].ways[way].is_dirty == true) {

            if (this->metadata_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                sub_blocks_wrong_first++;
            }

            // Written Sub-blocks before eviction
            if (this->metadata_sets[index].ways[way].written_sub_blocks[i] != 0) {
                sub_blocks_written++;
            }

            // Writes before eviction
            if (this->metadata_sets[index].ways[way].written_sub_blocks[i] == 1) {
                this->add_stat_writes_per_sub_blocks_1();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] == 2) {
                this->add_stat_writes_per_sub_blocks_2();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] == 3) {
                this->add_stat_writes_per_sub_blocks_3();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] == 4) {
                this->add_stat_writes_per_sub_blocks_4();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] == 5) {
                this->add_stat_writes_per_sub_blocks_5();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] <= 10) {
                this->add_stat_writes_per_sub_blocks_10();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] > 10 && this->metadata_sets[index].ways[way].written_sub_blocks[i] <= 100) {
                this->add_stat_writes_per_sub_blocks_100();
            }
            else if (this->metadata_sets[index].ways[way].written_sub_blocks[i] > 100) {
                this->add_stat_writes_per_sub_blocks_bigger();
            }
        }

    }
    this->stat_accessed_sub_block[sub_blocks_touched]++;

    if (this->metadata_sets[index].ways[way].is_dirty == true) {
        this->stat_written_sub_blocks_per_line[sub_blocks_written]++;
        if (sub_blocks_wrong_first != 0) {
            this->stat_written_lines_miss_predicted++;
        }
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Stats %u %"PRIu64"\n", sub_blocks_touched, this->stat_accessed_sub_block[sub_blocks_touched] )
};

/// ============================================================================
void line_usage_predictor_dsbp_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")

    (void)index;
    (void)way;
};


/// ============================================================================
std::string line_usage_predictor_dsbp_t::dsbp_metadata_line_to_string(dsbp_metadata_line_t *dsbp_metadata_line) {
    std::string PackageString;
    PackageString = "";

    PackageString = PackageString + "dsbp_LINE Learn:" + utils_t::uint32_to_char(dsbp_metadata_line->learn_mode);
    PackageString = PackageString + " Dead:" + utils_t::uint32_to_char(dsbp_metadata_line->is_dead);
    PackageString = PackageString + " pht Ptr:" + utils_t::uint32_to_char(dsbp_metadata_line->pht_pointer != NULL);

    PackageString = PackageString + "\n\t Valid Sub-Blocks      [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->valid_sub_blocks[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Real Usage Counter    [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->real_usage_counter[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Usage Counter         [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->usage_counter[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Overflow              [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->overflow[i]);
    }

    PackageString = PackageString + "]\n";
    return PackageString;
};



/// ============================================================================
/// dsbp - pht
/// ============================================================================
pht_line_t* line_usage_predictor_dsbp_t::dsbp_pht_find_line(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("dsbp_pht_find_line()\n")
    uint32_t pht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t pht_index = opcode_address & this->dsbp_pht_index_bits_mask;

    ERROR_ASSERT_PRINTF(pht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", pht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(pht_index < this->dsbp_pht_total_sets, "Wrong index %d > total_sets %d", pht_index, this->dsbp_pht_total_sets);

    for (uint32_t pht_way = 0; pht_way < this->get_dsbp_pht_associativity(); pht_way++) {
        if (this->dsbp_pht_sets[pht_index].ways[pht_way].opcode_address == opcode_address && this->dsbp_pht_sets[pht_index].ways[pht_way].offset == pht_offset) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Found pht Index %u - Way %u\n", pht_index, pht_way )
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht PC %"PRIu64" - Offset %"PRIu64" - Last Access %"PRIu64"\n", this->dsbp_pht_sets[pht_index].ways[pht_way].opcode_address,
                                                                                                    this->dsbp_pht_sets[pht_index].ways[pht_way].offset,
                                                                                                    this->dsbp_pht_sets[pht_index].ways[pht_way].last_access )
            return &this->dsbp_pht_sets[pht_index].ways[pht_way];
        }
    }
    return NULL;
}

/// ============================================================================
pht_line_t* line_usage_predictor_dsbp_t::dsbp_pht_evict_address(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("dsbp_pht_evict_address()\n")
    uint32_t pht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t pht_index = opcode_address & this->dsbp_pht_index_bits_mask;

    ERROR_ASSERT_PRINTF(pht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", pht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(pht_index < this->dsbp_pht_total_sets, "Wrong index %d > total_sets %d", pht_index, this->dsbp_pht_total_sets);

    pht_line_t *choosen_line = NULL;

    switch (this->dsbp_pht_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t pht_way = 0; pht_way < this->get_dsbp_pht_associativity(); pht_way++) {
                /// If the line is LRU
                if (this->dsbp_pht_sets[pht_index].ways[pht_way].last_access <= last_access) {
                    choosen_line = &this->dsbp_pht_sets[pht_index].ways[pht_way];
                    last_access = this->dsbp_pht_sets[pht_index].ways[pht_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t pht_way = (rand_r(&seed) % this->get_dsbp_pht_associativity());
            choosen_line = &this->dsbp_pht_sets[pht_index].ways[pht_way];
        }
        break;


        case REPLACEMENT_INVALID_OR_LRU:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_INVALID_OR_LRU not implemented.\n");
        break;

        case REPLACEMENT_FIFO:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_FIFO not implemented.\n");
        break;

        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_LRF not implemented.\n");
        break;

        case REPLACEMENT_DEAD_OR_LRU:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_LRU_DSBP should not use for line_usage_predictor.\n");
        break;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t pht PC %"PRIu64" - Offset %"PRIu64" - Last Access %"PRIu64"\n", choosen_line->opcode_address,
                                                                                                        choosen_line->offset,
                                                                                                        choosen_line->last_access )
    return choosen_line;
};

/// ============================================================================
std::string line_usage_predictor_dsbp_t::pht_line_to_string(pht_line_t *pht_line) {
    std::string PackageString;
    PackageString = "";

    PackageString = PackageString + "pht_LINE PC:" + utils_t::uint64_to_char(pht_line->opcode_address);
    PackageString = PackageString + " Offset:" + utils_t::uint32_to_char(pht_line->offset);
    PackageString = PackageString + " Last Access:" + utils_t::uint64_to_char(pht_line->last_access);
    PackageString = PackageString + " Has Pointer:" + utils_t::uint32_to_char(pht_line->pointer);

    PackageString = PackageString + "\n\t Usage Counter [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(pht_line->usage_counter[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Overflow      [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(pht_line->overflow[i]);
    }
    PackageString = PackageString + "]\n";
    return PackageString;
};


