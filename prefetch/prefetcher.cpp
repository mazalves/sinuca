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

#ifdef PREFETCHER_DEBUG
    #define PREFETCHER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PREFETCHER_DEBUG_PRINTF(...)
#endif


// ============================================================================
prefetch_t::prefetch_t() {
    this->prefetcher_type = PREFETCHER_DISABLE;
    this->full_buffer_type = FULL_BUFFER_STOP;

    this->request_buffer_size = 0;

    this->offset_bits_mask = 0;
    this->not_offset_bits_mask = 0;
};

// ============================================================================
prefetch_t::~prefetch_t() {
    /// De-Allocate memory to prevent memory leak
};

// ============================================================================
void prefetch_t::allocate() {
    /// Global Request Buffer
    this->request_buffer.allocate(this->request_buffer_size);

    /// OFFSET MASK
    this->offset_bits_mask = 0;
    this->not_offset_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(sinuca_engine.get_global_line_size()); i++) {
        this->offset_bits_mask |= 1 << i;
    }
    this->not_offset_bits_mask = ~offset_bits_mask;
};

// ============================================================================
void prefetch_t::clock(uint32_t subcycle) {
    (void) subcycle;
    PREFETCHER_DEBUG_PRINTF("==================== ");
    PREFETCHER_DEBUG_PRINTF("====================\n");
    PREFETCHER_DEBUG_PRINTF("cycle() \n");
};

// ============================================================================
int32_t prefetch_t::send_package(memory_package_t *package) {
    ERROR_PRINTF("Send package %s.\n", package->content_to_string().c_str());
    return POSITION_FAIL;
};

// ============================================================================
bool prefetch_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {
    ERROR_PRINTF("Received package %s into the input_port %u, latency %u.\n", package->content_to_string().c_str(), input_port, transmission_latency);
    return FAIL;
};

// ============================================================================
/// Token Controller Methods
// ============================================================================
bool prefetch_t::pop_token_credit(uint32_t src_id, memory_operation_t memory_operation) {
    ERROR_PRINTF("pop_token_credit %" PRIu32 " %s.\n", src_id, get_enum_memory_operation_char(memory_operation))
    return FAIL;
};

// ============================================================================
void prefetch_t::print_structures() {
    SINUCA_PRINTF("%s REQUEST_BUFFER SIZE:%d\n", this->get_label(), this->request_buffer.get_size());
    SINUCA_PRINTF("%s REQUEST_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(&this->request_buffer, this->request_buffer.get_capacity()).c_str());
};

// ============================================================================
void prefetch_t::panic() {
    this->print_structures();
};

// ============================================================================
void prefetch_t::periodic_check(){
    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(&this->request_buffer, this->request_buffer.get_capacity()) == OK, "Check_age failed.\n");
};

// ============================================================================
/// STATISTICS
// ============================================================================
void prefetch_t::reset_statistics() {

    this->stat_created_prefetches = 0;
    this->stat_dropped_prefetches = 0;

    this->stat_full_buffer = 0;

    this->stat_upstride_prefetches = 0;
    this->stat_downstride_prefetches = 0;
    this->stat_request_matches = 0;
};

// ============================================================================
void prefetch_t::print_statistics() {
     char title[100] = "";
    snprintf(title, sizeof(title), "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_created_prefetches", stat_created_prefetches);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dropped_prefetches", stat_dropped_prefetches);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_buffer", stat_full_buffer);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_upstride_prefetches", stat_upstride_prefetches);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_downstride_prefetches", stat_downstride_prefetches);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_request_matches", stat_request_matches);
};

// ============================================================================
void prefetch_t::print_configuration() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "prefetcher_type", get_enum_prefetch_policy_char(prefetcher_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "full_buffer_type", get_enum_full_buffer_char(full_buffer_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "request_buffer_size", request_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "offset_bits_mask", utils_t::address_to_binary(this->offset_bits_mask).c_str());
};
