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

#include "../sinuca.hpp"

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

// ============================================================================
line_usage_predictor_dewp_oracle_t::line_usage_predictor_dewp_oracle_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DEWP_ORACLE;

    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;
};

// ============================================================================
line_usage_predictor_dewp_oracle_t::~line_usage_predictor_dewp_oracle_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dewp_metadata_set_t>(metadata_sets);
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::allocate() {
    line_usage_predictor_t::allocate();

    // Cache Metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()),
                        "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s %d(lines) / %d(assoc) = %d (sets))\n",
                                        this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());

    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());


    this->metadata_sets = utils_t::template_allocate_array<dewp_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dewp_metadata_line_t>(this->get_metadata_associativity());
    }

};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


// ============================================================================
bool line_usage_predictor_dewp_oracle_t::check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_disabled()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

// ============================================================================
bool line_usage_predictor_dewp_oracle_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

// ============================================================================
bool line_usage_predictor_dewp_oracle_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_write()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

// ============================================================================
/// Mechanism Operations
// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_line_hit();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_PREDICTION_TURNOFF) {
        this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->stat_cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }
    this->metadata_sets[index].ways[way].real_access_counter_read++;
    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_NORMAL;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_line_miss();

    /// Statistics
    this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_TURNOFF;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


// ============================================================================
void line_usage_predictor_dewp_oracle_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_PRINTF("Should never enter in sub_block_miss().\n");

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_sub_block_miss();
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_writeback() package:%s\n", package->content_to_string().c_str());

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_send_writeback();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_PREDICTION_TURNOFF) {
        this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->stat_cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_NORMAL;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_writeback() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_recv_writeback();

    /// Statistics
    this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }
    this->metadata_sets[index].ways[way].real_access_counter_writeback++;
    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_NORMAL;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};



// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_eviction();

    /// Statistics
    this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

        // Reads before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter_read == 0) {
            this->add_stat_line_read_0();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read == 1) {
            this->add_stat_line_read_1();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 2 &&
        this->metadata_sets[index].ways[way].real_access_counter_read <= 3) {
            this->add_stat_line_read_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 4 &&
        this->metadata_sets[index].ways[way].real_access_counter_read <= 7) {
            this->add_stat_line_read_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 8 &&
        this->metadata_sets[index].ways[way].real_access_counter_read <= 15) {
            this->add_stat_line_read_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 16 &&
        this->metadata_sets[index].ways[way].real_access_counter_read <= 127) {
            this->add_stat_line_read_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_read >=128) {
            this->add_stat_line_read_128_bigger();
        }

        // Reads before eviction
        if (this->metadata_sets[index].ways[way].real_access_counter_writeback == 0) {
            this->add_stat_line_writeback_0();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback == 1) {
            this->add_stat_line_writeback_1();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 2 &&
        this->metadata_sets[index].ways[way].real_access_counter_writeback <= 3) {
            this->add_stat_line_writeback_2_3();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 4 &&
        this->metadata_sets[index].ways[way].real_access_counter_writeback <= 7) {
            this->add_stat_line_writeback_4_7();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 8 &&
        this->metadata_sets[index].ways[way].real_access_counter_writeback <= 15) {
            this->add_stat_line_writeback_8_15();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 16 &&
        this->metadata_sets[index].ways[way].real_access_counter_writeback <= 127) {
            this->add_stat_line_writeback_16_127();
        }
        else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >=128) {
            this->add_stat_line_writeback_128_bigger();
        }


    this->metadata_sets[index].ways[way].real_access_counter_read = 0;
    this->metadata_sets[index].ways[way].real_access_counter_writeback = 0;

    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_TURNOFF;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_invalidation();

    /// Statistics
    this->stat_cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        stat_cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_PREDICTION_TURNOFF;
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};



// ============================================================================
void line_usage_predictor_dewp_oracle_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

// ============================================================================
/// STATISTICS
// ============================================================================
void line_usage_predictor_dewp_oracle_t::reset_statistics() {
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

    this->stat_line_writeback_0 = 0;
    this->stat_line_writeback_1 = 0;
    this->stat_line_writeback_2_3 = 0;
    this->stat_line_writeback_4_7 = 0;
    this->stat_line_writeback_8_15 = 0;
    this->stat_line_writeback_16_127 = 0;
    this->stat_line_writeback_128_bigger = 0;

    this->stat_cycles_turned_on_whole_line = 0;
    this->stat_cycles_turned_off_whole_line = 0;
    this->stat_cycles_turned_off_whole_line_since_begin = 0;
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::print_statistics() {
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

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_0", stat_line_writeback_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_1", stat_line_writeback_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_2_3", stat_line_writeback_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_4_7", stat_line_writeback_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_8_15", stat_line_writeback_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_16_127", stat_line_writeback_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_128_bigger", stat_line_writeback_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_on_whole_line", stat_cycles_turned_on_whole_line);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_off_whole_line", stat_cycles_turned_off_whole_line);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_on_whole_cache", stat_cycles_turned_on_whole_line/ metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_off_whole_cache", stat_cycles_turned_off_whole_line/ metadata_line_number);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_off_whole_line_since_begin", stat_cycles_turned_off_whole_line_since_begin);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_cycles_turned_off_since_begin_whole_cache", stat_cycles_turned_off_whole_line_since_begin/ metadata_line_number);

    if ((this->stat_cycles_turned_on_whole_line + this->stat_cycles_turned_off_whole_line) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }
};

// ============================================================================
void line_usage_predictor_dewp_oracle_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);
};

