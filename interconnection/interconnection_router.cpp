//==============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
// Eduardo Henrique Molina da Cruz
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
//==============================================================================
#include "../sinuca.hpp"

#ifdef ROUTER_DEBUG
    #define ROUTER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define ROUTER_DEBUG_PRINTF(...)
#endif

//==============================================================================
interconnection_router_t::interconnection_router_t() {
    this->send_ready_cycle = 0;

    this->last_send = 0;
    this->set_type_component(COMPONENT_INTERCONNECTION_ROUTER);
};

// =============================================================================
interconnection_router_t::~interconnection_router_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint64_t>(recv_ready_cycle);

    utils_t::template_delete_matrix<memory_package_t>(input_buffer, this->get_max_ports());
    utils_t::template_delete_array<uint32_t>(input_buffer_position_start);
    utils_t::template_delete_array<uint32_t>(input_buffer_position_end);
    utils_t::template_delete_array<uint32_t>(input_buffer_position_used);

    utils_t::template_delete_array<uint64_t>(stat_transmitted_package_size);
};

//==============================================================================
void interconnection_router_t::allocate() {
    this->recv_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_max_ports(), 0);

    this->input_buffer = utils_t::template_allocate_matrix<memory_package_t>(this->get_max_ports(), this->get_input_buffer_size());
    this->input_buffer_position_start = utils_t::template_allocate_initialize_array<uint32_t>(this->get_max_ports(), 0);
    this->input_buffer_position_end = utils_t::template_allocate_initialize_array<uint32_t>(this->get_max_ports(), 0);
    this->input_buffer_position_used = utils_t::template_allocate_initialize_array<uint32_t>(this->get_max_ports(), 0);

    this->stat_transmitted_package_size = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
};

//==============================================================================
int32_t interconnection_router_t::input_buffer_insert(uint32_t port) {
    int32_t valid_position = POSITION_FAIL;
    /// There is free space.
    if (this->input_buffer_position_used[port] < this->input_buffer_size) {
        valid_position = this->input_buffer_position_end[port];
        this->input_buffer_position_used[port]++;
        this->input_buffer_position_end[port]++;
        if (this->input_buffer_position_end[port] >= this->input_buffer_size) {
            this->input_buffer_position_end[port] = 0;
        }
    }
    return valid_position;
};

//==============================================================================
void interconnection_router_t::input_buffer_remove(uint32_t port) {
    ERROR_ASSERT_PRINTF(this->input_buffer_position_used[port] > 0, "Trying to remove from router with no used position.\n");

    this->input_buffer[port][this->input_buffer_position_start[port]].package_clean();

    this->input_buffer_position_used[port]--;
    this->input_buffer_position_start[port]++;
    if (this->input_buffer_position_start[port] >= this->input_buffer_size) {
        this->input_buffer_position_start[port] = 0;
    }
};

//==============================================================================
void interconnection_router_t::input_buffer_reinsert(uint32_t port) {
    ERROR_ASSERT_PRINTF(this->input_buffer_position_used[port] > 0, "Trying to remove from ROB with no used position.\n");

    /// There is free space. Make a copy
    if (this->input_buffer_position_used[port] < this->input_buffer_size) {
        input_buffer[port][this->input_buffer_position_end[port]] = input_buffer[port][this->input_buffer_position_start[port]];
        this->input_buffer[port][this->input_buffer_position_start[port]].package_clean();
    }

    /// Change the pointers
    this->input_buffer_position_start[port]++;
    if (this->input_buffer_position_start[port] >= this->input_buffer_size) {
        this->input_buffer_position_start[port] = 0;
    }

    this->input_buffer_position_end[port]++;
    if (this->input_buffer_position_end[port] >= this->input_buffer_size) {
        this->input_buffer_position_end[port] = 0;
    }
};

//==============================================================================
void interconnection_router_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    ROUTER_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    ROUTER_DEBUG_PRINTF("====================\n");
    ROUTER_DEBUG_PRINTF("cycle() \n");

    /// Stalls the Router / Select Package / Send Package
    /// Makes the router stalls after a package send.
    if (this->send_ready_cycle <= sinuca_engine.get_global_cycle()) {
        uint32_t port = 0;
        /// Select a port to be activated.
        switch (this->get_selection_policy()) {
            case SELECTION_RANDOM:
                port = this->selectionRandom();
            break;

            case SELECTION_ROUND_ROBIN:
                port = this->selectionRoundRobin();
            break;
        }

        /// Send the oldest UNTREATED package.
        uint32_t position = this->input_buffer_position_start[port];
        if (this->input_buffer_position_used[port] != 0 &&
        input_buffer[port][position].state == PACKAGE_STATE_UNTREATED &&
        input_buffer[port][position].ready_cycle <= sinuca_engine.get_global_cycle()) {
            ROUTER_DEBUG_PRINTF("SENDING INPUT_BUFFER[%d][%d]: %s\n", port, position, this->input_buffer[port][position].memory_to_string().c_str());
            int32_t transmission_latency = send_package(&this->input_buffer[port][position]);
            if (transmission_latency != POSITION_FAIL) {
                this->add_stat_transmissions();
                this->stat_transmitted_package_size[this->input_buffer[port][position].memory_size]++;
                this->input_buffer_remove(port);
            }
            else {
                this->input_buffer_reinsert(port);
            }
        }
    }
};

//==============================================================================
int32_t interconnection_router_t::send_package(memory_package_t *package) {
    ROUTER_DEBUG_PRINTF("send_package() package:%s\n", package->memory_to_string().c_str());

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


//==============================================================================
bool interconnection_router_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {

    if (this->recv_ready_cycle[input_port] <= sinuca_engine.get_global_cycle()) {
        ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Input Port does not exist on this Router !\n");
        ERROR_ASSERT_PRINTF(package->id_dst != this->get_id(), "Final destination is a Router !\n");
        ERROR_ASSERT_PRINTF(package->hops != NULL, "The package arrived without any routing information !\n");

        /// Get the next position into the Circular Buffer
        int32_t position = this->input_buffer_insert(input_port);
        if (position != POSITION_FAIL) {
            this->input_buffer[input_port][position] = *package;
            this->input_buffer[input_port][position].package_untreated(1);

            this->recv_ready_cycle[input_port] = sinuca_engine.get_global_cycle() + transmission_latency;
            ROUTER_DEBUG_PRINTF("\tRECV DATA OK\n");
            return OK;
        }
    }
    ROUTER_DEBUG_PRINTF("\tRECV DATA FAIL (BUSY)\n");
    return FAIL;
};


//==============================================================================
// Selection Strategies
//==============================================================================
/// Selection strategy: Random
uint32_t interconnection_router_t::selectionRandom() {
    unsigned int seed = sinuca_engine.get_global_cycle() % 1000;
    uint32_t selected = (rand_r(&seed) % this->get_max_ports());
    return selected;
};

//==============================================================================
/// Selection strategy: Round Robin
uint32_t interconnection_router_t::selectionRoundRobin() {
    this->last_send++;
    if (this->last_send >= this->get_max_ports()) {
        this->last_send = 0;
    }
    return this->last_send;
};

//==============================================================================
void interconnection_router_t::print_structures() {
    SINUCA_PRINTF("%s INPUT_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->input_buffer, this->get_max_ports(), this->input_buffer_size).c_str())
};

//==============================================================================
void interconnection_router_t::panic() {
    this->print_structures();
};

//==============================================================================
void interconnection_router_t::periodic_check(){
    #ifdef ROUTER_DEBUG
        ROUTER_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->input_buffer, this->get_max_ports(), this->input_buffer_size) == OK, "Check_age failed.\n");
};

//==============================================================================
// STATISTICS
//==============================================================================
void interconnection_router_t::reset_statistics() {
    this->set_stat_transmissions(0);
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        this->stat_transmitted_package_size[i] = 0;
    }
};

//==============================================================================
void interconnection_router_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_transmissions", stat_transmissions);

    sinuca_engine.write_statistics_small_separator();
    char name[100];
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        sprintf(name, "stat_transmitted_package_size_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_transmitted_package_size[i]);
    }
};

//==============================================================================
void interconnection_router_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_latency", this->get_interconnection_latency());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_width", this->get_interconnection_width());

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "input_buffer_size", input_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "selection_policy", get_enum_selection_char(selection_policy));
};
