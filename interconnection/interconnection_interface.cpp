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

#if defined(INTERCONNECTION_DEBUG)
    #define INTERCONNECTION_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define INTERCONNECTION_DEBUG_PRINTF(...)
#endif
//==============================================================================
interconnection_interface_t::interconnection_interface_t(){
    this->type_component = COMPONENT_NUMBER;
    strcpy(label, "NO_NAME_COMPONENT");
    this->id = 0;
    this->max_ports = 0;
    this->used_ports = 0;
    this->interconnection_width = 0;
    this->interconnection_latency = 0;
    this->ports_output_components = NULL;
    this->interface_output_components = NULL;
};

//==============================================================================
interconnection_interface_t::~interconnection_interface_t(){
    utils_t::template_delete_array<uint32_t>(this->ports_output_components);
    utils_t::template_delete_array<interconnection_interface_t*>(this->interface_output_components);
};


//==============================================================================
void interconnection_interface_t::allocate_base() {
    INTERCONNECTION_DEBUG_PRINTF("Allocate interconnection_interface_t with %u ports.\n", this->get_max_ports())
    ERROR_ASSERT_PRINTF(this->interconnection_width != 0, "Wrong bandwith for component %s.\n", get_label())
    ERROR_ASSERT_PRINTF(this->interconnection_latency != 0, "Wrong latency for component %s.\n", get_label())
    if (this->get_max_ports() > 0) {
        this->ports_output_components = utils_t::template_allocate_initialize_array<uint32_t>(this->get_max_ports(), 0);
        this->interface_output_components = utils_t::template_allocate_initialize_array<interconnection_interface_t*>(this->get_max_ports(), NULL);
    }
};

//==============================================================================
void interconnection_interface_t::set_higher_lower_level_component(interconnection_interface_t *interface_A, uint32_t port_A, interconnection_interface_t *interface_B, uint32_t port_B) {
    ERROR_ASSERT_PRINTF(interface_A->max_ports > port_A, "Connection %s (%u) => %s (%u) failed due to small amount of ports %u.\n",
                                                interface_A->get_label(), port_A, interface_B->get_label(), port_B, interface_A->max_ports)
    ERROR_ASSERT_PRINTF(interface_B->max_ports > port_B, "Connection %s (%u) => %s (%u) failed due to small amount of ports %u.\n",
                                                interface_B->get_label(), port_B, interface_A->get_label(), port_A, interface_B->max_ports)

    interface_A->interface_output_components[port_A] = interface_B;
    interface_A->ports_output_components[port_A] = port_B;

    interface_B->interface_output_components[port_B] = interface_A;
    interface_B->ports_output_components[port_B] = port_A;
};

//==============================================================================
int32_t interconnection_interface_t::find_port_to_obj(interconnection_interface_t *obj) {
    uint32_t i;
    for (i = 0; i < this->get_max_ports(); i++) {
        if (this->get_interface_output_component(i) == obj)
            return i;
    }
    return POSITION_FAIL;
};


//==============================================================================
int32_t interconnection_interface_t::find_port_from_obj(interconnection_interface_t *obj) {
    uint32_t i;
    for (i = 0; i < this->get_max_ports(); i++) {
        if (this->get_interface_output_component(i) == obj)
            return this->get_ports_output_component(i);
    }
    return POSITION_FAIL;
};
