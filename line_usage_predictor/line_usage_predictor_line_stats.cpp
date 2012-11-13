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
line_usage_predictor_line_stats_t::line_usage_predictor_line_stats_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_SUBBLOCK_STATS;

    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;
};

/// ============================================================================
line_usage_predictor_line_stats_t::~line_usage_predictor_line_stats_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dlec_metadata_set_t>(metadata_sets);
};

/// ============================================================================
void line_usage_predictor_line_stats_t::allocate() {
    line_usage_predictor_t::allocate();

    // Cache Metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()),
                        "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s %d(lines) / %d(assoc) = %d (sets))\n",
                                        this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());

    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());


    this->metadata_sets = utils_t::template_allocate_array<dlec_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dlec_metadata_line_t>(this->get_metadata_associativity());
    }

    /// ================================================================
    /// Statistics
    /// ================================================================
};

/// ============================================================================
void line_usage_predictor_line_stats_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


/// ============================================================================
void line_usage_predictor_line_stats_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    (void)package;
};

/// ============================================================================
void line_usage_predictor_line_stats_t::line_sub_blocks_to_package(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
bool line_usage_predictor_line_stats_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t HIT\n")
    return true;
};

/// ============================================================================
bool line_usage_predictor_line_stats_t::check_line_is_last_access(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
bool line_usage_predictor_line_stats_t::check_line_is_last_write(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_write()\n")

    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
/// Mechanism Operations
/// ============================================================================
void line_usage_predictor_line_stats_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_hit();

    /// Statistics
    this->metadata_sets[index].ways[way].stat_access_counter++;
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();

    if (package->memory_operation == MEMORY_OPERATION_WRITE) {
        this->metadata_sets[index].ways[way].stat_write_counter++;
        /// First write
        if (this->metadata_sets[index].ways[way].stat_clock_first_write == 0) {
            this->metadata_sets[index].ways[way].stat_clock_first_write = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();
        }
        /// NOT First Write
        else {
            this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();
        }
    }

    // Update the METADATA real_access_counter
    this->metadata_sets[index].ways[way].real_access_counter++;
    this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    if (package->memory_operation == MEMORY_OPERATION_WRITE) {
        WARNING_PRINTF("Line hit received a WRITE, this component is a LLC only.\n");
        this->metadata_sets[index].ways[way].is_dirty = true;
        this->metadata_sets[index].ways[way].need_copyback = true;
    }
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_line_stats_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_miss();

    (void)package;
    (void)index;
    (void)way;

    /// Statistics
    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();


    // Clean the metadata entry
    this->metadata_sets[index].ways[way].clean();

    this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_NORMAL;
    this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();

    // Modify the package->sub_blocks (next level request)
    package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_line_stats_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_sub_block_miss();

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_line_stats_t::line_send_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_copyback() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_send_copyback();

    (void)package;
    (void)index;
    (void)way;

    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].need_copyback = false;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_line_stats_t::line_recv_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_copyback() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_recv_copyback();

    (void)package;
    (void)index;
    (void)way;

    this->metadata_sets[index].ways[way].is_dirty = true;
    this->metadata_sets[index].ways[way].need_copyback = true;

    /// Compute Dead Cycles
    this->dead_cycles += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_read;
    this->alive_cycles += this->metadata_sets[index].ways[way].stat_clock_last_read - this->metadata_sets[index].ways[way].stat_clock_first_read;
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_read >= this->metadata_sets[index].ways[way].stat_clock_first_read, "Possible underflow");

    /// Statistics
    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();

    this->metadata_sets[index].ways[way].stat_write_counter++;
    this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();

    /// First write
    if (this->metadata_sets[index].ways[way].stat_clock_first_write == 0) {
        this->metadata_sets[index].ways[way].stat_clock_first_write = sinuca_engine.get_global_cycle();
    }

    // Update the METADATA real_access_counter
    this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_NORMAL;
    this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
};



/// ============================================================================
void line_usage_predictor_line_stats_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    this->add_stat_eviction();


    //==================================================================
    // Statistics
    // Accesses before eviction
    if (this->metadata_sets[index].ways[way].stat_access_counter == 0) {
        this->add_stat_line_access_0();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter == 1) {
        this->add_stat_line_access_1();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 2 && this->metadata_sets[index].ways[way].stat_access_counter <= 3) {
        this->add_stat_line_access_2_3();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 4 && this->metadata_sets[index].ways[way].stat_access_counter <= 7) {
        this->add_stat_line_access_4_7();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 8 && this->metadata_sets[index].ways[way].stat_access_counter <= 15) {
        this->add_stat_line_access_8_15();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 16 && this->metadata_sets[index].ways[way].stat_access_counter <= 127) {
        this->add_stat_line_access_16_127();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >=128) {
        this->add_stat_line_access_128_bigger();
    }

    // Writes before eviction
    if (this->metadata_sets[index].ways[way].stat_write_counter == 0) {
        this->add_stat_line_write_0();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter == 1) {
        this->add_stat_line_write_1();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 2 && this->metadata_sets[index].ways[way].stat_write_counter <= 3) {
        this->add_stat_line_write_2_3();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 4 && this->metadata_sets[index].ways[way].stat_write_counter <= 7) {
        this->add_stat_line_write_4_7();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 8 && this->metadata_sets[index].ways[way].stat_write_counter <= 15) {
        this->add_stat_line_write_8_15();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 16 && this->metadata_sets[index].ways[way].stat_write_counter <= 127) {
        this->add_stat_line_write_16_127();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >=128) {
        this->add_stat_line_write_128_bigger();
    }


    if (this->metadata_sets[index].ways[way].stat_clock_first_write != 0) {
        ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_read != 0, "Possible not read line");
        ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_write != 0, "Possible not written line");

        ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_read >= this->metadata_sets[index].ways[way].stat_clock_last_write, "Possible underflow");
        this->cycles_last_write_to_last_access += this->metadata_sets[index].ways[way].stat_clock_last_read - this->metadata_sets[index].ways[way].stat_clock_last_write;
        ERROR_ASSERT_PRINTF(sinuca_engine.get_global_cycle() >= this->metadata_sets[index].ways[way].stat_clock_last_write, "Possible underflow");
        this->cycles_last_write_to_eviction += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_write;
    }
    ERROR_ASSERT_PRINTF(sinuca_engine.get_global_cycle() >= this->metadata_sets[index].ways[way].stat_clock_last_read, "Possible underflow");
    this->cycles_last_access_to_eviction += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_read;

    /// Compute Dead Cycles
    this->dead_cycles += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_read;
    this->alive_cycles += this->metadata_sets[index].ways[way].stat_clock_last_read - this->metadata_sets[index].ways[way].stat_clock_first_read;
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_read >= this->metadata_sets[index].ways[way].stat_clock_first_read, "Possible underflow");

    this->metadata_sets[index].ways[way].reset_statistics();

    /// Statistics
    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();


    this->metadata_sets[index].ways[way].clean();
};

/// ============================================================================
void line_usage_predictor_line_stats_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_invalidation();

    /// Compute Dead Cycles
    this->dead_cycles += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_read;
    this->alive_cycles += this->metadata_sets[index].ways[way].stat_clock_last_read - this->metadata_sets[index].ways[way].stat_clock_first_read;
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].stat_clock_last_read >= this->metadata_sets[index].ways[way].stat_clock_first_read, "Possible underflow");

    /// Statistics
    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();


    this->metadata_sets[index].ways[way].clean();
};



/// ============================================================================
void line_usage_predictor_line_stats_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

/// ============================================================================
void line_usage_predictor_line_stats_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_line_stats_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_line_stats_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_copyback = 0;
    this->stat_recv_copyback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    this->stat_line_access_0 = 0;
    this->stat_line_access_1 = 0;
    this->stat_line_access_2_3 = 0;
    this->stat_line_access_4_7 = 0;
    this->stat_line_access_8_15 = 0;
    this->stat_line_access_16_127 = 0;
    this->stat_line_access_128_bigger = 0;

    this->stat_line_write_0 = 0;
    this->stat_line_write_1 = 0;
    this->stat_line_write_2_3 = 0;
    this->stat_line_write_4_7 = 0;
    this->stat_line_write_8_15 = 0;
    this->stat_line_write_16_127 = 0;
    this->stat_line_write_128_bigger = 0;

    this->cycles_last_write_to_last_access = 0;
    this->cycles_last_write_to_eviction = 0;
    this->cycles_last_access_to_eviction = 0;

    this->dead_cycles = 0;
    this->alive_cycles = 0;
};

/// ============================================================================
void line_usage_predictor_line_stats_t::print_statistics() {
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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_0", stat_line_access_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_1", stat_line_access_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_2_3", stat_line_access_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_4_7", stat_line_access_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_8_15", stat_line_access_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_16_127", stat_line_access_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_128_bigger", stat_line_access_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_0", stat_line_write_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_1", stat_line_write_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_2_3", stat_line_write_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_4_7", stat_line_write_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_8_15", stat_line_write_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_16_127", stat_line_write_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_128_bigger", stat_line_write_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_write_to_last_access", cycles_last_write_to_last_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_write_to_eviction", cycles_last_write_to_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_access_to_eviction", cycles_last_access_to_eviction);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "cycles_last_write_to_last_access_ratio",
                                                                                        cycles_last_write_to_last_access, stat_send_copyback);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "cycles_last_write_to_eviction_ratio",
                                                                                        cycles_last_write_to_eviction, stat_send_copyback);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "cycles_last_access_to_eviction_ratio",
                                                                                        cycles_last_access_to_eviction, stat_eviction);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "dead_cycles", dead_cycles);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "alive_cycles", alive_cycles);
    if ((alive_cycles + dead_cycles) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }

};

/// ============================================================================
void line_usage_predictor_line_stats_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);
};

