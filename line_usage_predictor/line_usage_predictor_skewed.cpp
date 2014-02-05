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
line_usage_predictor_skewed_t::line_usage_predictor_skewed_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_SKEWED;

    this->early_eviction = true;
    this->early_writeback = true;
    this->turnoff_dead_lines = true;

    this->skewed_table_line_number = 0;

    this->fsm_bits = 0;
    this->fsm_max_counter = 0;
    this->fsm_dead_threshold = 0;

    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;
};

/// ============================================================================
line_usage_predictor_skewed_t::~line_usage_predictor_skewed_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<skewed_metadata_set_t>(metadata_sets);

    utils_t::template_delete_array<uint32_t>(skewed_table_1);
    utils_t::template_delete_array<uint32_t>(skewed_table_2);
    utils_t::template_delete_array<uint32_t>(skewed_table_3);
};

/// ============================================================================
void line_usage_predictor_skewed_t::allocate() {
    line_usage_predictor_t::allocate();

    /// Cache Metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()),
                        "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s %d(lines) / %d(assoc) = %d (sets))\n",
                                        this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());

    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());


    this->metadata_sets = utils_t::template_allocate_array<skewed_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<skewed_metadata_line_t>(this->get_metadata_associativity());
    }

    /// Skewed Table
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_skewed_table_line_number()),
                        "Wrong line_number(%u).\n", this->get_skewed_table_line_number());
    this->skewed_table_1 = utils_t::template_allocate_initialize_array<uint32_t>(this->get_skewed_table_line_number(), 0);
    this->skewed_table_2 = utils_t::template_allocate_initialize_array<uint32_t>(this->get_skewed_table_line_number(), 0);
    this->skewed_table_3 = utils_t::template_allocate_initialize_array<uint32_t>(this->get_skewed_table_line_number(), 0);

    /// FSM
    for (uint32_t i = 0; i < this->get_fsm_bits(); i++) {
        this->fsm_max_counter |= 1 << i;
    }
    this->fsm_dead_threshold = this->get_fsm_max_counter() / 2;

    /// Skewed Table MASK
    this->skewed_table_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_skewed_table_line_number()); i++) {
        this->skewed_table_bits_mask |= 1 << i;
    }


};

/// ============================================================================
void line_usage_predictor_skewed_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


/// ============================================================================
void line_usage_predictor_skewed_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    (void)package;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_skewed_t::line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
bool line_usage_predictor_skewed_t::check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;

    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
        return false;
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
        return true;
    }
};

/// ============================================================================
bool line_usage_predictor_skewed_t::check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_disabled()\n")

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
bool line_usage_predictor_skewed_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;

    return (this->metadata_sets[index].ways[way].is_dead_read &&
            //this->metadata_sets[index].ways[way].is_dead_writeback &&
            this->early_eviction);
};

/// ============================================================================
bool line_usage_predictor_skewed_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_write()\n")

    (void)cache;
    (void)cache_line;

    return (//this->metadata_sets[index].ways[way].is_dead_writeback &&
            this->metadata_sets[index].ways[way].is_dead_read &&
            this->metadata_sets[index].ways[way].is_dirty &&
            this->early_writeback);
};

/// ============================================================================
/// Mechanism Operations
/// ============================================================================
void line_usage_predictor_skewed_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(package->memory_operation != MEMORY_OPERATION_WRITEBACK, "Line hit received a WRITEBACK.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_line_hit();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }


// Simple module h(k) = k mod m
// Knuth Variant on Division h(k) = k(k+3) mod m
// Knuth's multiplicative method h(k) = k*2654435761 mod m

    uint64_t signature1 = 0;
    uint64_t signature2 = 0;
    uint64_t signature3 = 0;

    // SKEWED TABLE UPDATE
    if (index < 64) {
        signature1 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature %
                                            this->get_skewed_table_line_number());
        this->skewed_table_1[signature1] -= (this->skewed_table_1[signature1] > 0);/// Update the FSM

        signature2 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature *
                                            (this->metadata_sets[index].ways[way].signature + 3) %
                                            this->get_skewed_table_line_number());
        this->skewed_table_2[signature2] -= (this->skewed_table_2[signature2] > 0);/// Update the FSM

        signature3 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature * 2654435761%
                                            this->get_skewed_table_line_number());
        this->skewed_table_3[signature3] -= (this->skewed_table_3[signature3] > 0);/// Update the FSM
    }

    // SIGNATURE UPDATE
    uint64_t hash_pc = utils_t::hash_function(HASH_FUNCTION_XOR_SIMPLE, package->opcode_address, package->opcode_address >> 16, 16);
    this->metadata_sets[index].ways[way].signature += hash_pc;
    // ===

    // SKEWED TABLE CHECK PREDICTION
    signature1 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature %
                                            this->get_skewed_table_line_number());
    signature2 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature *
                                            (this->metadata_sets[index].ways[way].signature + 3) %
                                            this->get_skewed_table_line_number());
    signature3 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature * 2654435761 %
                                            this->get_skewed_table_line_number());

    uint32_t decision = this->skewed_table_1[signature1] + this->skewed_table_2[signature2] + this->skewed_table_3[signature3];

    if (decision >= (this->fsm_dead_threshold * 3) &&
    this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_WRONG_FIRST) {
        this->metadata_sets[index].ways[way].is_dead_read = true;
        // Can Turn off the sub_blocks
        if (this->metadata_sets[index].ways[way].is_dirty == false && this->turnoff_dead_lines == true) {
            this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
        }
    }
    // ===

    this->metadata_sets[index].ways[way].real_access_counter_read++;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_skewed_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE, "Metadata Line should be clean\n");

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_line_miss();

    /// Statistics
    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_skewed_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())

    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE, "Metadata Line should be clean\n");

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_sub_block_miss();

    this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    if (this->metadata_sets[index].ways[way].clock_last_access == 0){
        cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    /// If caused by misprediction or invalidation?!
    if (this->metadata_sets[index].ways[way].is_dead_read == true) {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRONG_FIRST;
    }
    else {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
    }

    this->metadata_sets[index].ways[way].is_dead_read = false;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_skewed_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_writeback() package:%s\n", package->content_to_string().c_str());

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    /// Mechanism statistics
    this->add_stat_send_writeback();

    /// Statistics
    this->cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;

    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRITEBACK;

    // If predicted as dead
    if (this->metadata_sets[index].ways[way].is_dead_read == true && this->turnoff_dead_lines == true) {
        // METADATA Turn off the sub_blocks
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_skewed_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_writeback() index:%d, way:%d package:%s\n", index, way, package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;
    (void)package;

    /// Mechanism statistics
    this->add_stat_recv_writeback();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].real_access_counter_writeback++;
    this->metadata_sets[index].ways[way].is_dirty = true;

    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;
    }
    else if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_WRITEBACK) {
        // Enable Learn_mode
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRONG_FIRST;
        this->metadata_sets[index].ways[way].is_dead_read = false;
    }

    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str());
};



/// ============================================================================
void line_usage_predictor_skewed_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

    (void)cache;
    (void)cache_line;

    /// Mechanism statistics
    this->add_stat_eviction();

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



    //==================================================================
    // Prediction Accuracy Statistics
    if (this->metadata_sets[index].ways[way].is_dirty == true) {
        switch (this->metadata_sets[index].ways[way].line_status) {
            case LINE_SUB_BLOCK_LEARN:
                ERROR_PRINTF("Should not be in LINE_SUB_BLOCK_LEARN status\n")
            break;

            case LINE_SUB_BLOCK_NORMAL:
                this->add_stat_dead_writeback_notsent_over();
            break;

            case LINE_SUB_BLOCK_DISABLE:
                ERROR_PRINTF("Should not be in LINE_SUB_BLOCK_DISABLE status\n")
            break;

            case LINE_SUB_BLOCK_WRITEBACK:
                this->add_stat_dead_read_disable_under();
            break;

            case LINE_SUB_BLOCK_WRONG_FIRST:
                this->add_stat_dead_writeback_sent_under();
            break;

        }
    }
    else {
        switch (this->metadata_sets[index].ways[way].line_status) {
            case LINE_SUB_BLOCK_LEARN:
                ERROR_PRINTF("Should not be in LINE_SUB_BLOCK_LEARN status\n")
            break;

            case LINE_SUB_BLOCK_NORMAL:
                this->add_stat_dead_read_normal_over();
            break;

            case LINE_SUB_BLOCK_DISABLE:
                this->add_stat_dead_read_disable_correct();
            break;

            case LINE_SUB_BLOCK_WRONG_FIRST:
                this->add_stat_dead_read_disable_under();
            break;

            case LINE_SUB_BLOCK_WRITEBACK:
                this->add_stat_dead_writeback_sent_correct();
            break;
        }
    }

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        if (this->metadata_sets[index].ways[way].clock_last_access == 0){
            cycles_turned_off_whole_line_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
        }
    }
    else {
        this->cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    uint64_t signature1 = 0;
    uint64_t signature2 = 0;
    uint64_t signature3 = 0;
    // SKEWED TABLE UPDATE
    if (index < 64) {
        signature1 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature %
                                            this->get_skewed_table_line_number());
        this->skewed_table_1[signature1] += (this->skewed_table_1[signature1] < this->fsm_max_counter);/// Update the FSM

        signature2 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature *
                                            (this->metadata_sets[index].ways[way].signature + 3) %
                                            this->get_skewed_table_line_number());
        this->skewed_table_2[signature2] += (this->skewed_table_2[signature2] < this->fsm_max_counter);/// Update the FSM

        signature3 = skewed_table_get_index(this->metadata_sets[index].ways[way].signature * 2654435761%
                                            this->get_skewed_table_line_number());
        this->skewed_table_3[signature3] += (this->skewed_table_3[signature3] < this->fsm_max_counter);/// Update the FSM
    }


    this->metadata_sets[index].ways[way].clean();
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_skewed_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation() index:%d, way:%d\n", index, way);
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);

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
        this->cycles_turned_on_whole_line += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_last_access;
    }

    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].clock_last_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_skewed_t::print_structures() {
    line_usage_predictor_t::print_structures();

};

/// ============================================================================
void line_usage_predictor_skewed_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_skewed_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_skewed_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_writeback = 0;
    this->stat_recv_writeback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    /// Prediction accuracy
    this->stat_dead_read_learn = 0;
    this->stat_dead_read_normal_over = 0;
    this->stat_dead_read_normal_correct = 0;
    this->stat_dead_read_disable_correct = 0;
    this->stat_dead_read_disable_under = 0;

    this->stat_dead_writeback_learn = 0;
    this->stat_dead_writeback_notsent_over = 0;
    this->stat_dead_writeback_notsent_correct = 0;
    this->stat_dead_writeback_sent_correct = 0;
    this->stat_dead_writeback_sent_under = 0;

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

    this->cycles_turned_on_whole_line = 0;
    this->cycles_turned_off_whole_line = 0;
    this->cycles_turned_off_whole_line_since_begin = 0;
};

/// ============================================================================
void line_usage_predictor_skewed_t::print_statistics() {
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

    /// prediction accuracy
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_learn", stat_dead_read_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_over", stat_dead_read_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_correct", stat_dead_read_normal_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_correct", stat_dead_read_disable_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_under", stat_dead_read_disable_under);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_learn", stat_dead_writeback_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_notsent_over", stat_dead_writeback_notsent_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_notsent_correct", stat_dead_writeback_notsent_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_sent_correct", stat_dead_writeback_sent_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_sent_under", stat_dead_writeback_sent_under);


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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_on_whole_line", cycles_turned_on_whole_line);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_line", cycles_turned_off_whole_line);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_on_whole_cache", cycles_turned_on_whole_line/ metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_cache", cycles_turned_off_whole_line/ metadata_line_number);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_whole_line_since_begin", cycles_turned_off_whole_line_since_begin);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_since_begin_whole_cache", cycles_turned_off_whole_line_since_begin/ metadata_line_number);

    if ((this->cycles_turned_on_whole_line + this->cycles_turned_off_whole_line) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }
};

/// ============================================================================
void line_usage_predictor_skewed_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_eviction", early_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_writeback", early_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "turnoff_dead_lines", turnoff_dead_lines);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// skewed table
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "skewed_table_line_number", skewed_table_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "skewed_table_bits_mask", utils_t::address_to_binary(skewed_table_bits_mask).c_str());

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fsm_bits", fsm_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fsm_max_counter", fsm_max_counter);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "fsm_dead_threshold", fsm_dead_threshold);

};

