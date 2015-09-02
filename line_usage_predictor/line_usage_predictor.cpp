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
line_usage_predictor_t::line_usage_predictor_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DISABLE;
};

// ============================================================================
line_usage_predictor_t::~line_usage_predictor_t() {
};

// ============================================================================
void line_usage_predictor_t::allocate() {
};

// ============================================================================
void line_usage_predictor_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};

// ============================================================================
int32_t line_usage_predictor_t::send_package(memory_package_t *package) {
    ERROR_PRINTF("Send package %s.\n", package->content_to_string().c_str());
    return POSITION_FAIL;
};

// ============================================================================
bool line_usage_predictor_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {
    ERROR_PRINTF("Received package %s into the input_port %u, latency %u.\n", package->content_to_string().c_str(), input_port, transmission_latency);
    return FAIL;
};

// ============================================================================
/// Token Controller Methods
// ============================================================================
bool line_usage_predictor_t::pop_token_credit(uint32_t src_id, memory_operation_t memory_operation) {
    ERROR_PRINTF("pop_token_credit %" PRIu32 " %s.\n", src_id, get_enum_memory_operation_char(memory_operation))
    return FAIL;
};

// ============================================================================
void line_usage_predictor_t::print_structures() {
};

// ============================================================================
void line_usage_predictor_t::panic() {
    this->print_structures();
};

// ============================================================================
void line_usage_predictor_t::periodic_check(){
    #ifdef LINE_USAGE_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

// ============================================================================
/// STATISTICS
// ============================================================================
void line_usage_predictor_t::reset_statistics() {
};

// ============================================================================
void line_usage_predictor_t::print_statistics() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};

// ============================================================================
void line_usage_predictor_t::print_configuration() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_usage_predictor_type", get_enum_line_usage_predictor_policy_char(line_usage_predictor_type));
};
