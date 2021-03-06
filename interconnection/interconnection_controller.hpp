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

class interconnection_controller_t : public interconnection_interface_t {
    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        routing_algorithm_t routing_algorithm;

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        /// Routing table sized[max_id][max_id] of white packages, with *hops and hop_count only;
        interconnection_interface_t ***predecessor;
        edge_t **adjacency_matrix;
        routing_table_element_t **route_matrix;

        int32_t **high_latency_matrix;
        int32_t **low_latency_matrix;
        int32_t **total_high_latency_matrix;
        int32_t **total_low_latency_matrix;


    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        interconnection_controller_t();
        ~interconnection_controller_t();
        inline const char* get_label() {
            return "INTERCONNECTION_CTRL";
        };
        inline const char* get_type_component_label() {
            return "INTERCONNECTION_CTRL";
        };

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        bool pop_token_credit(uint32_t src_id, memory_operation_t memory_operation);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        // ====================================================================
        void print_graph();

        void create_route(interconnection_interface_t *src, interconnection_interface_t *dst);

        void find_package_route(memory_package_t *package);
        uint32_t find_package_route_latency(memory_package_t *package, interconnection_interface_t *src, interconnection_interface_t *dst);

        inline int32_t get_total_high_latency(uint32_t src, uint32_t dst) {
            ERROR_ASSERT_PRINTF(src < sinuca_engine.interconnection_interface_array_size &&
                                dst < sinuca_engine.interconnection_interface_array_size,
                                "get_total_high_latency received wrong src:%u or dst:%u.\n", src, dst)

            return this->total_high_latency_matrix[src][dst];
        }

        inline int32_t get_total_low_latency(uint32_t src, uint32_t dst) {
            ERROR_ASSERT_PRINTF(src < sinuca_engine.interconnection_interface_array_size &&
                                dst < sinuca_engine.interconnection_interface_array_size,
                                "get_total_low_latency received wrong src:%u or dst:%u.\n", src, dst)

            return this->total_low_latency_matrix[src][dst];
        }

        void create_communication_graph();
        void create_communication_cost();
        /// routing algorithms
        void routing_algorithm_floyd_warshall();  /// Create the routing table with floy-wharsall routing algorithm
        void routing_algorithm_xy();  /// Create the routing table for mesh NoCs with XY routing
        void routing_algorithm_odd_even();  /// Create the routing table for mesh NoCs with OddEven routing



        INSTANTIATE_GET_SET(routing_algorithm_t, routing_algorithm)
};
