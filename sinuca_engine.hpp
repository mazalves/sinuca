//==============================================================================
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
//==============================================================================
/// ============================================================================
/// Configuration File Control
/// ============================================================================
class sinuca_engine_t {
    public:
        /// Program input
        char *arg_configuration_file_name;
        char *arg_configuration_path;
        char *arg_trace_file_name;
        char *arg_result_file_name;
        uint32_t arg_warmup_instructions;
        bool arg_is_compressed;
        char *arg_graph_file_name;

        std::ofstream result_file;
        std::ofstream graph_file;

        /// Array of components
        interconnection_interface_t* *interconnection_interface_array;
        processor_t* *processor_array;
        cache_memory_t* *cache_memory_array;
        memory_controller_t* *memory_controller_array;
        interconnection_router_t* *interconnection_router_array;

        uint32_t interconnection_interface_array_size;
        uint32_t processor_array_size;
        uint32_t cache_memory_array_size;
        uint32_t memory_controller_array_size;
        uint32_t interconnection_router_array_size;

		uint32_t round_robin_processor;
		uint32_t round_robin_cache_memory;
		uint32_t round_robin_memory_controller;
		uint32_t round_robin_interconnection_router;

        /// Control the Global Cycle
        uint64_t global_cycle;
        /// Control the line size for all components
        uint32_t global_line_size;
        uint64_t global_offset_bits_mask;
        uint64_t global_not_offset_bits_mask;
        /// Control if all the allocation is ready
        bool is_simulation_allocated;
        /// Control for Run Time Debug
        bool is_runtime_debug;

        /// Control the Trace Reading
        bool *is_processor_trace_eof;
        bool is_simulation_eof;
        bool is_warm_up;

        trace_reader_t *trace_reader;
        directory_controller_t *directory_controller;
        interconnection_controller_t *interconnection_controller;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        /// Time statistics
        struct timeval stat_timer_start, stat_timer_end;
        /// Memory Usage statistics
        /// virtual memory
        double stat_vm_start;
        double stat_vm_allocate;
        double stat_vm_end;
        double stat_vm_max;
        /// resident set size
        double stat_rss_start;
        double stat_rss_allocate;
        double stat_rss_end;
        double stat_rss_max;
        /// Save during warmup
        uint64_t reset_cycle;
        /// Oldest Packages
        uint64_t stat_old_memory_package;
        uint64_t stat_old_opcode_package;
        uint64_t stat_old_uop_package;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        sinuca_engine_t();
        ~sinuca_engine_t();
        inline const char* get_label() {
            return "SINUCA_ENGINE";
        };
        inline const char* get_type_component_label() {
            return "SINUCA_ENGINE";
        };

        /// Public Methods
        void premature_termination();
        bool alive();
        void set_is_processor_trace_eof(uint32_t cpuid);

        void initialize();
        void initialize_processor();
        void initialize_cache_memory();
        void initialize_memory_controller();
        void initialize_interconnection_router();

        void initialize_directory_controller();
        void initialize_interconnection_controller();

        void make_connections();

        void global_panic();
        void global_periodic_check();
        void global_clock();

        INSTANTIATE_GET_SET(interconnection_interface_t**, interconnection_interface_array);
        INSTANTIATE_GET_SET(processor_t**, processor_array);
        INSTANTIATE_GET_SET(cache_memory_t**, cache_memory_array);
        INSTANTIATE_GET_SET(memory_controller_t**, memory_controller_array);
        INSTANTIATE_GET_SET(interconnection_router_t**, interconnection_router_array);

        INSTANTIATE_GET_SET(uint32_t, interconnection_interface_array_size);
        INSTANTIATE_GET_SET(uint32_t, processor_array_size);
        INSTANTIATE_GET_SET(uint32_t, cache_memory_array_size);
        INSTANTIATE_GET_SET(uint32_t, memory_controller_array_size);
        INSTANTIATE_GET_SET(uint32_t, interconnection_router_array_size);

		INSTANTIATE_GET_SET(uint32_t, round_robin_processor);
		INSTANTIATE_GET_SET(uint32_t, round_robin_cache_memory);
		INSTANTIATE_GET_SET(uint32_t, round_robin_memory_controller);
		INSTANTIATE_GET_SET(uint32_t, round_robin_interconnection_router);

        INSTANTIATE_GET_SET(bool, is_simulation_allocated);
        INSTANTIATE_GET_SET(bool, is_runtime_debug);
        INSTANTIATE_GET_SET(bool, is_simulation_eof);
        INSTANTIATE_GET_SET(bool, is_warm_up);

        INSTANTIATE_GET_SET(uint64_t, global_cycle);

        void set_global_line_size(uint32_t new_size);   /// Define one global_line_size and global_offset_bits_mask
        uint32_t get_global_line_size();

        INSTANTIATE_GET_SET(uint64_t, global_offset_bits_mask);
        INSTANTIATE_GET_SET(uint64_t, global_not_offset_bits_mask);

        inline bool global_cmp_tag_index_bank(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->global_not_offset_bits_mask) == (memory_addressB & this->global_not_offset_bits_mask);
        }


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        void write_graph(const char *buffer);

        void write_statistics(const char *buffer);

        void write_statistics_small_separator();
        void write_statistics_big_separator();

        void write_statistics_comments(const char *comment);

        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, const char *value);
        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, bool value);
        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, uint32_t value);
        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value);
        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, float value);
        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, double value);

        void write_statistics_value_percentage(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value, uint64_t total);
        void write_statistics_value_ratio(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value, uint64_t total);
        void write_statistics_value_ratio(const char *obj_type, const char *obj_label, const char *variable_name, double value, uint64_t total);

        void global_reset_statistics();
        void global_print_statistics();
        void global_print_configuration();
        void global_print_graph();

        /// Memory Usage statistics
        /// virtual memory
        INSTANTIATE_GET_SET(double, stat_vm_start);
        INSTANTIATE_GET_SET(double, stat_vm_allocate);
        INSTANTIATE_GET_SET(double, stat_vm_end);
        INSTANTIATE_GET_SET(double, stat_vm_max);
        /// resident set size
        INSTANTIATE_GET_SET(double, stat_rss_start);
        INSTANTIATE_GET_SET(double, stat_rss_allocate);
        INSTANTIATE_GET_SET(double, stat_rss_end);
        INSTANTIATE_GET_SET(double, stat_rss_max);
        /// Save during warmup
        INSTANTIATE_GET_SET(uint64_t, reset_cycle);
        /// Oldest Packages
        INSTANTIATE_GET_SET(uint64_t, stat_old_memory_package);
        INSTANTIATE_GET_SET(uint64_t, stat_old_opcode_package);
        INSTANTIATE_GET_SET(uint64_t, stat_old_uop_package);
};

