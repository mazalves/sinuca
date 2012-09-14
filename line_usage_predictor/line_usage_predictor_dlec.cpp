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
line_usage_predictor_dlec_t::line_usage_predictor_dlec_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DLEC;

    this->usage_counter_bits = 0;
    this->usage_counter_max = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;

    /// ahtm
    this->ahtm_sets = NULL;
    this->ahtm_line_number = 0;
    this->ahtm_associativity = 0;
    this->ahtm_total_sets = 0;
    this->ahtm_replacement_policy = REPLACEMENT_LRU;

    /// ahtc
    this->ahtc_sets = NULL;
    this->ahtc_line_number = 0;
    this->ahtc_associativity = 0;
    this->ahtc_total_sets = 0;
    this->ahtc_replacement_policy = REPLACEMENT_LRU;

};

/// ============================================================================
line_usage_predictor_dlec_t::~line_usage_predictor_dlec_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dlec_metadata_set_t>(metadata_sets);
    utils_t::template_delete_array<aht_set_t>(ahtm_sets);
    utils_t::template_delete_array<aht_set_t>(ahtc_sets);
};

/// ============================================================================
void line_usage_predictor_dlec_t::allocate() {
    line_usage_predictor_t::allocate();

    this->usage_counter_max = pow(2, this->usage_counter_bits) - 1;

    ///=========================================================================
    /// metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s dsbp %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets(), this->get_sub_block_total());
    this->metadata_sets = utils_t::template_allocate_array<dlec_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dlec_metadata_line_t>(this->get_metadata_associativity());
    }

    ///=========================================================================
    /// aht misses
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_ahtm_line_number() / this->get_ahtm_associativity()), "Wrong line number(%u) or associativity(%u).\n", this->get_ahtm_line_number(), this->get_ahtm_associativity());
    this->set_ahtm_total_sets(this->get_ahtm_line_number() / this->get_ahtm_associativity());
    this->ahtm_sets = utils_t::template_allocate_array<aht_set_t>(this->get_ahtm_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s ahtm %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_ahtm_line_number(), this->get_ahtm_associativity(), this->get_ahtm_total_sets(), this->get_sub_block_total());

    /// INDEX MASK
    this->ahtm_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_ahtm_total_sets()); i++) {
        this->ahtm_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_ahtm_total_sets(); i++) {
        this->ahtm_sets[i].ways = utils_t::template_allocate_array<aht_line_t>(this->get_ahtm_associativity());
    }

    ///=========================================================================
    /// aht copyback
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_ahtc_line_number() / this->get_ahtc_associativity()), "Wrong line number(%u) or associativity(%u).\n", this->get_ahtc_line_number(), this->get_ahtc_associativity());
    this->set_ahtc_total_sets(this->get_ahtc_line_number() / this->get_ahtc_associativity());
    this->ahtc_sets = utils_t::template_allocate_array<aht_set_t>(this->get_ahtc_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s ahtc %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_ahtc_line_number(), this->get_ahtc_associativity(), this->get_ahtc_total_sets(), this->get_sub_block_total());

    /// INDEX MASK
    this->ahtc_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_ahtc_total_sets()); i++) {
        this->ahtc_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_ahtc_total_sets(); i++) {
        this->ahtc_sets[i].ways = utils_t::template_allocate_array<aht_line_t>(this->get_ahtc_associativity());
    }
};

/// ============================================================================
void line_usage_predictor_dlec_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};

/// ============================================================================
void line_usage_predictor_dlec_t::print_structures() {
    line_usage_predictor_t::allocate();
};

/// ============================================================================
void line_usage_predictor_dlec_t::panic() {
    line_usage_predictor_t::allocate();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_dlec_t::periodic_check(){
    line_usage_predictor_t::allocate();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_dlec_t::reset_statistics() {
    line_usage_predictor_t::allocate();

    this->stat_ahtm_access = 0;
    this->stat_ahtm_hit = 0;
    this->stat_ahtm_miss = 0;

    this->stat_ahtc_access = 0;
    this->stat_ahtc_hit = 0;
    this->stat_ahtc_miss = 0;
};

/// ============================================================================
void line_usage_predictor_dlec_t::print_statistics() {
    line_usage_predictor_t::allocate();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_access", stat_ahtm_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_hit", stat_ahtm_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_miss", stat_ahtm_miss);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_access", stat_ahtc_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_hit", stat_ahtc_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_miss", stat_ahtc_miss);

};

/// ============================================================================
void line_usage_predictor_dlec_t::print_configuration() {
    line_usage_predictor_t::allocate();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "usage_counter_bits", usage_counter_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "usage_counter_max", usage_counter_max);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// ahtm
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_line_number", ahtm_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_associativity", ahtm_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_total_sets", ahtm_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_replacement_policy", get_enum_replacement_char(ahtm_replacement_policy));

    /// ahtc
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_line_number", ahtc_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_associativity", ahtc_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_total_sets", ahtc_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_replacement_policy", get_enum_replacement_char(ahtc_replacement_policy));
};


/// ============================================================================

/// ============================================================================
void line_usage_predictor_dlec_t::fill_package_sub_blocks(memory_package_t *package) {
    (void)package;
};

/// ============================================================================
bool line_usage_predictor_dlec_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    return true;
};

/// ============================================================================
bool line_usage_predictor_dlec_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    (void)index;
    (void)way;

    return false;
};

/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_dlec_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;
};



/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)cache_memory;
    (void)cache_line;
    (void)index;
    (void)way;

    // Modify the package->sub_blocks (next level request)
    package->memory_size = 1;
};


/// ============================================================================
void line_usage_predictor_dlec_t::line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->content_to_string().c_str())

    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_dlec_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")

    (void)index;
    (void)way;
};

/// ============================================================================
void line_usage_predictor_dlec_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")

    (void)index;
    (void)way;
};
