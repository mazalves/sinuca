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

#ifdef INTERCONNECTION_CTRL_DEBUG
    #define INTERCONNECTION_CTRL_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define INTERCONNECTION_CTRL_DEBUG_PRINTF(...)
#endif

// ============================================================================
/// routing_table_element_t
// ============================================================================
routing_table_element_t::routing_table_element_t(){
    this->hops = NULL;
    this->hop_count = 0;
};

// ============================================================================
routing_table_element_t::~routing_table_element_t(){
    utils_t::template_delete_array<uint32_t>(this->hops);
};

// ============================================================================
/// interconnection_controller_t
// ============================================================================
interconnection_controller_t::interconnection_controller_t() {
    this->routing_algorithm = ROUTING_ALGORITHM_FLOYD_WARSHALL;

    this->predecessor = NULL;
    this->adjacency_matrix = NULL;
    this->route_matrix = NULL;

    this->high_latency_matrix = NULL;
    this->low_latency_matrix = NULL;

    this->total_high_latency_matrix = NULL;
    this->total_low_latency_matrix = NULL;
};

// ============================================================================
interconnection_controller_t::~interconnection_controller_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_matrix<routing_table_element_t>(route_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<edge_t>(adjacency_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<interconnection_interface_t*>(predecessor, sinuca_engine.get_interconnection_interface_array_size());

    utils_t::template_delete_matrix<int32_t>(high_latency_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<int32_t>(low_latency_matrix, sinuca_engine.get_interconnection_interface_array_size());

    utils_t::template_delete_matrix<int32_t>(total_high_latency_matrix, sinuca_engine.get_interconnection_interface_array_size());
    utils_t::template_delete_matrix<int32_t>(total_low_latency_matrix, sinuca_engine.get_interconnection_interface_array_size());
};

// ============================================================================
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
    this->adjacency_matrix = utils_t::template_allocate_matrix<edge_t>(sinuca_engine.get_interconnection_interface_array_size(),
                                                                        sinuca_engine.get_interconnection_interface_array_size());

    this->predecessor = utils_t::template_allocate_initialize_matrix<interconnection_interface_t*>(
                                                    sinuca_engine.get_interconnection_interface_array_size(),
                                                    sinuca_engine.get_interconnection_interface_array_size(), NULL);

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

    /// Latency table
    this->high_latency_matrix = utils_t::template_allocate_initialize_matrix<int32_t>(sinuca_engine.get_interconnection_interface_array_size(),
                                                                                sinuca_engine.get_interconnection_interface_array_size(), -1);
    this->low_latency_matrix = utils_t::template_allocate_initialize_matrix<int32_t>(sinuca_engine.get_interconnection_interface_array_size(),
                                                                                sinuca_engine.get_interconnection_interface_array_size(), -1);

    this->total_high_latency_matrix = utils_t::template_allocate_initialize_matrix<int32_t>(sinuca_engine.get_interconnection_interface_array_size(),
                                                                                sinuca_engine.get_interconnection_interface_array_size(), -1);
    this->total_low_latency_matrix = utils_t::template_allocate_initialize_matrix<int32_t>(sinuca_engine.get_interconnection_interface_array_size(),
                                                                                sinuca_engine.get_interconnection_interface_array_size(), -1);

    this->create_communication_cost();
};

// ============================================================================
void interconnection_controller_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    INTERCONNECTION_CTRL_DEBUG_PRINTF("==================== ");
    INTERCONNECTION_CTRL_DEBUG_PRINTF("====================\n");
    INTERCONNECTION_CTRL_DEBUG_PRINTF("cycle() \n");
};


// ============================================================================
int32_t interconnection_controller_t::send_package(memory_package_t *package) {
    ERROR_PRINTF("Send package %s.\n", package->content_to_string().c_str());
    return POSITION_FAIL;
};

// ============================================================================
bool interconnection_controller_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {
    ERROR_PRINTF("Received package %s into the input_port %u, latency %u.\n", package->content_to_string().c_str(), input_port, transmission_latency);
    return FAIL;
};

// ============================================================================
/// Token Controller Methods
// ============================================================================
bool interconnection_controller_t::check_token_list(memory_package_t *package) {
    ERROR_PRINTF("check_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};

// ============================================================================
void interconnection_controller_t::remove_token_list(memory_package_t *package) {
    ERROR_PRINTF("remove_token_list %s.\n", get_enum_memory_operation_char(package->memory_operation))
};



// ============================================================================
/// Create a graph using the interconnection components as Cache, Cache Ports, Router
// ============================================================================
void interconnection_controller_t::create_communication_graph() {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("create_communication_graph()\n");
    uint32_t i, j, jid;
    interconnection_interface_t *obj, *obj2;
    int32_t port;

    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        INTERCONNECTION_CTRL_DEBUG_PRINTF("Interconnection Interface [%u] = Id:%u Label:%s Ports:%u\n",
                                        i, sinuca_engine.interconnection_interface_array[i]->get_id(),
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

// ============================================================================
/// Routing Strategies
// ============================================================================
void interconnection_controller_t::routing_algorithm_floyd_warshall() {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("routing_algorithm_floyd_warshall()\n");
    uint32_t i, j, k;

    for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (adjacency_matrix[i][j].weight != INFINITE && i != j) {
                this->predecessor[i][j] = sinuca_engine.interconnection_interface_array[i];
            }
            else {
                this->predecessor[i][j] = NULL;
            }
        }
    }

    for (k = 0; k < sinuca_engine.get_interconnection_interface_array_size(); k++) {
        for (i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
            for (j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
                if (adjacency_matrix[i][j].weight > adjacency_matrix[i][k].weight + adjacency_matrix[k][j].weight) {
                    adjacency_matrix[i][j].weight = adjacency_matrix[i][k].weight + adjacency_matrix[k][j].weight;
                    this->predecessor[i][j] = predecessor[k][j];
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
                printf("\t%s -> %s\n", sinuca_engine.interconnection_interface_array[i]->get_label(), sinuca_engine.interconnection_interface_array[j]->get_label());
                for (k = 0; k <= route_matrix[i][j].hop_count; k++) {
                    INTERCONNECTION_CTRL_DEBUG_PRINTF("\t[%d]\n", route_matrix[i][j].hops[k])
                }
            }
        }
    }
#endif
};

// ============================================================================
/// Routing Policy: routingXY
void interconnection_controller_t::routing_algorithm_xy() {
    ERROR_PRINTF("Routing policy not implemented.\n");
};

// ============================================================================
/// Routing Policy: ROUTING_ODD_EVEN:
void interconnection_controller_t::routing_algorithm_odd_even() {
    ERROR_PRINTF("Routing policy not implemented.\n");
};

// ============================================================================
void interconnection_controller_t::create_route(interconnection_interface_t *src, interconnection_interface_t *dst) {
    INTERCONNECTION_CTRL_DEBUG_PRINTF("create_route()\n");
    uint32_t found = 0, count = 0;
    interconnection_interface_t *real_src = src, *real_dst = dst;
    interconnection_interface_t *old_dst = dst;

    INTERCONNECTION_CTRL_DEBUG_PRINTF("%s to %s\n", src->get_label(), dst->get_label());

    do {
        count++;
        if (predecessor[src->get_id()][dst->get_id()] == src) {
            found = 1;
        }
        dst = predecessor[src->get_id()][dst->get_id()];
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
        if (predecessor[src->get_id()][dst->get_id()] == src) {
            found = 1;
        }

        old_dst = dst;
        dst = predecessor[src->get_id()][dst->get_id()];

        INTERCONNECTION_CTRL_DEBUG_PRINTF("\t\t%s[%u]<->%s[%u]\n", old_dst->get_label(), adjacency_matrix[old_dst->get_id()][dst->get_id()].src_port, dst->get_label(),  adjacency_matrix[old_dst->get_id()][dst->get_id()].dst_port);
        route_matrix[real_src->get_id()][real_dst->get_id()].hops[count] = adjacency_matrix[old_dst->get_id()][dst->get_id()].src_port;
    } while (!found);
};

// ============================================================================
void interconnection_controller_t::create_communication_cost() {
    uint32_t max_latency = 0, min_latency = 0;
    uint32_t min_width = 0;
    /// Generate the maximum (full line) and minimum (request only) latency between two ADJACENT components
    for (uint32_t i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (uint32_t j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (i != j && route_matrix[i][j].hop_count == 0) {
                /// Find the max latency between two adjacents components
                max_latency = sinuca_engine.interconnection_interface_array[i]->get_interconnection_latency();
                if (sinuca_engine.interconnection_interface_array[j]->get_interconnection_latency() > max_latency) {
                    max_latency = sinuca_engine.interconnection_interface_array[j]->get_interconnection_latency();
                }

                /// Find the min width between two adjacents components
                min_width = sinuca_engine.interconnection_interface_array[i]->get_interconnection_width();
                if (sinuca_engine.interconnection_interface_array[j]->get_interconnection_width() < min_width) {
                    min_width = sinuca_engine.interconnection_interface_array[j]->get_interconnection_width();
                }

                /// Set the high_latency (full package / answer)
                this->high_latency_matrix[i][j] = max_latency * ((sinuca_engine.global_line_size / min_width) + 1 * ((sinuca_engine.global_line_size % min_width) != 0));

                /// Set the low_latency (empty package / request)
                this->low_latency_matrix[i][j] = max_latency;
            }
        }
    }

    /// Generate a matrix with maximum latency between ANY two components
    for (uint32_t i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (uint32_t j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (i == j || route_matrix[i][j].hop_count == 0 || route_matrix[i][j].hops == NULL) {
                this->total_high_latency_matrix[i][j] = this->high_latency_matrix[i][j];
                this->total_low_latency_matrix[i][j] = this->low_latency_matrix[i][j];
            }
            else {
                max_latency = 0;
                min_latency = 0;

                uint32_t actual_id = 0, next_id = 0;
                interconnection_interface_t *actual = NULL;
                actual = sinuca_engine.interconnection_interface_array[i];

                for (int32_t k = route_matrix[i][j].hop_count; k >= 0; k--) {
                    actual_id = actual->get_id();
                    actual = actual->get_interface_output_component(route_matrix[i][j].hops[k]);
                    next_id = actual->get_id();

                    max_latency += this->high_latency_matrix[actual_id][next_id];
                    min_latency += this->low_latency_matrix[actual_id][next_id];
                }
                this->total_high_latency_matrix[i][j] = max_latency;
                this->total_low_latency_matrix[i][j] = min_latency;
            }
        }
    }
};

// ============================================================================
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


// ============================================================================
uint32_t interconnection_controller_t::find_package_route_latency(memory_package_t *package, interconnection_interface_t *src, interconnection_interface_t *dst){
    /// The transmission latency is defined as 1 for requests, and line_size for answers
    int32_t max_latency = this->high_latency_matrix[src->get_id()][dst->get_id()];
    int32_t low_latency = this->low_latency_matrix[src->get_id()][dst->get_id()];

    ERROR_ASSERT_PRINTF(max_latency != -1, "Transmiting between not adjacent components,  max_latency = -1\n");
    ERROR_ASSERT_PRINTF(low_latency != -1, "Transmiting between not adjacent components,  low_latency = -1\n");

    switch (package->memory_operation) {
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_PREFETCH:
            /// BIG
            if (package->is_answer) {
                return max_latency;
            }
            /// SMALL
            else {
                return low_latency;
            }
        break;

        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_WRITEBACK:
            /// SMALL
            if (package->is_answer) {
                return low_latency;
            }
            /// BIG
            else {
                return max_latency;
            }
        break;
    }
    ERROR_PRINTF("Found MEMORY_OPERATION_NUMBER\n");
    return 1;
};

// ============================================================================
void interconnection_controller_t::print_structures() {

};

// ============================================================================
void interconnection_controller_t::panic() {
    this->print_structures();
};

// ============================================================================
void interconnection_controller_t::periodic_check(){
    #ifdef INTERCONNECTION_CTRL_DEBUG
        this->print_structures();
    #endif
};

// ============================================================================
void interconnection_controller_t::reset_statistics() {
};

// ============================================================================
void interconnection_controller_t::print_statistics() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};

// ============================================================================
void interconnection_controller_t::print_configuration() {
    char title[100] = "";
    snprintf(title, sizeof(title), "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();
};

// ============================================================================
void interconnection_controller_t::print_graph() {
    char graph_line[100] = "";
    snprintf(graph_line, sizeof(graph_line), "## Plot the graph of connection ready to Graphviz");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "## GV = {dot, neato, smyrna, lefty, dotty}\n");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "## $ dot -Tpdf -o graph.pdf graph.txt\n\n");
    sinuca_engine.write_graph(graph_line);


    snprintf(graph_line, sizeof(graph_line), "digraph G\n");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "  {\n");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "  node [shape=box,style=filled];\n");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "  overlap=scale;\n");
    sinuca_engine.write_graph(graph_line);
    snprintf(graph_line, sizeof(graph_line), "  splines=true;\n");
    sinuca_engine.write_graph(graph_line);

    for (uint32_t i = 0; i < sinuca_engine.get_interconnection_interface_array_size(); i++) {
        for (uint32_t j = 0; j < sinuca_engine.get_interconnection_interface_array_size(); j++) {
            if (i != j && route_matrix[i][j].hop_count == 0) {
                snprintf(graph_line, sizeof(graph_line), "  %s -> %s;", sinuca_engine.interconnection_interface_array[i]->get_label(),
                                                    sinuca_engine.interconnection_interface_array[j]->get_label());
                sinuca_engine.write_graph(graph_line);
            }
        }
        snprintf(graph_line, sizeof(graph_line), "\n");
        sinuca_engine.write_graph(graph_line);
    }

    snprintf(graph_line, sizeof(graph_line), "}\n");
    sinuca_engine.write_graph(graph_line);
};

