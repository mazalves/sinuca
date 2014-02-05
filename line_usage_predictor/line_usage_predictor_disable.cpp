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
line_usage_predictor_disable_t::line_usage_predictor_disable_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DISABLE;
};

/// ============================================================================
line_usage_predictor_disable_t::~line_usage_predictor_disable_t() {
    /// De-Allocate memory to prevent memory leak
};

/// ============================================================================
void line_usage_predictor_disable_t::allocate() {
    line_usage_predictor_t::allocate();
};

/// ============================================================================
void line_usage_predictor_disable_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};

/// ============================================================================

/// ============================================================================
void line_usage_predictor_disable_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())
    (void)package;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_disable_t::line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
bool line_usage_predictor_disable_t::check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    return true;
};

/// ============================================================================
bool line_usage_predictor_disable_t::check_line_is_disabled(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_disabled()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};


/// ============================================================================
bool line_usage_predictor_disable_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
bool line_usage_predictor_disable_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
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
void line_usage_predictor_disable_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    this->add_stat_line_hit();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;
};



/// ============================================================================
void line_usage_predictor_disable_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    this->add_stat_line_miss();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;
};


/// ============================================================================
void line_usage_predictor_disable_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    this->add_stat_sub_block_miss();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;
};


/// ============================================================================
void line_usage_predictor_disable_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_writeback() package:%s\n", package->content_to_string().c_str())
    this->add_stat_send_writeback();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;
};


/// ============================================================================
void line_usage_predictor_disable_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_writeback() package:%s\n", package->content_to_string().c_str())
    this->add_stat_recv_writeback();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;
};


/// ============================================================================
void line_usage_predictor_disable_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    this->add_stat_eviction();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;
};

/// ============================================================================
void line_usage_predictor_disable_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    this->add_stat_invalidation();         /// Access Statistics

    (void)cache;
    (void)cache_line;
    (void)index;
    (void)way;
};


/// ============================================================================
void line_usage_predictor_disable_t::print_structures() {
    line_usage_predictor_t::print_structures();
};

/// ============================================================================
void line_usage_predictor_disable_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_disable_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_disable_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_writeback = 0;
    this->stat_recv_writeback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;
};

/// ============================================================================
void line_usage_predictor_disable_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_hit", stat_line_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_send_writeback", stat_send_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_recv_writeback", stat_recv_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);
};

/// ============================================================================
void line_usage_predictor_disable_t::print_configuration() {
    line_usage_predictor_t::allocate();
};

