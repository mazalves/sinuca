/// ============================================================================
//
// Copyright (C) 2010, 2011, 2012
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

#ifdef BRANCH_PREDICTOR_DEBUG
    #define BRANCH_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define BRANCH_PREDICTOR_DEBUG_PRINTF(...)
#endif

/// ============================================================================
branch_predictor_t::branch_predictor_t() {
    this->branch_predictor_type = BRANCH_PREDICTOR_DISABLE;
};

/// ============================================================================
branch_predictor_t::~branch_predictor_t() {
    /// De-Allocate memory to prevent memory leak
};

/// ============================================================================
void branch_predictor_t::allocate() {
};

/// ============================================================================
void branch_predictor_t::clock(uint32_t subcycle) {
    (void) subcycle;
    BRANCH_PREDICTOR_DEBUG_PRINTF("==================== ");
    BRANCH_PREDICTOR_DEBUG_PRINTF("====================\n");
    BRANCH_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};

/// ============================================================================
int32_t branch_predictor_t::send_package(memory_package_t *package) {
    ERROR_PRINTF("Send package %s.\n", package->content_to_string().c_str());
    return POSITION_FAIL;
};

/// ============================================================================
bool branch_predictor_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {
    ERROR_PRINTF("Received package %s into the input_port %u, latency %u.\n", package->content_to_string().c_str(), input_port, transmission_latency);
    return FAIL;
};

/// ============================================================================
/// Token Controller Methods
/// ============================================================================
bool branch_predictor_t::check_token_list(memory_package_t *package) {
    ERROR_PRINTF("check_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};

/// ============================================================================
void branch_predictor_t::remove_token_list(memory_package_t *package) {
    ERROR_PRINTF("remove_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
};


/// ============================================================================
void branch_predictor_t::print_structures() {
};

/// ============================================================================
void branch_predictor_t::panic() {
    this->print_structures();
};

/// ============================================================================
void branch_predictor_t::periodic_check(){
    #ifdef BRANCH_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void branch_predictor_t::reset_statistics() {
    this->set_stat_branch_predictor_operation(0);
    this->set_stat_branch_predictor_hit(0);
    this->set_stat_branch_predictor_miss(0);

    this->set_stat_branch_predictor_taken(0);
    this->set_stat_branch_predictor_not_taken(0);

    this->set_stat_branch_predictor_conditional(0);
    this->set_stat_branch_predictor_unconditional(0);

    return;
};

/// ============================================================================
void branch_predictor_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_operation", stat_branch_predictor_operation);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_hit", stat_branch_predictor_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_miss", stat_branch_predictor_miss);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_branch_predictor_miss_ratio", stat_branch_predictor_miss, stat_branch_predictor_operation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_taken", stat_branch_predictor_taken);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_not_taken", stat_branch_predictor_not_taken);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_branch_predictor_taken_ratio", stat_branch_predictor_taken, stat_branch_predictor_operation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_conditional", stat_branch_predictor_conditional);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_branch_predictor_unconditional", stat_branch_predictor_unconditional);


};

/// ============================================================================
/// ============================================================================
void branch_predictor_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "branch_predictor_type", get_enum_branch_predictor_policy_char(branch_predictor_type));
};
