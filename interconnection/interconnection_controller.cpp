/// ============================================================================
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
/// ============================================================================
#include "../sinuca.hpp"

#ifdef INTERCONNECTION_CTRL_DEBUG
    #define INTERCONNECTION_CTRL_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define INTERCONNECTION_CTRL_DEBUG_PRINTF(...)
#endif

/// ============================================================================
/// routing_table_element_t
/// ============================================================================
routing_table_element_t::routing_table_element_t(){
    this->hops = NULL;
    this->hop_count = 0;
};

/// ============================================================================
routing_table_element_t::~routing_table_element_t(){
    utils_t::template_delete_array<uint32_t>(this->hops);
};

/// ============================================================================
/// interconnection_controller_t
/// ============================================================================
interconnection_controller_t::interconnection_controller_t() {
};

/// ============================================================================
interconnection_controller_t::~interconnection_controller_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_matrix<routing_table_element_t>(route_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<edge_t>(adjacency_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<interconnection_interface_t*>(pred, sinuca_engine.get_interconnection_interface_array_size());
};

/// ============================================================================
void interconnection_controller_t::allocate() {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("allocate()\n");
    /// Allocate the router_matrix which will supply the route between elements.
    this->route_matrix = utils_t::template_allocate_matrix<routing_table_element_t>(sinuca_engine.get_interconnection_interface_array_size(), sinuca_engine.get_interconnection_interface_array_size());
    for (uint32_t i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (uint32_t j = i + 1; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            route_matrix[i][j].hops = NULL;
            route_matrix[i][j].hop_count = 0;
        }
    }

    /// Allocate the adjacency_matrix which will have the information about component conexions.
    this->adjacency_matrix = utils_t::template_allocate_matrix<edge_t>(sinuca_engine.get_interconnection_interface_array_size(), sinuca_engine.get_interconnection_interface_array_size());

    this->create_communication_graph();

    /// Choose the algorithm to generate the routing algorithm
    switch (this->get_routing_algorithm()) {
        case ROUTING_ALGORITHM_XY:
            this->routing_algorithm_xy();
        break;

        case ROUTING_ALGORITHM_ODD_EVEN:
            this->routing_algorithm_odd_even();
        break;

        case ROUTING_ALGORITHM_FLOYD_WARSHALL:
            this->routing_algorithm_floyd_warshall();
        break;
    }
};

/// ============================================================================
void interconnection_controller_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    INTERCONNECTION_CTRL_DEBUG_PRINTF("==================== ");
    INTERCONNECTION_CTRL_DEBUG_PRINTF("====================\n");
    INTERCONNECTION_CTRL_DEBUG_PRINTF("cycle() \n");
};

/// ============================================================================
bool interconnection_controller_t::receive_package(memory_package_t *package, uint32_t input_port) {
    ERROR_PRINTF("Received package %s into the input_port %u.\n", package->memory_to_string().c_str(), input_port);
    return FAIL;
};

/// ============================================================================
/// Create a graph using the interconnection components as Cache, Cache Ports, Router
/// ============================================================================
void interconnection_controller_t::create_communication_graph() {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("create_communication_graph()\n");
    uint32_t i, j, jid;
    interconnection_interface_t *obj, *obj2;
    int32_t port;

    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        INTERCONNECTION_CTRL_DEBUG_PRINTF("Interconnection Interface [%u] = Id:%u Label:%s Ports:%u\n", i, sinuca_engine.interconnection_interface_array[i]->get_id(),
                                                                                                        sinuca_engine.interconnection_interface_array[i]->get_label(),
                                                                                                        sinuca_engine.interconnection_interface_array[i]->get_max_ports())
        /// Start the Diagonal, where src == dst
        adjacency_matrix[i][i].weight = 0;
        adjacency_matrix[i][i].src = sinuca_engine.interconnection_interface_array[i];
        adjacency_matrix[i][i].dst = sinuca_engine.interconnection_interface_array[i];
        adjacency_matrix[i][i].src_port = 0;
        adjacency_matrix[i][i].dst_port = 0;
        for (j = i + 1; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            adjacency_matrix[i][j].weight = INFINITE;
            adjacency_matrix[j][i].weight = INFINITE;

            /// Start the upper diagonal
            adjacency_matrix[i][j].src = sinuca_engine.interconnection_interface_array[i];
            adjacency_matrix[i][j].dst = sinuca_engine.interconnection_interface_array[j];
            adjacency_matrix[i][j].src_port = 0;
            adjacency_matrix[i][j].dst_port = 0;

            /// Start the lower diagonal
            adjacency_matrix[j][i].src = sinuca_engine.interconnection_interface_array[j];
            adjacency_matrix[j][i].dst = sinuca_engine.interconnection_interface_array[i];
            adjacency_matrix[j][i].src_port = 0;
            adjacency_matrix[j][i].dst_port = 0;
        }
    }

    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        obj = sinuca_engine.interconnection_interface_array[i];
        for (j = 0; j < obj->get_max_ports(); j++) {
            obj2 = obj->get_interface_output_component(j);
            ERROR_ASSERT_PRINTF(obj2 != NULL, "Component %s at port %u has a pointer to NULL\n", obj->get_label(), j)
            jid = obj2->get_id();
            if (obj->get_type_component() != COMPONENT_PROCESSOR && obj2->get_type_component() != COMPONENT_PROCESSOR) {
                adjacency_matrix[i][jid].weight = 1;
            }
            else {
                adjacency_matrix[i][jid].weight = UNDESIRABLE;
            }
            adjacency_matrix[i][jid].src = obj;
            port = j;
            adjacency_matrix[i][jid].src_port = port;
            adjacency_matrix[i][jid].dst = obj2;
            adjacency_matrix[i][jid].dst_port = obj->get_ports_output_component(j);
            if (adjacency_matrix[i][j].weight != INFINITE && adjacency_matrix[i][j].weight != 0) {
                INTERCONNECTION_CTRL_DEBUG_PRINTF("\t%s[%u]->%s[%u]\n", obj->get_label(), adjacency_matrix[i][j].src_port, adjacency_matrix[i][j].dst->get_label(),  adjacency_matrix[i][j].dst_port);
            }
        }
    }
};

/// ============================================================================
/// Routing Strategies
/// ============================================================================
void interconnection_controller_t::routing_algorithm_floyd_warshall() {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("routing_algorithm_floyd_warshall()\n");
    uint32_t i, j, k;

    this->pred = utils_t::template_allocate_initialize_matrix<interconnection_interface_t*>(sinuca_engine.get_interconnection_interface_array_size(), sinuca_engine.get_interconnection_interface_array_size(), NULL);
    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (adjacency_matrix[i][j].weight != INFINITE && i != j) {
                pred[i][j] = sinuca_engine.interconnection_interface_array[i];
            }
            else {
                pred[i][j] = NULL;
            }
        }
    }

    for (k = 0; k < sinuca_engine.get_interconnection_interface_array_size(); k++) {
        for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
            for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
                if (adjacency_matrix[i][j].weight > adjacency_matrix[i][k].weight + adjacency_matrix[k][j].weight) {
                    adjacency_matrix[i][j].weight = adjacency_matrix[i][k].weight + adjacency_matrix[k][j].weight;
                    pred[i][j] = pred[k][j];
                }
            }
        }
    }

    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (i != j) {
                this->create_route(sinuca_engine.interconnection_interface_array[i], sinuca_engine.interconnection_interface_array[j]);
            }
        }
    }

#ifdef INTERCONNECTION_CTRL_DEBUG
    INTERCONNECTION_CTRL_DEBUG_PRINTF("-------------------------------------------------------------\n");
    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (i != j) {
                INTERCONNECTION_CTRL_DEBUG_PRINTF("%s to %s:\n", sinuca_engine.interconnection_interface_array[i]->get_label(), sinuca_engine.interconnection_interface_array[j]->get_label());
                for (k = 0; k <= route_matrix[i][j].hop_count; k++) {
                    INTERCONNECTION_CTRL_DEBUG_PRINTF("\t[%d]\n", route_matrix[i][j].hops[k])
                }
            }
        }
    }
#endif
};

/// ============================================================================
/// Routing Policy: routingXY
void interconnection_controller_t::routing_algorithm_xy() {
    ERROR_PRINTF("Routing policy not implemented.\n");
};

/// ============================================================================
/// Routing Policy: ROUTING_ODD_EVEN:
void interconnection_controller_t::routing_algorithm_odd_even() {
    ERROR_PRINTF("Routing policy not implemented.\n");
};

/// ============================================================================
void interconnection_controller_t::create_route(interconnection_interface_t *src, interconnection_interface_t *dst) {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("create_route()\n");
    uint32_t found = 0, count = 0;
    interconnection_interface_t *real_src = src, *real_dst = dst;
    interconnection_interface_t *old_dst = dst;

    INTERCONNECTION_CTRL_DEBUG_PRINTF("%s to %s\n", src->get_label(), dst->get_label());

    do {
        count++;
        if (pred[src->get_id()][dst->get_id()] == src) {
            found = 1;
        }
        dst = pred[src->get_id()][dst->get_id()];
    } while (!found);

    INTERCONNECTION_CTRL_DEBUG_PRINTF("\tHops=%d\n", count);

    found = 0;
    dst = src;
    src = old_dst;
    old_dst = dst;

    /// Position in the hops[] != from max_hops
    route_matrix[real_src->get_id()][real_dst->get_id()].hop_count = count - 1;
    route_matrix[real_src->get_id()][real_dst->get_id()].hops = utils_t::template_allocate_initialize_array<uint32_t>(count, 0);

    do {
        count--;
        if (pred[src->get_id()][dst->get_id()] == src) {
            found = 1;
        }
            old_dst = dst;
            dst = pred[src->get_id()][dst->get_id()];


        INTERCONNECTION_CTRL_DEBUG_PRINTF("\t\t%s[%u]<->%s[%u]\n", old_dst->get_label(), adjacency_matrix[old_dst->get_id()][dst->get_id()].src_port, dst->get_label(),  adjacency_matrix[old_dst->get_id()][dst->get_id()].dst_port);
        route_matrix[real_src->get_id()][real_dst->get_id()].hops[count] = adjacency_matrix[old_dst->get_id()][dst->get_id()].src_port;
    } while (!found);
};

/// ============================================================================
void interconnection_controller_t::find_package_route(memory_package_t *package) {
    uint32_t src = package->id_src;
    uint32_t dst = package->id_dst;

    INTERCONNECTION_CTRL_DEBUG_PRINTF("find_package_route()\n");
    ERROR_ASSERT_PRINTF(src != dst, "find_package_route received src == dst.\n")
    ERROR_ASSERT_PRINTF(src < sinuca_engine.interconnection_interface_array_size &&
                        dst < sinuca_engine.interconnection_interface_array_size, "find_package_route received wrong src:%u or dst:%u.\n", src, dst)
    package->hop_count = route_matrix[src][dst].hop_count;
    package->hops = route_matrix[src][dst].hops;
    ERROR_ASSERT_PRINTF(package->hops != NULL, "find_package_route will return a NULL route.\n")

    #ifdef INTERCONNECTION_CTRL_DEBUG
        INTERCONNECTION_CTRL_DEBUG_PRINTF("RETURNING HOPS: %d  ROUTE:\n", package->hop_count);
        for (int32_t k = 0; k <= package->hop_count; k++) {
            INTERCONNECTION_CTRL_DEBUG_PRINTF("\t[%d]\n", package->hops[k]);
        }
    #endif
};

/// ============================================================================
uint32_t interconnection_controller_t::find_package_route_latency(memory_package_t *package) {
    switch (package->memory_operation) {
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_PREFETCH:
            if (package->is_answer) {   /// BIG
                return (package->memory_size / 8);
            }
            else {  /// SMALL
                return 1;
            }
        break;

        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_COPYBACK:
            if (package->is_answer) {   /// SMALL
                return 1;
            }
            else {  /// BIG
                return (package->memory_size / 8);
            }
        break;
    }
    ERROR_PRINTF("Found MEMORY_OPERATION_NUMBER\n");
    return 1;
};

/// ============================================================================
void interconnection_controller_t::print_structures() {

};

/// ============================================================================
void interconnection_controller_t::panic() {
    this->print_structures();
};

/// ============================================================================
void interconnection_controller_t::periodic_check(){
    #ifdef INTERCONNECTION_CTRL_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
void interconnection_controller_t::reset_statistics() {
};

/// ============================================================================
void interconnection_controller_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};

/// ============================================================================
void interconnection_controller_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};
