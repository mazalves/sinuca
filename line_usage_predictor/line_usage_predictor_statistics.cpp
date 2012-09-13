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
line_usage_predictor_statistics_t::line_usage_predictor_statistics_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DISABLE;

    this->dsbp_sub_block_size = 0;
    this->dsbp_sub_block_total = 0;

    this->dsbp_usage_counter_bits = 0;
    this->dsbp_usage_counter_max = 0;

    this->dsbp_sets = NULL;
    this->dsbp_line_number = 0;
    this->dsbp_associativity = 0;
    this->dsbp_total_sets = 0;


    /// STATISTICS
    this->stat_accessed_sub_block = NULL;
    this->stat_active_sub_block_per_access = NULL;
    this->stat_active_sub_block_per_cycle = NULL;
    this->stat_written_sub_blocks_per_line = NULL;
};

/// ============================================================================
line_usage_predictor_statistics_t::~line_usage_predictor_statistics_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dsbp_metadata_set_t>(dsbp_sets);

    utils_t::template_delete_array<uint64_t>(stat_accessed_sub_block);
    utils_t::template_delete_array<uint64_t>(stat_active_sub_block_per_access);
    utils_t::template_delete_array<uint64_t>(stat_active_sub_block_per_cycle);
    utils_t::template_delete_array<uint64_t>(stat_written_sub_blocks_per_line);
};

/// ============================================================================
void line_usage_predictor_statistics_t::allocate() {
    line_usage_predictor_t::allocate();

    // Cache Metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size() / this->get_dsbp_sub_block_size()), "Wrong line_size(%u) or sub_block_size(%u).\n", this->get_dsbp_line_number(), this->get_dsbp_associativity());
    this->set_dsbp_sub_block_total(sinuca_engine.get_global_line_size() / this->get_dsbp_sub_block_size());

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_dsbp_line_number() / this->get_dsbp_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_dsbp_line_number(), this->get_dsbp_associativity());
    this->set_dsbp_total_sets(this->get_dsbp_line_number() / this->get_dsbp_associativity());

    this->dsbp_usage_counter_max = pow(2, this->dsbp_usage_counter_bits) -1;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s dsbp %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_dsbp_line_number(), this->get_dsbp_associativity(), this->get_dsbp_total_sets(), this->get_dsbp_sub_block_total());
    this->dsbp_sets = utils_t::template_allocate_array<dsbp_metadata_set_t>(this->get_dsbp_total_sets());
    for (uint32_t i = 0; i < this->get_dsbp_total_sets(); i++) {
        this->dsbp_sets[i].ways = utils_t::template_allocate_array<dsbp_metadata_line_t>(this->get_dsbp_associativity());

        for (uint32_t j = 0; j < this->get_dsbp_associativity(); j++) {
            this->dsbp_sets[i].ways[j].valid_sub_blocks = utils_t::template_allocate_initialize_array<line_sub_block_t>(sinuca_engine.get_global_line_size(), LINE_SUB_BLOCK_DISABLE);
            this->dsbp_sets[i].ways[j].real_usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->dsbp_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->dsbp_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);

            this->dsbp_sets[i].ways[j].clock_become_alive = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
            this->dsbp_sets[i].ways[j].clock_become_dead = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->dsbp_sets[i].ways[j].written_sub_blocks = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);

            this->dsbp_sets[i].ways[j].active_sub_blocks = 0;
            this->dsbp_sets[i].ways[j].is_dead = true;
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
void line_usage_predictor_statistics_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");

};

/// ============================================================================
void line_usage_predictor_statistics_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

/// ============================================================================
void line_usage_predictor_statistics_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_statistics_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
// STATISTICS
/// ============================================================================
void line_usage_predictor_statistics_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_copyback = 0;
    this->stat_eviction = 0;

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
void line_usage_predictor_statistics_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    uint64_t stat_average_dead_cycles = 0;
    for (uint32_t index = 0; index < this->get_dsbp_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_dsbp_associativity(); way++) {
            this->line_eviction(index, way);
            stat_average_dead_cycles += this->dsbp_sets[index].ways[way].stat_total_dead_cycles / sinuca_engine.get_global_line_size();
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_average_dead_cycles", stat_average_dead_cycles);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback", stat_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);

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
void line_usage_predictor_statistics_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    /// ====================================================================
    /// dsbp
    /// ====================================================================
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_usage_counter_bits", dsbp_usage_counter_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_usage_counter_max", dsbp_usage_counter_max);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_sub_block_size", dsbp_sub_block_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_sub_block_total", dsbp_sub_block_total);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_line_number", dsbp_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_associativity", dsbp_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dsbp_total_sets", dsbp_total_sets);

};


/// ============================================================================
// Input:   base_address, size
// Output:  start_sub_block, end_Sub_block
void line_usage_predictor_statistics_t::get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end) {
    ERROR_ASSERT_PRINTF(size > 0, "Received a request with invalid size.\n")
    uint64_t address_offset = base_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t address_size = address_offset + size;
    if (address_size >= sinuca_engine.get_global_line_size()){
        address_size = sinuca_engine.get_global_line_size();
    }

    sub_block_ini = uint32_t(address_offset / this->dsbp_sub_block_size) * this->dsbp_sub_block_size;
    sub_block_end = uint32_t( (address_size / this->dsbp_sub_block_size) +
                                        uint32_t((address_size % this->dsbp_sub_block_size) != 0) ) * this->dsbp_sub_block_size;
}

/// ============================================================================
void line_usage_predictor_statistics_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->memory_to_string().c_str())

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
            if (i % this->dsbp_sub_block_size == 0) {
                SINUCA_PRINTF("|");
            }
            SINUCA_PRINTF("%u ", package->sub_blocks[i])
        }
        SINUCA_PRINTF("]\n")
    #endif
};

/// ============================================================================
bool line_usage_predictor_statistics_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->memory_to_string().c_str())

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")

    (void)package;
    (void)index;
    (void)way;

    return true;
};

/// ============================================================================
bool line_usage_predictor_statistics_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    (void)index;
    (void)way;

    return false;
};


/// ============================================================================
// Mechanism Operations
/// ============================================================================
void line_usage_predictor_statistics_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->memory_to_string().c_str())

    // Update the METADATA real_usage_counter
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->dsbp_sets[index].ways[way].real_usage_counter[i] += package->sub_blocks[i];
        if (package->sub_blocks[i] == true) {
            this->dsbp_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        }
    }
};

/*
/// ============================================================================
void line_usage_predictor_statistics_t::compute_static_energy(uint32_t index, uint32_t way) {

    // Statistics for Static Energy
    bool aux_computed_sub_blocks[sinuca_engine.get_global_line_size()];

    // Add the clock_become_dead for the never turned-off sub_blocks
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (this->dsbp_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
            this->dsbp_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
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
                if (this->dsbp_sets[index].ways[way].clock_become_dead[i] < old_sub_block_clock) {
                    old_sub_block_clock = this->dsbp_sets[index].ways[way].clock_become_dead[i];
                    old_sub_block_position = i;
                    sub_blocks_become_dead = 1;
                }
                else if (this->dsbp_sets[index].ways[way].clock_become_dead[i] == old_sub_block_clock) {
                    old_sub_block_clock = this->dsbp_sets[index].ways[way].clock_become_dead[i];
                    old_sub_block_position = i;
                    sub_blocks_become_dead++;
                }
            }
        }

        // Compute the sub_block time of life
        uint64_t time_of_life = 0;
        if (computed_cycles == 0) {
            ERROR_ASSERT_PRINTF(this->dsbp_sets[index].ways[way].clock_become_dead[old_sub_block_position] >= this->dsbp_sets[index].ways[way].clock_become_alive[old_sub_block_position], "Buffer underflow\n")
            time_of_life = this->dsbp_sets[index].ways[way].clock_become_dead[old_sub_block_position] - this->dsbp_sets[index].ways[way].clock_become_alive[old_sub_block_position];
        }
        else {
            ERROR_ASSERT_PRINTF(this->dsbp_sets[index].ways[way].clock_become_dead[old_sub_block_position] >= computed_cycles, "Buffer underflow\n")
            time_of_life = this->dsbp_sets[index].ways[way].clock_become_dead[old_sub_block_position] - computed_cycles;
        }
        this->stat_active_sub_block_per_cycle[aux_computed_number] += time_of_life;

        // Update the cycles already computed.
        computed_cycles = this->dsbp_sets[index].ways[way].clock_become_dead[old_sub_block_position];

        // Reduce the number of sub_blocks to compute
        for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
            if (this->dsbp_sets[index].ways[way].clock_become_dead[i] == old_sub_block_clock) {
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
*/

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_statistics_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->memory_to_string().c_str())
    this->add_stat_line_miss();

    this->line_eviction(index, way);
    package->memory_size = sinuca_engine.get_global_line_size();

    // Clean the metadata entry
    this->dsbp_sets[index].ways[way].learn_mode = true;
    this->dsbp_sets[index].ways[way].is_dirty = false;
    this->dsbp_sets[index].ways[way].pht_pointer = NULL;
    this->dsbp_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->dsbp_sets[index].ways[way].usage_counter[i] = 0;
        this->dsbp_sets[index].ways[way].overflow[i] = true;

        this->dsbp_sets[index].ways[way].real_usage_counter[i] = 0;
        this->dsbp_sets[index].ways[way].written_sub_blocks[i] = 0;

        this->dsbp_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
        this->dsbp_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        this->dsbp_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
    }

    // Add the last access information
    this->line_hit(package, index, way);

    // Modify the package->sub_blocks (next level request)
    package->memory_size = sinuca_engine.get_global_line_size();


};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_statistics_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->memory_to_string().c_str())
    this->add_stat_sub_block_miss();

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_statistics_t::line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->memory_to_string().c_str())
    (void)cache_memory;
    (void)cache_line;


    this->add_stat_line_miss();

    this->line_eviction(index, way);

    // Clean the metadata entry
    this->dsbp_sets[index].ways[way].learn_mode = true;
    this->dsbp_sets[index].ways[way].is_dead = false;
    this->dsbp_sets[index].ways[way].is_dirty = true;
    this->dsbp_sets[index].ways[way].pht_pointer = NULL;
    this->dsbp_sets[index].ways[way].active_sub_blocks = sinuca_engine.get_global_line_size();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        this->dsbp_sets[index].ways[way].usage_counter[i] = 0;
        this->dsbp_sets[index].ways[way].overflow[i] = true;

        this->dsbp_sets[index].ways[way].real_usage_counter[i] = 0;
        this->dsbp_sets[index].ways[way].written_sub_blocks[i] = 0;

        this->dsbp_sets[index].ways[way].clock_become_alive[i] = sinuca_engine.get_global_cycle();
        this->dsbp_sets[index].ways[way].clock_become_dead[i] = sinuca_engine.get_global_cycle();
        this->dsbp_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
    }

    // Modify the package->sub_blocks (next level request)
    package->memory_size = 1;
};


/// ============================================================================
void line_usage_predictor_statistics_t::line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    this->add_stat_copyback();
    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_statistics_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")

    this->add_stat_eviction();

    ERROR_ASSERT_PRINTF(index < this->dsbp_total_sets, "Wrong index %d > total_sets %d", index, this->dsbp_total_sets);
    ERROR_ASSERT_PRINTF(way < this->dsbp_associativity, "Wrong way %d > associativity %d", way, this->dsbp_associativity);

    //==================================================================
    // Statistics
    uint32_t sub_blocks_touched = 0;
    uint32_t sub_blocks_written = 0;
    uint32_t sub_blocks_wrong_first = 0;
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        // Average Dead Time
        this->dsbp_sets[index].ways[way].stat_total_dead_cycles += sinuca_engine.get_global_cycle() - this->dsbp_sets[index].ways[way].clock_become_dead[i];
/*
        // Prediction Accuracy
        switch (this->dsbp_sets[index].ways[way].valid_sub_blocks[i]) {
            case LINE_SUB_BLOCK_DISABLE:
                if (this->dsbp_sets[index].ways[way].real_usage_counter[i] == 0) {
                    this->stat_dsbp_line_sub_block_disable_always++;
                }
                else {
                    this->stat_dsbp_line_sub_block_disable_turnoff++;
                }
            break;

            case LINE_SUB_BLOCK_NORMAL:
                if (this->dsbp_sets[index].ways[way].overflow[i] == 1) {
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
*/

        // Touches before eviction
        if (this->dsbp_sets[index].ways[way].real_usage_counter[i] == 0) {
            this->add_stat_sub_block_touch_0();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] == 1) {
            this->add_stat_sub_block_touch_1();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] >= 2 && this->dsbp_sets[index].ways[way].real_usage_counter[i] <= 3) {
            this->add_stat_sub_block_touch_2_3();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] >= 4 && this->dsbp_sets[index].ways[way].real_usage_counter[i] <= 7) {
            this->add_stat_sub_block_touch_4_7();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] >= 8 && this->dsbp_sets[index].ways[way].real_usage_counter[i] <= 15) {
            this->add_stat_sub_block_touch_8_15();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] >= 16 && this->dsbp_sets[index].ways[way].real_usage_counter[i] <= 127) {
            this->add_stat_sub_block_touch_16_127();
        }
        else if (this->dsbp_sets[index].ways[way].real_usage_counter[i] >=128) {
            this->add_stat_sub_block_touch_128_bigger();
        }

        // Sub-Blocks accessed before eviction
        if (this->dsbp_sets[index].ways[way].real_usage_counter[i] != 0) {
            sub_blocks_touched++;
        }

        if (this->dsbp_sets[index].ways[way].is_dirty == true) {

            if (this->dsbp_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                sub_blocks_wrong_first++;
            }

            // Written Sub-blocks before eviction
            if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] != 0) {
                sub_blocks_written++;
            }

            // Writes before eviction
            if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] == 1) {
                this->add_stat_writes_per_sub_blocks_1();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] == 2) {
                this->add_stat_writes_per_sub_blocks_2();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] == 3) {
                this->add_stat_writes_per_sub_blocks_3();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] == 4) {
                this->add_stat_writes_per_sub_blocks_4();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] == 5) {
                this->add_stat_writes_per_sub_blocks_5();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] <= 10) {
                this->add_stat_writes_per_sub_blocks_10();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] > 10 && this->dsbp_sets[index].ways[way].written_sub_blocks[i] <= 100) {
                this->add_stat_writes_per_sub_blocks_100();
            }
            else if (this->dsbp_sets[index].ways[way].written_sub_blocks[i] > 100) {
                this->add_stat_writes_per_sub_blocks_bigger();
            }
        }

    }
    this->stat_accessed_sub_block[sub_blocks_touched]++;

    if (this->dsbp_sets[index].ways[way].is_dirty == true) {
        this->stat_written_sub_blocks_per_line[sub_blocks_written]++;
        if (sub_blocks_wrong_first != 0) {
            this->stat_written_lines_miss_predicted++;
        }
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Stats %u %"PRIu64"\n", sub_blocks_touched, this->stat_accessed_sub_block[sub_blocks_touched] )
};

/// ============================================================================
void line_usage_predictor_statistics_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")

    (void)index;
    (void)way;
};


/// ============================================================================
std::string line_usage_predictor_statistics_t::dsbp_metadata_line_to_string(dsbp_metadata_line_t *dsbp_metadata_line) {
    std::string PackageString;
    PackageString = "";

    PackageString = PackageString + "dsbp_LINE Learn:" + utils_t::uint32_to_char(dsbp_metadata_line->learn_mode);
    PackageString = PackageString + " Dead:" + utils_t::uint32_to_char(dsbp_metadata_line->is_dead);
    PackageString = PackageString + " pht Ptr:" + utils_t::uint32_to_char(dsbp_metadata_line->pht_pointer != NULL);

    PackageString = PackageString + "\n\t Valid Sub-Blocks      [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->dsbp_sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->valid_sub_blocks[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Real Usage Counter    [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->dsbp_sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->real_usage_counter[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Usage Counter         [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->dsbp_sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->usage_counter[i]);
    }
    PackageString = PackageString + "]\n";

    PackageString = PackageString + "\t Overflow              [";
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
        if (i % this->dsbp_sub_block_size == 0) {
            PackageString = PackageString + "|";
        }
        PackageString = PackageString + " " + utils_t::uint32_to_char(dsbp_metadata_line->overflow[i]);
    }

    PackageString = PackageString + "]\n";
    return PackageString;
};


