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

// ============================================================================
/// Interconnection Interface
// ============================================================================
 /*! Generic class which creates the II (Interconnection Interface).
  * Network Routers to Processors, Cache Memories, and Main Memory.
  * The II will exist on almost every component in order to interface the components
  */
class interconnection_interface_t {
    protected:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t id;                    /// ID unique which defines the object.
        component_t type_component;     /// ID unique which defines the object.
        char label[500];                /// Comprehensive object's name.
        uint32_t max_ports;
        uint32_t used_ports;
        uint32_t interconnection_width;
        uint32_t interconnection_latency;
        container_token_t token_list;

        // ====================================================================
        /// Set by this->set_higher_lower_level_component()
        // ====================================================================
        interconnection_interface_t* *interface_output_components;
        uint32_t *ports_output_components;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        interconnection_interface_t();
        virtual ~interconnection_interface_t();

        inline const char* get_type_component_label() {
            return get_enum_component_char(this->get_type_component());
        }

        inline const char* get_label() {
            return this->label;
        }
        inline void set_label(const char *str) {
            ERROR_ASSERT_PRINTF(strlen(str) <= sizeof(label), "Label too big (>%u chars).\n", (uint32_t)strlen(str))
            strcpy(label, str);
        }

        inline interconnection_interface_t* get_interface_output_component(uint32_t position) {
            return interface_output_components[position];
        }

        inline uint32_t get_ports_output_component(uint32_t position) {
            return ports_output_components[position];
        }

        void allocate_base();
        static void set_higher_lower_level_component(interconnection_interface_t *interface_A, uint32_t port_A, interconnection_interface_t *interface_B, uint32_t port_B);
        int32_t find_port_to_obj(interconnection_interface_t *obj);  /// returns -1 if does not find
        int32_t find_port_from_obj(interconnection_interface_t *obj);  /// returns -1 if does not find

        INSTANTIATE_GET_SET(uint32_t, id)
        INSTANTIATE_GET_SET(component_t, type_component)
        INSTANTIATE_GET_SET(uint32_t, max_ports)
        INSTANTIATE_GET_SET_ADD(uint32_t, used_ports)
        INSTANTIATE_GET_SET(uint32_t, interconnection_width)
        INSTANTIATE_GET_SET(uint32_t, interconnection_latency)

        // ====================================================================
        /// Inheritance
        // ====================================================================
        /// Basic Methods
        virtual void allocate() = 0;  /// Called after all the parameters are set
        virtual void clock(uint32_t sub_cycle) = 0;   /// Called every cycle
        virtual int32_t send_package(memory_package_t *package) = 0;    /// Find the route between Sender and Receiver and return -1 (fail) or lantency (sent)
        virtual bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) = 0; /// Only Sender calculate the latency and pass to the receiver.

        /// Token Controller Methods
        virtual bool check_token_list(memory_package_t *package) = 0;       /// Check for available position to receive a package and allocate a token
        virtual void remove_token_list(memory_package_t *package) = 0;      /// After arrive the package remove from token list

        /// Debug Methods
        virtual void periodic_check() = 0;    /// Check all the internal structures
        virtual void print_structures() = 0;  /// Print the internal structures
        virtual void panic() = 0;             /// Called when some ERROR happens

        /// Statistics Methods
        virtual void reset_statistics()=0;      /// Reset all internal statistics variables
        virtual void print_statistics()=0;      /// Print out the internal statistics variables
        virtual void print_configuration()=0;   /// Print out the internal configuration variables
};
