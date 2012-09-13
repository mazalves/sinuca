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
line_usage_predictor_lwp_t::line_usage_predictor_lwp_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DISABLE;
};

/// ============================================================================
line_usage_predictor_lwp_t::~line_usage_predictor_lwp_t() {
    /// De-Allocate memory to prevent memory leak
};

/// ============================================================================
void line_usage_predictor_lwp_t::allocate() {
    line_usage_predictor_t::allocate();
};

/// ============================================================================
void line_usage_predictor_lwp_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};

/// ============================================================================
void line_usage_predictor_lwp_t::print_structures() {
    line_usage_predictor_t::allocate();
};

/// ============================================================================
void line_usage_predictor_lwp_t::panic() {
    line_usage_predictor_t::allocate();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_lwp_t::periodic_check(){
    line_usage_predictor_t::allocate();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
// STATISTICS
/// ============================================================================
void line_usage_predictor_lwp_t::reset_statistics() {
    line_usage_predictor_t::allocate();

};

/// ============================================================================
void line_usage_predictor_lwp_t::print_statistics() {
    line_usage_predictor_t::allocate();
};

/// ============================================================================
void line_usage_predictor_lwp_t::print_configuration() {
    line_usage_predictor_t::allocate();
};


/// ============================================================================

/// ============================================================================
void line_usage_predictor_lwp_t::fill_package_sub_blocks(memory_package_t *package) {
    (void)package;
};

/// ============================================================================
bool line_usage_predictor_lwp_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    return true;
};

/// ============================================================================
bool line_usage_predictor_lwp_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_lwp_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;
};



/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_lwp_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_lwp_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_lwp_t::line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)cache_memory;
    (void)cache_line;
    (void)index;
    (void)way;

    // Modify the package->sub_blocks (next level request)
    package->memory_size = 1;
};


/// ============================================================================
void line_usage_predictor_lwp_t::line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->memory_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_lwp_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")

    (void)index;
    (void)way;
};

/// ============================================================================
void line_usage_predictor_lwp_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")

    (void)index;
    (void)way;
};
