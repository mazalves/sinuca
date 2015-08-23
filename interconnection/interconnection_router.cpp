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

#ifdef ROUTER_DEBUG
    #define ROUTER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define ROUTER_DEBUG_PRINTF(...)
#endif

// ============================================================================
interconnection_router_t::interconnection_router_t() {
    this->set_type_component(COMPONENT_INTERCONNECTION_ROUTER);

    this->selection_policy = SELECTION_ROUND_ROBIN;

    this->packages_inside_router = 0;

    this->input_buffer = NULL;
    this->input_buffer_size = 0;

    this->send_ready_cycle = 0;
    this->recv_ready_cycle = NULL;

    this->last_selected = 0;

    this->stat_transmitted_package_size = NULL;
};

// =============================================================================
interconnection_router_t::~interconnection_router_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint64_t>(recv_ready_cycle);
    utils_t::template_delete_array< circular_buffer_t<memory_package_t> >(input_buffer);
    utils_t::template_delete_array<uint64_t>(stat_transmitted_package_size);
};

// ============================================================================
void interconnection_router_t::allocate() {
    this->recv_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_max_ports(), 0);

    this->input_buffer = utils_t::template_allocate_array< circular_buffer_t<memory_package_t> >(this->get_max_ports());
    for (uint32_t i = 0; i < this->get_max_ports(); i++) {
        this->input_buffer[i].allocate(this->get_input_buffer_size());
    }

    this->stat_transmitted_package_size = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);
};


// ============================================================================
void interconnection_router_t::clock(uint32_t subcycle) {
    (void) subcycle;
    ROUTER_DEBUG_PRINTF("==================== ID(%u) ", this->get_id());
    ROUTER_DEBUG_PRINTF("====================\n");
    ROUTER_DEBUG_PRINTF("cycle() \n");

    /// Nothing to be done this cycle. -- Improve the performance
    if (this->packages_inside_router == 0) return;

    /// Stalls the Router / Select Package / Send Package
    /// Makes the router stalls after a package send.
    if (this->send_ready_cycle <= sinuca_engine.get_global_cycle()) {
        uint32_t port = 0;
        /// Select a port to be activated.
        switch (this->get_selection_policy()) {
            case SELECTION_ROUND_ROBIN:
                port = this->selection_round_robin();
            break;

            case SELECTION_RANDOM:
                port = this->selection_random();
            break;

            case SELECTION_BUFFER_LEVEL:
                port = this->selection_buffer_level();
            break;
        }

        if (this->input_buffer[port].is_empty()) {
            ROUTER_DEBUG_PRINTF("PORT[%d] IS EMPTY\n", port);
            return;
        }

        if (this->input_buffer[port].front()->state == PACKAGE_STATE_UNTREATED &&
            this->input_buffer[port].front()->ready_cycle <= sinuca_engine.get_global_cycle()) {

            ROUTER_DEBUG_PRINTF("SENDING INPUT_BUFFER[%d]: %s\n", port, this->input_buffer[port].front()->content_to_string().c_str());
            int32_t transmission_latency = send_package(this->input_buffer[port].front());
            if (transmission_latency != POSITION_FAIL) {
                this->input_buffer[port].front()->package_clean();
                this->input_buffer[port].pop_front();
                this->packages_inside_router--;
            }
            else {
                this->input_buffer[port].pop_push();
            }
        }

    }
};

// ============================================================================
int32_t interconnection_router_t::send_package(memory_package_t *package) {
    ROUTER_DEBUG_PRINTF("send_package() package:%s\n", package->content_to_string().c_str());

    if (this->send_ready_cycle <= sinuca_engine.get_global_cycle()) {
        ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
        uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
        ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
        package->hop_count--;  /// Consume its own port

        uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package, this, this->get_interface_output_component(output_port));
        bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port), transmission_latency);
        if (sent) {
            ROUTER_DEBUG_PRINTF("\tSEND DATA OK\n");
            this->send_ready_cycle = sinuca_engine.get_global_cycle() + transmission_latency;

            /// Statistics
            this->add_stat_transmissions();
            this->stat_transmitted_package_size[package->memory_size]++;
            this->stat_total_send_size += package->memory_size;
            this->stat_total_send_flits += package->memory_size / this->get_interconnection_width();

            return transmission_latency;
        }
        else {
            ROUTER_DEBUG_PRINTF("\tSEND DATA FAIL\n");
            package->hop_count++;  /// Do not Consume its own port
            return POSITION_FAIL;
        }
    }
    ROUTER_DEBUG_PRINTF("\tSEND DATA FAIL (BUSY)\n");
    return POSITION_FAIL;
};


// ============================================================================
bool interconnection_router_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {

    if (this->recv_ready_cycle[input_port] <= sinuca_engine.get_global_cycle()) {
        ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Input Port does not exist on this Router !\n");
        ERROR_ASSERT_PRINTF(package->id_dst != this->get_id(), "Final destination is a Router !\n");
        ERROR_ASSERT_PRINTF(package->hops != NULL, "The package arrived without any routing information !\n");

        /// Get the next position into the Circular Buffer
        int32_t position = this->input_buffer[input_port].push_back(*package);
        if (position != POSITION_FAIL) {
            ROUTER_DEBUG_PRINTF("\tRECV DATA OK\n");
            this->input_buffer[input_port].back()->package_untreated(1);
            this->packages_inside_router++;

            this->recv_ready_cycle[input_port] = sinuca_engine.get_global_cycle() + transmission_latency;

            /// Statistics
            this->stat_total_recv_size += package->memory_size;
            this->stat_total_recv_flits += package->memory_size / this->get_interconnection_width();
            return OK;
        }
    }
    ROUTER_DEBUG_PRINTF("\tRECV DATA FAIL (BUSY)\n");
    return FAIL;
};

// ============================================================================
/// Token Controller Methods
// ============================================================================
bool interconnection_router_t::check_token_list(memory_package_t *package) {
    ERROR_PRINTF("check_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};

// ============================================================================
void interconnection_router_t::remove_token_list(memory_package_t *package) {
    ERROR_PRINTF("remove_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
};


// ============================================================================
// Selection Strategies
// ============================================================================

// ============================================================================
/// Selection strategy: Round Robin
uint32_t interconnection_router_t::selection_round_robin() {

    for (uint32_t i = 0; i < this->get_max_ports(); i++) {
        this->last_selected++;
        if (this->last_selected >= this->get_max_ports()) {
            this->last_selected = 0;
        }
        if (!this->input_buffer[last_selected].is_empty()) {
            break;
        }
    }

    return this->last_selected;
};

// ============================================================================
/// Selection strategy: Random
uint32_t interconnection_router_t::selection_random() {
    this->last_selected = sinuca_engine.get_global_cycle() % this->get_max_ports();
    return this->last_selected;
};

// ============================================================================
/// Selection strategy: Buffer Level
uint32_t interconnection_router_t::selection_buffer_level(){
    uint32_t size_selected = 0;
    this->last_selected = 0;
    for (uint32_t i = 0; i < this->get_max_ports(); i++) {
        uint32_t total = this->input_buffer[i].get_size();

        if (total > size_selected) {
            this->last_selected = i;
            size_selected = total;
        }
    }
    return this->last_selected;
};

// ============================================================================
void interconnection_router_t::print_structures() {
    for (uint32_t i = 0; i < this->get_max_ports(); i++) {
        SINUCA_PRINTF("%s INPUT_BUFFER PORT:%u SIZE:%u BEG:%u END:%u\n", this->get_label(), i, this->input_buffer[i].get_size(), this->input_buffer[i].beg_index, this->input_buffer[i].end_index);
        SINUCA_PRINTF("%s INPUT_BUFFER:\n%s", this->get_label(),
                memory_package_t::print_all(&this->input_buffer[i], this->input_buffer[i].get_capacity()).c_str())
    }
};

// ============================================================================
void interconnection_router_t::panic() {
    this->print_structures();
};

// ============================================================================
void interconnection_router_t::periodic_check(){
    #ifdef ROUTER_DEBUG
        ROUTER_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    for (uint32_t i = 0; i < this->get_max_ports(); i++) {
        ERROR_ASSERT_PRINTF(memory_package_t::check_age(&this->input_buffer[i], this->input_buffer[i].get_capacity()) == OK,
                    "Check_age failed.\n");
    }
};

// ============================================================================
/// STATISTICS
// ============================================================================
void interconnection_router_t::reset_statistics() {
    this->set_stat_transmissions(0);

    this->set_stat_total_send_size(0);
    this->set_stat_total_recv_size(0);

    this->set_stat_total_send_flits(0);
    this->set_stat_total_recv_flits(0);

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        this->stat_transmitted_package_size[i] = 0;
    }
};

// ============================================================================
void interconnection_router_t::print_statistics() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_transmissions", stat_transmissions);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_total_send_size", this->stat_total_send_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_total_recv_size", this->stat_total_recv_size);

    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "send_size_per_cycle_warm", stat_total_send_size,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "recv_size_per_cycle_warm", this->stat_total_recv_size,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_total_send_flits", this->stat_total_send_flits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_total_recv_flits", this->stat_total_recv_flits);

    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "send_flits_per_cycle_warm", stat_total_send_flits,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "recv_flits_per_cycle_warm", this->stat_total_recv_flits,
                                                                                                                       sinuca_engine.get_global_cycle() - sinuca_engine.get_reset_cycle());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_small_separator();

    char name[100];
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i += 4) {
        sprintf(name, "stat_transmitted_package_size_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_transmitted_package_size[i]);
    }
};

// ============================================================================
void interconnection_router_t::print_configuration() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_latency", this->get_interconnection_latency());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_width", this->get_interconnection_width());

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "input_buffer_size", input_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "selection_policy", get_enum_selection_char(selection_policy));
};
