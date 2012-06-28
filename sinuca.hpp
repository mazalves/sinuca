/// ============================================================================
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
/// ============================================================================
/// Main header file: sinuca.h
/// This is the main header file, it contains all the prototypes and
/// describes the relations between classes.
#ifndef _SINUCA_SINUCA_HPP_
#define _SINUCA_SINUCA_HPP_

/// Define the MAX SIZE of each type
#define __STDC_LIMIT_MACROS
/// Define macros to use uint_64 for printf (PRIu64) and scanf (SCNu64)
#define __STDC_FORMAT_MACROS

/// C Includes
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdint.h>
#include <signal.h>
/// C++ Includes
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <deque>
#include <vector>
#include <string>


/// Embedded Libraries
#include "./libs/include/zlib.h"
#include "./libs/include/libconfig.h++"
/// Sinuca Related
#include "./enumerations.hpp"

/// ============================================================================
/// Classes
/// ============================================================================
class trace_reader_t;
class sinuca_engine_t;
/// Packages
class opcode_package_t;
class uop_package_t;
class memory_package_t;
/// Interconnection
class interconnection_interface_t;
class interconnection_controller_t;
class interconnection_router_t;
/// Processor
class branch_predictor_t;
class reorder_buffer_line_t;
class processor_t;
/// Directory
class directory_controller_line_t;
class directory_controller_t;
/// Cache and Main Memory
class prefetch_t;
class line_usage_predictor_t;
class cache_line_t;
class cache_set_t;
class cache_memory_t;
class main_memory_t;
/// Useful static methods
class utils_t;

/// ============================================================================
/// Global SINUCA_ENGINE instantiation
/// ============================================================================
extern sinuca_engine_t sinuca_engine;

/// ============================================================================
/// Definitions for Log, Debug, Warning, Error and Statistics
/// ============================================================================
#define HEART_BEAT      1000000  /// Period to inform the Progress
#define MAX_ALIVE_TIME   100000  /// Max Time for a request to be solved
#define PERIODIC_CHECK   100000  /// Period between the Periodic Check

#define INITIALIZE_DEBUG 0//200000    /// Cycle to start the DEBUG_PRINTF (0 to disable)
#define FINALIZE_DEBUG 0      /// Cycle to stop the DEBUG_PRINTF (0 to disable)

/// Class specific definitions
#define INFINITE (std::numeric_limits<uint32_t>::max())             /// interconnection_controller_t
#define UNDESIRABLE (sinuca_engine.get_processor_array_size() * 10) /// interconnection_controller_t
#define TRACE_LINE_SIZE 200     /// trace_reader_t (should not be smaller than on trace line)
#define MAX_UOP_DECODED 5       /// processor_t (Max number of uops from one opcode)

#define POSITION_FAIL -1        /// FAIL when return is int32_t
#define FAIL 0                  /// FAIL when return is uint32_t
#define OK 1                    /// OK when return is int32_t or uint32_t

/// DETAIL DESCRIPTION: Almost all errors and messages use this definition.
/// It will DEACTIVATE all the other messages below
#define SINUCA_PRINTF(...) printf(__VA_ARGS__);

/// DEBUG DESCRIPTION: Lots of details do help during the debug phase.
// ~ #define SINUCA_DEBUG
#ifdef SINUCA_DEBUG
    // ~ #define CONFIGURATOR_DEBUG
    // ~ #define TRACE_READER_DEBUG
    // ~ #define TRACE_GENERATOR_DEBUG
    // ~ #define PROCESSOR_DEBUG
    // ~ #define SYNC_DEBUG
    // ~ #define BRANCH_PREDICTOR_DEBUG
    // ~ #define CACHE_DEBUG
    // ~ #define PREFETCHER_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG
    // ~ #define MAIN_MEMORY_DEBUG
    // ~ #define ROUTER_DEBUG
    // ~ #define INTERCONNECTION_CTRL_DEBUG
    #define DIRECTORY_CTRL_DEBUG
    // ~ #define SHOW_FREE_PACKAGE
    #define DEBUG_PRINTF(...)           {\
                                            if (sinuca_engine.get_is_runtime_debug()) {\
                                                SINUCA_PRINTF("DEBUG %s: ", get_label());\
                                                SINUCA_PRINTF(__VA_ARGS__);\
                                            }\
                                        }
#else
    #define DEBUG_PRINTF(...)
#endif

/// WARNING DESCRIPTION: Non Critical Messages, most are TODO
#define SINUCA_WARNING
#ifdef SINUCA_WARNING
    #define WARNING_PRINTF(...)         {\
                                            SINUCA_PRINTF("WARNING: ");\
                                            SINUCA_PRINTF(__VA_ARGS__);\
                                        }
#else
    #define WARNING_PRINTF(...)
#endif

/// ERROR DESCRIPTION: Unstable state, sinuca_global_panic() and exit(1).

#define ERROR
#ifdef ERROR
    #define ERROR_INFORMATION()         {\
                                            sinuca_engine.global_panic();\
                                            SINUCA_PRINTF("ERROR INFORMATION\n");\
                                            SINUCA_PRINTF("ERROR: File: %s at Line: %u\n", __FILE__, __LINE__);\
                                            SINUCA_PRINTF("ERROR: Function: %s\n", __PRETTY_FUNCTION__);\
                                            SINUCA_PRINTF("ERROR: Cycle: %"PRIu64"\n", sinuca_engine.get_global_cycle());\
                                        }

    #define ERROR_ASSERT_PRINTF(v, ...) if (!(v)) {\
                                            ERROR_INFORMATION();\
                                            SINUCA_PRINTF("ERROR_ASSERT: %s\n", #v);\
                                            SINUCA_PRINTF("ERROR: ");\
                                            SINUCA_PRINTF(__VA_ARGS__);\
                                            exit(EXIT_FAILURE);\
                                        }

    #define ERROR_PRINTF(...)           {\
                                            ERROR_INFORMATION();\
                                            SINUCA_PRINTF("ERROR: ");\
                                            SINUCA_PRINTF(__VA_ARGS__);\
                                            exit(EXIT_FAILURE);\
                                        }
#else
    #define ERROR_INFORMATION(...)
    #define ERROR_ASSERT_PRINTF(v, ...)
    #define ERROR_PRINTF(...)
#endif

/// ============================================================================
/// MACROS to create get_ and set_ methods for variables.
/// ============================================================================
/// Creates the Get, Set methods automatically.
#define INSTANTIATE_GET_SET(TYPE, NAME) \
    inline void set_ ## NAME(TYPE input_NAME) {\
        this->NAME = input_NAME;\
    };\
    inline TYPE get_ ## NAME() {\
        return this->NAME;\
    };

/// Creates the Get, Set and Add methods automatically (useful to statistics).
#define INSTANTIATE_GET_SET_ADD(TYPE, NAME) \
    inline void add_ ## NAME() {\
        this->NAME++;\
    };\
    inline void set_ ## NAME(TYPE input_NAME) {\
        this->NAME = input_NAME;\
    };\
    inline TYPE get_ ## NAME() {\
        return this->NAME;\
    };

/// ============================================================================
/// Structures
/// ============================================================================
typedef std::vector <uint32_t>                      container_register_t;
typedef std::vector <opcode_package_t>              container_opcode_package_t;
typedef std::vector <container_opcode_package_t>    container_static_dictionary_t;

typedef std::vector <reorder_buffer_line_t*>        container_ptr_reorder_buffer_line_t;
typedef std::vector <directory_controller_line_t*>  container_ptr_directory_controller_line_t;
typedef std::vector <cache_memory_t*>               container_ptr_cache_memory_t;
typedef std::vector <main_memory_t*>                container_ptr_main_memory_t;
/// ============================================================================
/// Trace Reader
/// ============================================================================
 /*! Read the instruction/memory trace files
  * Open the static trace => Dictionary of instruction per Basic Block
  * Open the dynamic trace(s) => Reads the threads Basic Block execution trace
  * Open the memory trace(s) => Match the memory addresses for R/W Instructions
  */
class trace_reader_t {
    private:
        std::ifstream StaticTraceFile;
        std::ifstream *DynamicTraceFile;
        std::ifstream *MemoryTraceFile;

        gzFile gzStaticTraceFile;
        gzFile *gzDynamicTraceFile;
        gzFile *gzMemoryTraceFile;

        bool *insideBBL;
        uint64_t *trace_opcode_counter;
        uint64_t *trace_opcode_max;
        uint64_t trace_opcode_total;

        container_static_dictionary_t static_dictionary;
        uint32_t *actual_bbl;
        uint32_t *actual_bbl_opcode;
        bool compressed_trace_file_on;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        trace_reader_t();
        ~trace_reader_t();
        void allocate(char *in_file, bool is_compact, uint32_t ncpus);  /// must be called after the parameters are set properly
        inline const char* get_label() {
            return "TRACE_READER";
        };
        inline const char* get_type_component_label() {
            return "TRACE_READER";
        };

        uint64_t trace_size(uint32_t cpuid);
        void gzgetline(gzFile file, char* new_line);
        void gzgetline(gzFile file, std::string& new_line);
        uint32_t trace_next_dynamic(uint32_t cpuid, sync_t *sync_found);
        std::string trace_next_memory(uint32_t cpuid);
        bool trace_fetch(uint32_t cpuid, opcode_package_t *m);
        void genenate_static_dictionary();
        void check_static_dictionary();

        /// Progress
        uint64_t get_trace_opcode_counter(uint32_t cpuid) { return this->trace_opcode_counter[cpuid]; }
        uint64_t get_trace_opcode_max(uint32_t cpuid) { return this->trace_opcode_max[cpuid]; }
        INSTANTIATE_GET_SET_ADD(uint64_t, trace_opcode_total);
};


/// ============================================================================
/// Configuration File Control
/// ============================================================================
class sinuca_engine_t {
    public:
        /// Program input
        char *arg_configuration_file_name;
        char *arg_trace_file_name;
        char *arg_result_file_name;
        uint32_t arg_warmup_instructions;
        bool arg_is_compressed;

        std::ofstream result_file;

        /// Array of components
        interconnection_interface_t* *interconnection_interface_array;
        processor_t* *processor_array;
        cache_memory_t* *cache_memory_array;
        main_memory_t* *main_memory_array;
        interconnection_router_t* *interconnection_router_array;

        uint32_t interconnection_interface_array_size;
        uint32_t processor_array_size;
        uint32_t cache_memory_array_size;
        uint32_t main_memory_array_size;
        uint32_t interconnection_router_array_size;

        /// Control the Global Cycle
        uint64_t global_cycle;
        /// Control the line size for all components
        uint32_t global_line_size;
        uint64_t global_offset_bits_mask;
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
        void initialize_main_memory();
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
        INSTANTIATE_GET_SET(main_memory_t**, main_memory_array);
        INSTANTIATE_GET_SET(interconnection_router_t**, interconnection_router_array);

        INSTANTIATE_GET_SET(uint32_t, interconnection_interface_array_size);
        INSTANTIATE_GET_SET(uint32_t, processor_array_size);
        INSTANTIATE_GET_SET(uint32_t, cache_memory_array_size);
        INSTANTIATE_GET_SET(uint32_t, main_memory_array_size);
        INSTANTIATE_GET_SET(uint32_t, interconnection_router_array_size);

        INSTANTIATE_GET_SET(bool, is_simulation_allocated);
        INSTANTIATE_GET_SET(bool, is_runtime_debug);
        INSTANTIATE_GET_SET(bool, is_simulation_eof);
        INSTANTIATE_GET_SET(bool, is_warm_up);

        INSTANTIATE_GET_SET(uint64_t, global_cycle);

        void set_global_line_size(uint32_t new_size);   /// Define one global_line_size and global_offset_bits_mask
        uint32_t get_global_line_size();

        INSTANTIATE_GET_SET(uint64_t, global_offset_bits_mask);


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        void write_statistics(const char *buffer);

        void write_statistics_small_separator();
        void write_statistics_big_separator();

        void write_statistics_comments(const char *comment);

        void write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, const char *value);
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


/// ============================================================================
/// Opcode Package (High Level Instruction)
/// ============================================================================
class opcode_package_t {
    public:
        /// TRACE Variables
        char opcode_assembly[20];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        container_register_t read_regs;
        container_register_t write_regs;

        bool is_read;
        uint64_t read_address;
        uint32_t read_size;

        bool is_read2;
        uint64_t read2_address;
        uint32_t read2_size;

        bool is_write;
        uint64_t write_address;
        uint32_t write_size;

        bool is_branch;
        bool is_predicated;
        bool is_prefetch;

        /// SINUCA Control Variables
        uint64_t opcode_number;
        package_state_t state;
        uint64_t ready_cycle;
        uint64_t born_cycle;
        sync_t sync_type;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        opcode_package_t() {
            this->package_clean();
        };
        ~opcode_package_t() {
            this->read_regs.clear();
            this->write_regs.clear();
        };


        std::string opcode_to_string();
        void opcode_to_trace_char(char *trace_line);

        std::string opcode_to_trace_string();
        void trace_string_to_opcode(std::string input_string);
        void trace_string_to_read(std::string input_string);
        void trace_string_to_read2(std::string input_string);
        void trace_string_to_write(std::string input_string);

        opcode_package_t & operator=(const opcode_package_t &package);
        bool operator==(const opcode_package_t &package);

        void package_clean();
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_stall(uint32_t wait_time);
        void package_ready(uint32_t wait_time);

        static int32_t find_free(opcode_package_t *input_array, uint32_t size_array);
        static int32_t find_opcode_number(opcode_package_t *input_array, uint32_t size_array, uint64_t opcode_number);
        static bool check_age(opcode_package_t *input_array, uint32_t size_array);
        static bool check_age(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
        static std::string print_all(opcode_package_t *input_array, uint32_t size_array);
        static std::string print_all(opcode_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
};

/// ============================================================================
/// Microcode Package (Low Level Instruction)
/// ============================================================================
class uop_package_t {
    public:
        /// TRACE Variables
        char opcode_assembly[20];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        container_register_t read_regs;
        container_register_t write_regs;

        instruction_operation_t uop_operation;
        uint64_t memory_address;
        uint32_t memory_size;

        /// SINUCA Control Variables
        uint64_t opcode_number;
        uint64_t uop_number;
        package_state_t state;
        uint64_t ready_cycle;
        uint64_t born_cycle;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        uop_package_t() {
            this->package_clean();
        };
        ~uop_package_t() {
            this->read_regs.clear();
            this->write_regs.clear();
        };


        std::string uop_to_string();

        uop_package_t & operator=(const uop_package_t &package);
        bool operator==(const uop_package_t &package);

        void opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode);
        void package_clean();
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_stall(uint32_t wait_time);
        void package_ready(uint32_t wait_time);

        static int32_t find_free(uop_package_t *input_array, uint32_t size_array);
        static bool check_age(uop_package_t *input_array, uint32_t size_array);
        static bool check_age(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
        static std::string print_all(uop_package_t *input_array, uint32_t size_array);
        static std::string print_all(uop_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
};


/// ============================================================================
/// Memory Package
/// ============================================================================
 /*! Memory Package class to transport information about memory requests
  */
class memory_package_t {
    public:
        uint32_t id_owner;                      /// if (read / write) PROCESSOR.ID   else if (copy-back / prefetch) CACHE_MEMORY.ID
        uint64_t opcode_number;                 /// initial opcode number
        uint64_t opcode_address;                /// initial opcode address
        uint64_t uop_number;                    /// initial uop number (Instruction == 0)
        uint64_t memory_address;                /// memory address
        uint32_t memory_size;                   /// operation size after offset

        package_state_t state;                  /// package state
        uint64_t ready_cycle;                   /// package latency
        uint64_t born_cycle;                    /// package create time

        memory_operation_t memory_operation;    /// memory operation
        bool is_answer;                         /// is answer or request

        bool *sub_blocks;

        /// Router Control
        uint32_t id_src;                        /// id src component
        uint32_t id_dst;                        /// id dst component

        uint32_t *hops;                         /// route information
        int32_t hop_count;                     /// route information

        /// ====================================================================
        /// Methods
        /// ====================================================================
        memory_package_t() {
            ERROR_ASSERT_PRINTF(sinuca_engine.get_global_line_size() > 0, "Allocating 0 positions.\n")
            this->sub_blocks = new bool[sinuca_engine.get_global_line_size()];

            this->package_clean();
        };
        ~memory_package_t() {
            if (this->sub_blocks) delete [] sub_blocks;
        };


        std::string memory_to_string();

        memory_package_t & operator=(const memory_package_t &package);
        void package_clean();
        void package_untreated(uint32_t wait_time);
        void package_wait(uint32_t wait_time);
        void package_ready(uint32_t wait_time);
        void package_transmit(uint32_t stall_time);
        void packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                    uint64_t memory_address, uint32_t memory_size,
                    package_state_t state, uint32_t stall_time,
                    memory_operation_t memory_operation, bool is_answer,
                    uint32_t id_src, uint32_t id_dst, uint32_t *hops, uint32_t hop_count);

        static int32_t find_free(memory_package_t *input_array, uint32_t size_array);
        static void find_old_rqst_ans_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state, int32_t &position_rqst, int32_t &position_ans);
        static int32_t find_old_request_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state);
        static int32_t find_old_answer_state_ready(memory_package_t *input_array, uint32_t size_array, package_state_t state);
        static int32_t find_state_mem_address(memory_package_t *input_array, uint32_t size_array, package_state_t state, uint64_t address);

        static std::string print(memory_package_t input_package);
        static std::string print_all(memory_package_t *input_array, uint32_t size_array);
        static std::string print_all(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);

        static bool check_age(memory_package_t *input_array, uint32_t size_array);
        static bool check_age(memory_package_t **input_matrix, uint32_t size_x_matrix, uint32_t size_y_matrix);
};


/// ============================================================================
/// Statistics
/// ============================================================================
 /*! Statistics methods to be implemented on every component
  */
class statistics_t {
    public:
        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        virtual void reset_statistics()=0;      /// Reset all internal statistics variables
        virtual void print_statistics()=0;      /// Print out the internal statistics variables
        virtual void print_configuration()=0;   /// Print out the internal configuration variables
};

/// ============================================================================
/// Interconnection Interface
/// ============================================================================
 /*! Generic class which creates the II (Interconnection Interface).
  * Network Routers to Processors, Cache Memories, and Main Memory.
  * The II will exist on almost every component in order to interface the components
  */
class interconnection_interface_t : public statistics_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t id;                    /// ID unique which defines the object.
        component_t type_component;     /// ID unique which defines the object.
        char label[100];                /// Comprehensive object's name.
        uint32_t max_ports;
        uint32_t used_ports;
        uint32_t interconnection_width;
        uint32_t interconnection_latency;

        /// ====================================================================
        /// Set by this->set_higher_lower_level_component()
        /// ====================================================================
        interconnection_interface_t* *interface_output_components;
        uint32_t *ports_output_components;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        interconnection_interface_t();
        ~interconnection_interface_t();

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


        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        virtual void allocate()=0;  /// Called after all the parameters are set
        virtual void clock(uint32_t sub_cycle)=0;   /// Called every cycle
        virtual bool receive_package(memory_package_t *package, uint32_t input_port)=0; /// return OK or FAIL
        virtual void periodic_check()=0;    /// Check all the internal structures
        virtual void print_structures()=0;  /// Print the internal structures
        virtual void panic()=0;             /// Called when some ERROR happens
};

/// ============================================================================
/// Interconnection Controller
/// ============================================================================
 /*! Interconnection Controller.
  * This component connected between the L1 Cache and the UCA/NUCA memory,
  * will be the smart guy how knows where to search a data address
  * among all the NUCA banks.
  */
class edge_t {
    public:
        interconnection_interface_t *src, *dst;
        uint32_t src_port, dst_port;
        uint64_t weight;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        edge_t() {
            this->src = NULL;
            this->dst = NULL;
            this->src_port = 0;
            this->dst_port = 0;
            this->weight = 0;
        };
        ~edge_t() {};
};

class routing_table_element_t {
    public:
        uint32_t *hops;
        uint32_t hop_count;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        routing_table_element_t();
        ~routing_table_element_t();
};

class interconnection_controller_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        routing_algorithm_t routing_algorithm;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        /// Routing table sized[max_id][max_id] of white packages, with *hops and hop_count only;
        interconnection_interface_t ***pred;
        edge_t **adjacency_matrix;
        routing_table_element_t **route_matrix;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        interconnection_controller_t();
        ~interconnection_controller_t();
        inline const char* get_label() {
            return "INTERCONNECTION_CTRL";
        };
        inline const char* get_type_component_label() {
            return "INTERCONNECTION_CTRL";
        };
        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void create_route(interconnection_interface_t *src, interconnection_interface_t *dst);
        void find_package_route(memory_package_t *package);
        uint32_t find_package_route_latency(memory_package_t *package);
        void create_communication_graph();

        /// routing algorithms
        void routing_algorithm_floyd_warshall();  /// Create the routing table with floy-wharsall routing algorithm
        void routing_algorithm_xy();  /// Create the routing table for mesh NoCs with XY routing
        void routing_algorithm_odd_even();  /// Create the routing table for mesh NoCs with OddEven routing

        INSTANTIATE_GET_SET(routing_algorithm_t, routing_algorithm)
};

/// ============================================================================
/// Network-on-Chip Router.
/// ============================================================================
 /*! Class used to interconnect interconnection_interface_t components
  */
class interconnection_router_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t input_buffer_size;  /// Input buffer depth.
        selection_t selection_policy;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        memory_package_t **input_buffer;  /// Circular Input buffer [ports][input_buffer_size].
        uint32_t *input_buffer_position_start;
        uint32_t *input_buffer_position_end;
        uint32_t *input_buffer_position_used;

        uint64_t ready_cycle;  /// Time left for the router's next operation.
        uint32_t last_send;  /// The last port that has something been sent. Used by RoundRobin and BufferLevel selection.


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_transmissions;
    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        interconnection_router_t();
        ~interconnection_router_t();

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        /// Circular Buffer find next free space
        int32_t input_buffer_insert(uint32_t port);
        /// Circular Buffer delete oldest package
        void input_buffer_remove(uint32_t port);
        void input_buffer_reinsert(uint32_t port);

        /// Selection strategies
        uint32_t selectionRandom();
        uint32_t selectionBufferLevel();
        uint32_t selectionRoundRobin();

        INSTANTIATE_GET_SET(selection_t, selection_policy)
        INSTANTIATE_GET_SET(uint32_t, ready_cycle)
        INSTANTIATE_GET_SET(uint32_t, input_buffer_size)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_transmissions)
};


/// ============================================================================
/// Branch Predictor
/// ============================================================================
class branch_target_buffer_line_t {
    public:
        uint64_t tag;
        uint64_t last_access;
        uint64_t usage_counter;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        branch_target_buffer_line_t() {
            this->tag = 0;
            this->last_access = 0;
            this->usage_counter = 0;
        };
        ~branch_target_buffer_line_t() {
        };
};

class branch_target_buffer_set_t {
    public:
        branch_target_buffer_line_t *ways;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        branch_target_buffer_set_t() {
        };
        ~branch_target_buffer_set_t() {
            if (this->ways) delete [] ways;
            // ~ utils_t::template_delete_array<branch_target_buffer_line_t>(ways);
        };
};

/// Class for Branch Predictor
class branch_predictor_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        branch_predictor_policy_t branch_predictor_type;   /// Prefetch policy choosen by the user
        uint32_t btb_line_number;                   /// Branch Target Buffer Size
        uint32_t btb_associativity;                 /// Branch Target Buffer Associativity
        replacement_t btb_replacement_policy;       /// Branch Target Buffer Replacement Policy

        uint32_t bht_signature_bits;                /// Signature size
        hash_function_t bht_signature_hash;         /// Signature hash function (SIGNATURE xor PC)
        uint32_t bht_fsm_bits;                      /// Finite State Machine size

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        branch_target_buffer_set_t *btb;    /// Branch Target Buffer
        uint32_t btb_total_sets;            /// Branch Target Buffer Associativity

        uint64_t btb_index_bits_mask;       /// Index mask
        uint64_t btb_index_bits_shift;

        uint64_t btb_tag_bits_mask;         /// Tag mask
        uint64_t btb_tag_bits_shift;

        uint32_t *bht;                      /// Branch History Table (2Level predictor)
        uint32_t bht_signature;             /// Branch History signature (2Level predictor)
        uint32_t bht_signature_bits_mask;   /// Branch History signature mask (2Level predictor)
        uint32_t bht_fsm_bits_mask;         /// Branch History fsm mask (2Level predictor)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_btb_accesses;
        uint64_t stat_btb_hit;
        uint64_t stat_btb_miss;

        uint64_t stat_branch_predictor_operation;
        uint64_t stat_branch_predictor_hit;
        uint64_t stat_branch_predictor_miss;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        branch_predictor_t();
        ~branch_predictor_t();
        inline const char* get_type_component_label() {
            return "BRANCH_PREDICTOR";
        };

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        uint32_t btb_evict_address(uint64_t opcode_address);
        bool btb_find_update_address(uint64_t opcode_address);

        inline uint64_t btb_get_tag(uint64_t addr) {
            return (addr & this->btb_tag_bits_mask) >> this->btb_tag_bits_shift;
        }

        inline uint64_t btb_get_index(uint64_t addr) {
            return (addr & this->btb_index_bits_mask) >> this->btb_index_bits_shift;
        }

        bool bht_find_update_prediction(opcode_package_t actual_opcode, opcode_package_t next_opcode);
        processor_stage_t predict_branch(opcode_package_t actual_opcode, opcode_package_t next_opcode);


        INSTANTIATE_GET_SET(branch_predictor_policy_t, branch_predictor_type)
        INSTANTIATE_GET_SET(uint32_t, btb_line_number)
        INSTANTIATE_GET_SET(uint32_t, btb_associativity)
        INSTANTIATE_GET_SET(uint32_t, btb_total_sets)
        INSTANTIATE_GET_SET(replacement_t, btb_replacement_policy)

        INSTANTIATE_GET_SET(uint32_t, bht_signature_bits)
        INSTANTIATE_GET_SET(uint32_t, bht_signature_bits_mask)
        INSTANTIATE_GET_SET(hash_function_t, bht_signature_hash)
        INSTANTIATE_GET_SET(uint32_t, bht_fsm_bits)
        INSTANTIATE_GET_SET(uint32_t, bht_fsm_bits_mask)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_miss)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_operation)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_miss)
};

/// ============================================================================
/// Processor Cores.
/// ============================================================================
 /*! Class to instantiate the processor cores.
  * This class will receive the instructions in order from the trace file.
  * It will implements also the write buffer.
  */

class reorder_buffer_line_t {
    public:
        uop_package_t uop;                          /// uOP stored
        processor_stage_t stage;                    /// Stage of the uOP
        uint32_t wait_deps_number;                  /// Must wait BEFORE execution
        reorder_buffer_line_t* *deps_ptr_array;     /// Elements to wake-up AFTER execution

        /// ====================================================================
        /// Methods
        /// ====================================================================
        reorder_buffer_line_t() {
            this->uop.package_clean();
            this->stage = PROCESSOR_STAGE_DECODE;
            this->wait_deps_number = 0;
            this->deps_ptr_array = NULL;
        }

        ~reorder_buffer_line_t() {
            if (this->deps_ptr_array) delete [] deps_ptr_array;
            // ~ utils_t::template_delete_array<reorder_buffer_line_t*>(deps_ptr_array);
        }
};



class processor_t : public interconnection_interface_t {
    public:
        /// Branch Predictor
        branch_predictor_t branch_predictor;  /// Branch Predictor
        uint64_t branch_opcode_number;                /// Opcode which will solve the branch
        processor_stage_t branch_solve_stage;           /// Stage which will solve the branch

    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t core_id;

        /// Buffers' Size
        uint32_t fetch_buffer_size;
        uint32_t decode_buffer_size;
        uint32_t reorder_buffer_size;

        /// Stages' Latency
        uint32_t stage_fetch_cycles;
        uint32_t stage_decode_cycles;
        uint32_t stage_rename_cycles;
        uint32_t stage_dispatch_cycles;
        uint32_t stage_execution_cycles;
        uint32_t stage_commit_cycles;

        /// Stages' Width
        uint32_t stage_fetch_width;
        uint32_t stage_decode_width;
        uint32_t stage_rename_width;
        uint32_t stage_dispatch_width;
        uint32_t stage_execution_width;
        uint32_t stage_commit_width;

        /// Integer Funcional Units
        uint32_t number_fu_int_alu;
        uint32_t latency_fu_int_alu;

        uint32_t number_fu_int_mul;
        uint32_t latency_fu_int_mul;

        uint32_t number_fu_int_div;
        uint32_t latency_fu_int_div;

        /// Floating Point Functional Units
        uint32_t number_fu_fp_alu;
        uint32_t latency_fu_fp_alu;

        uint32_t number_fu_fp_mul;
        uint32_t latency_fu_fp_mul;

        uint32_t number_fu_fp_div;
        uint32_t latency_fu_fp_div;

        /// Memory Functional Units
        uint32_t number_fu_mem_load;
        uint32_t latency_fu_mem_load;

        uint32_t number_fu_mem_store;
        uint32_t latency_fu_mem_store;

        uint32_t read_buffer_size;
        uint32_t write_buffer_size;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t offset_bits_mask;      /// Offset mask
        uint64_t not_offset_bits_mask;  /// Offset mask


        /// Synchronization Control Variables
        sync_t sync_status;                    /// Used to sync or request sync
        uint64_t sync_status_time;              /// Last time the sync changed
        bool trace_over;

        /// Stages Control Variables
        opcode_package_t trace_next_opcode;
        uint64_t fetch_pc;
        uint64_t fetch_pc_line_buffer;

        uint64_t fetch_opcode_counter;
        uint64_t decode_opcode_counter;

        uint64_t decode_uop_counter;
        uint64_t rename_uop_counter;
        uint64_t commit_uop_counter;

        /// Fetch buffer
        opcode_package_t *fetch_buffer;
        uint32_t fetch_buffer_position_start;
        uint32_t fetch_buffer_position_end;
        uint32_t fetch_buffer_position_used;

        /// Decode buffer
        uop_package_t *decode_buffer;
        uint32_t decode_buffer_position_start;
        uint32_t decode_buffer_position_end;
        uint32_t decode_buffer_position_used;

        /// Reorder buffer
        reorder_buffer_line_t *reorder_buffer;
        uint32_t reorder_buffer_position_start;
        uint32_t reorder_buffer_position_end;
        uint32_t reorder_buffer_position_used;

        /// Register Alias Table for Renaming
        uint32_t register_alias_table_size;
        reorder_buffer_line_t* *register_alias_table;
        reorder_buffer_line_t* *memory_map_table;

        /// Containers to fast the execution, with pointers of UOPs ready.
        container_ptr_reorder_buffer_line_t unified_reservation_station;    /// dispatch->execute
        container_ptr_reorder_buffer_line_t unified_functional_units;       /// execute->commit
        /// ====================================================================
        /// Integer Functional Units
        uint32_t fu_int_alu;
        uint32_t fu_int_mul;
        uint32_t fu_int_div;

        /// ====================================================================
        /// Floating Point Functional Units
        uint32_t fu_fp_alu;
        uint32_t fu_fp_mul;
        uint32_t fu_fp_div;

        /// ====================================================================
        /// Memory Functional Units
        uint32_t fu_mem_load;
        uint32_t fu_mem_store;

        memory_package_t *read_buffer;
        memory_package_t *write_buffer;

        cache_memory_t *data_cache;
        cache_memory_t *inst_cache;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_reset_fetch_opcode_counter;
        uint64_t stat_reset_decode_uop_counter;

        uint64_t stat_nop_completed;
        uint64_t stat_branch_completed;
        uint64_t stat_other_completed;

        uint64_t stat_int_alu_completed;
        uint64_t stat_int_mul_completed;
        uint64_t stat_int_div_completed;

        uint64_t stat_fp_alu_completed;
        uint64_t stat_fp_mul_completed;
        uint64_t stat_fp_div_completed;

        uint64_t stat_instruction_read_completed;
        uint64_t stat_memory_read_completed;
        uint64_t stat_memory_write_completed;

        uint64_t stat_min_instruction_read_wait_time;
        uint64_t stat_max_instruction_read_wait_time;
        uint64_t stat_acumulated_instruction_read_wait_time;

        uint64_t stat_min_memory_read_wait_time;
        uint64_t stat_max_memory_read_wait_time;
        uint64_t stat_acumulated_memory_read_wait_time;

        uint64_t stat_min_memory_write_wait_time;
        uint64_t stat_max_memory_write_wait_time;
        uint64_t stat_acumulated_memory_write_wait_time;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        processor_t();
        ~processor_t();
        int32_t send_instruction_package(opcode_package_t *is);
        int32_t send_data_package(memory_package_t *ms);

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void synchronize(sync_t new_sync);
        void solve_branch(uint64_t opcode_number, processor_stage_t processor_stage);
        void stage_fetch();
        void stage_decode();
        void stage_rename();
        void stage_dispatch();
        void stage_execution();
        void solve_data_forward(reorder_buffer_line_t *rob_line);
        void stage_commit();

        inline uint32_t cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }

        inline bool is_busy() {
            return (trace_over == false || reorder_buffer_position_used != 0);
        }

        /// Fetch Buffer =======================================================
        int32_t fetch_buffer_insert();
        void fetch_buffer_remove();
        int32_t fetch_buffer_find_opcode_number(uint64_t opcode_number);
        /// ====================================================================

        /// Decode Buffer ======================================================
        int32_t decode_buffer_insert();
        void decode_buffer_remove();
        /// ====================================================================

        /// Reorder Buffer =====================================================
        void rob_line_clean(uint32_t reorder_buffer_line);
        std::string rob_line_to_string(uint32_t reorder_buffer_line);
        std::string rob_print_all();
        int32_t rob_insert();
        void rob_remove();
        int32_t rob_find_uop_number(uint64_t uop_number);
        bool rob_check_age();
        /// ====================================================================

        INSTANTIATE_GET_SET(uint32_t, core_id)
        INSTANTIATE_GET_SET(uint64_t, offset_bits_mask)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)

        INSTANTIATE_GET_SET(uint32_t, fetch_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, decode_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, reorder_buffer_size)

        /// Stages' Latency
        INSTANTIATE_GET_SET(uint32_t, stage_fetch_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_decode_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_rename_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_dispatch_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_execution_cycles)
        INSTANTIATE_GET_SET(uint32_t, stage_commit_cycles)

        /// Stages' Width
        INSTANTIATE_GET_SET(uint32_t, stage_fetch_width)
        INSTANTIATE_GET_SET(uint32_t, stage_decode_width)
        INSTANTIATE_GET_SET(uint32_t, stage_rename_width)
        INSTANTIATE_GET_SET(uint32_t, stage_dispatch_width)
        INSTANTIATE_GET_SET(uint32_t, stage_execution_width)
        INSTANTIATE_GET_SET(uint32_t, stage_commit_width)

        INSTANTIATE_GET_SET(uint64_t, fetch_opcode_counter)
        INSTANTIATE_GET_SET(uint64_t, decode_opcode_counter)

        INSTANTIATE_GET_SET(uint64_t, decode_uop_counter)
        INSTANTIATE_GET_SET(uint64_t, rename_uop_counter)
        INSTANTIATE_GET_SET(uint64_t, commit_uop_counter)

        /// Integer Funcional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_int_alu)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_alu)
        INSTANTIATE_GET_SET(uint32_t, number_fu_int_mul)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_mul)
        INSTANTIATE_GET_SET(uint32_t, number_fu_int_div)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_int_div)

        /// Floating Point Functional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_alu)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_alu)
        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_mul)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_mul)
        INSTANTIATE_GET_SET(uint32_t, number_fu_fp_div)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_fp_div)

        /// Memory Functional Units
        INSTANTIATE_GET_SET(uint32_t, number_fu_mem_load)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_mem_load)
        INSTANTIATE_GET_SET(uint32_t, number_fu_mem_store)
        INSTANTIATE_GET_SET(uint32_t, latency_fu_mem_store)

        INSTANTIATE_GET_SET(uint32_t, read_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, write_buffer_size)

        INSTANTIATE_GET_SET(cache_memory_t*, data_cache)
        INSTANTIATE_GET_SET(cache_memory_t*, inst_cache)

        /// Processor Synchronization
        INSTANTIATE_GET_SET(sync_t, sync_status);
        INSTANTIATE_GET_SET(uint64_t, sync_status_time);

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_fetch_opcode_counter)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_reset_decode_uop_counter)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_nop_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_other_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_alu_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_mul_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_int_div_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_alu_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_mul_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_fp_div_completed)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_read_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_memory_read_completed)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_memory_write_completed)

        inline void add_stat_instruction_read_completed(uint64_t born_cycle) {
            this->stat_instruction_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            stat_acumulated_instruction_read_wait_time += new_time;
            if (stat_min_instruction_read_wait_time > new_time) stat_min_instruction_read_wait_time = new_time;
            if (stat_max_instruction_read_wait_time < new_time) stat_max_instruction_read_wait_time = new_time;
        };

        inline void add_stat_memory_read_completed(uint64_t born_cycle) {
            this->stat_memory_read_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_memory_read_wait_time += new_time;
            if (stat_min_memory_read_wait_time > new_time) stat_min_memory_read_wait_time = new_time;
            if (stat_max_memory_read_wait_time < new_time) stat_max_memory_read_wait_time = new_time;
        };

        inline void add_stat_memory_write_completed(uint64_t born_cycle) {
            this->stat_memory_write_completed++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_memory_write_wait_time += new_time;
            if (stat_min_memory_write_wait_time > new_time) stat_min_memory_write_wait_time = new_time;
            if (stat_max_memory_write_wait_time < new_time) stat_max_memory_write_wait_time = new_time;
        };
};

/// ============================================================================
/// Directory
/// ============================================================================
 /*! Directory for maintain the cache coherence on LLC (Last Level Cache).
 */
class directory_controller_line_t {
    public:
        uint32_t id_owner;
        uint64_t opcode_number;
        uint64_t opcode_address;
        uint64_t uop_number;

        lock_t lock_type;

        uint32_t *cache_request_order;
        uint32_t cache_requested;

        memory_operation_t initial_memory_operation;
        uint64_t initial_memory_address;
        uint32_t initial_memory_size;

        uint64_t born_cycle;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        directory_controller_line_t();
        ~directory_controller_line_t();
        directory_controller_line_t & operator=(const directory_controller_line_t &line);

        void packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                        lock_t lock_type,
                        memory_operation_t initial_memory_operaton, uint64_t initial_memory_address, uint32_t initial_memory_size);
        std::string directory_controller_line_to_string();
        static bool check_age(container_ptr_directory_controller_line_t *input_array, uint32_t size_array);
};


class directory_controller_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        coherence_protocol_t coherence_protocol_type;
        inclusiveness_t inclusiveness_type;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t not_offset_bits_mask;
        container_ptr_cache_memory_t *llc_caches;
        container_ptr_directory_controller_line_t *directory_lines;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_instruction_hit;
        uint64_t stat_read_hit;
        uint64_t stat_prefetch_hit;
        uint64_t stat_write_hit;
        uint64_t stat_copyback_hit;

        uint64_t stat_instruction_miss;
        uint64_t stat_read_miss;
        uint64_t stat_prefetch_miss;
        uint64_t stat_write_miss;
        uint64_t stat_copyback_miss;

        uint64_t stat_min_instruction_wait_time;
        uint64_t stat_max_instruction_wait_time;
        uint64_t stat_acumulated_instruction_wait_time;

        uint64_t stat_min_read_wait_time;
        uint64_t stat_max_read_wait_time;
        uint64_t stat_acumulated_read_wait_time;

        uint64_t stat_min_prefetch_wait_time;
        uint64_t stat_max_prefetch_wait_time;
        uint64_t stat_acumulated_prefetch_wait_time;

        uint64_t stat_min_write_wait_time;
        uint64_t stat_max_write_wait_time;
        uint64_t stat_acumulated_write_wait_time;

        uint64_t stat_min_copyback_wait_time;
        uint64_t stat_max_copyback_wait_time;
        uint64_t stat_acumulated_copyback_wait_time;


    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        directory_controller_t();
        ~directory_controller_t();

        inline const char* get_label() {
            return "DIRECTORY_CTRL";
        };
        inline const char* get_type_component_label() {
            return "DIRECTORY_CTRL";
        };

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();

        /// ====================================================================

        package_state_t treat_cache_request(uint32_t obj_id, memory_package_t *package);
        package_state_t treat_cache_answer(uint32_t obj_id, memory_package_t *package);
        bool create_cache_copyback(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way);

        uint32_t find_next_obj_id(cache_memory_t *cache_memory, uint64_t memory_address);
        bool is_locked(uint64_t memory_address);

        bool coherence_is_read(memory_operation_t memory_operation);
        bool coherence_is_hit(cache_line_t *cache_line, memory_package_t *package);
        bool coherence_need_copyback(cache_line_t *cache_line);
        protocol_status_t look_higher_levels(cache_memory_t *cache_memory, memory_package_t *package, bool check_llc);
        void coherence_invalidate_all(cache_memory_t *cache_memory, uint64_t memory_address);
        void coherence_new_operation(cache_memory_t *cache_memory, cache_line_t *cache_line,  memory_package_t *package, bool is_hit);

        inline uint32_t cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & not_offset_bits_mask) == (memory_addressB & not_offset_bits_mask);
        };

        inline uint32_t get_directory_lines_size() {
            return this->directory_lines->size();
        };

        INSTANTIATE_GET_SET(container_ptr_directory_controller_line_t*, directory_lines)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)
        INSTANTIATE_GET_SET(coherence_protocol_t, coherence_protocol_type)
        INSTANTIATE_GET_SET(inclusiveness_t, inclusiveness_type)


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_prefetch_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback_hit)

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_read_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_write_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_copyback_miss)

        inline void add_stat_instruction_miss(uint64_t born_cycle) {
            this->stat_instruction_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_miss(uint64_t born_cycle) {
            this->stat_read_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_miss(uint64_t born_cycle) {
            this->stat_prefetch_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };

        inline void add_stat_write_miss(uint64_t born_cycle) {
            this->stat_write_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_miss(uint64_t born_cycle) {
            this->stat_copyback_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };
};

/// ============================================================================
/// Prefetcher
/// ============================================================================
class cache_prefetch_stream_table_t {
    public:
        uint64_t last_memory_address;           /// Last Memory Request which matched into this stream
        int64_t memory_address_difference;      /// Difference between one access to another
        uint32_t relevance_count;               /// Number of Memory Requests which matched into this stream
        uint32_t prefetch_ahead;                /// Number of prefetchs ahead, already done
        uint64_t cycle_last_activation;         /// Last time a Memory Request matched into this stream
        uint64_t cycle_last_request;            /// Last prefetch done

        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_prefetch_stream_table_t() {
            this->last_memory_address = 0;
            this->memory_address_difference = 0;
            this->relevance_count = 0;
            this->cycle_last_activation = 0;
            this->prefetch_ahead = 0;
            this->cycle_last_request = 0;
        };
        ~cache_prefetch_stream_table_t() {
        };
};

/// Class for Prefetch.
class prefetch_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        prefetch_policy_t prefetcher_type;  /// Prefetch policy choosen by the user
        full_buffer_t full_buffer_type;
        uint32_t request_buffer_size;

        /// Set in case Stream Prefetch were used.
        uint32_t stream_table_size;             /// Prefetch Stream Detector Table Size
        uint32_t stream_address_distance;          /// Prefetch Range to Detect Stream
        uint32_t stream_window;                 /// Detect the Stream as Dead
        uint32_t stream_threshold_activate;     /// Minimum relevance to start prefetch
        uint32_t stream_prefetch_degree;               /// Maximum number of prefetchs ahead
        uint32_t stream_wait_between_requests;  /// Wait time between prefetch generation

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        memory_package_t *request_buffer;  /// Prefetch transactions waiting for room on the MSHR (High Level Input)
        uint32_t request_buffer_position_start;
        uint32_t request_buffer_position_end;
        uint32_t request_buffer_position_used;

        uint64_t offset_bits_mask;
        uint64_t not_offset_bits_mask;

        cache_prefetch_stream_table_t *stream_table;
    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        prefetch_t();
        ~prefetch_t();
        inline const char* get_type_component_label() {
            return "PREFETCH";
        };

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();

        /// REQUEST_BUFFER =====================================================
        int32_t request_buffer_insert();
        memory_package_t* request_buffer_get_older();
        void request_buffer_remove();

        /// STREAM_TABLE =====================================================
        void stream_table_line_clean(uint32_t stream_buffer_line);
        std::string stream_table_line_to_string(uint32_t stream_buffer_line);
        std::string stream_table_print_all();
        /// ====================================================================

        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void treat_prefetch(memory_package_t *package);
        memory_package_t* next_prefetch();

        INSTANTIATE_GET_SET(prefetch_policy_t, prefetcher_type)
        INSTANTIATE_GET_SET(full_buffer_t, full_buffer_type)

        INSTANTIATE_GET_SET(uint64_t, offset_bits_mask)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)

        INSTANTIATE_GET_SET(memory_package_t*, request_buffer)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_start)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_end)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_used)

        INSTANTIATE_GET_SET(uint32_t, stream_table_size)
        INSTANTIATE_GET_SET(uint32_t, stream_address_distance)
        INSTANTIATE_GET_SET(uint32_t, stream_window)
        INSTANTIATE_GET_SET(uint32_t, stream_threshold_activate)
        INSTANTIATE_GET_SET(uint32_t, stream_prefetch_degree)
        INSTANTIATE_GET_SET(uint32_t, stream_wait_between_requests)
};

/// ============================================================================
/// Dead Sub-Block Predictor
/// ============================================================================
class DSBP_PHT_line_t {
    public:
        uint64_t pc;
        uint64_t offset;
        uint64_t last_access;
        bool pointer;
        uint64_t *usage_counter;
        bool *overflow;

        DSBP_PHT_line_t() {
            this->pc = 0;
            this->offset = 0;
            this->last_access = 0;
            this->pointer = 0;
            this->usage_counter = NULL;
            this->overflow = NULL;
        };
        ~DSBP_PHT_line_t() {
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };
};

class DSBP_PHT_sets_t {
    public:
        DSBP_PHT_line_t *ways;
};


class DSBP_metadata_line_t {
    public:
        line_sub_block_t *valid_sub_blocks;
        uint64_t *real_usage_counter;
        uint64_t *usage_counter;
        bool *overflow;
        bool learn_mode;
        DSBP_PHT_line_t *PHT_pointer;

        // Static Energy
        uint64_t *clock_become_alive;
        uint64_t *clock_become_dead;

        uint32_t active_sub_blocks;
        bool is_dead;

        DSBP_metadata_line_t() {
            this->valid_sub_blocks = NULL;
            this->usage_counter = NULL;
            this->overflow = NULL;
            this->learn_mode = 0;
            this->PHT_pointer = NULL;
            this->clock_become_alive = NULL;
            this->clock_become_dead = NULL;
            this->active_sub_blocks = 0;
            this->is_dead = true;
        };
        ~DSBP_metadata_line_t() {
            if (this->valid_sub_blocks) delete [] valid_sub_blocks;
            if (this->real_usage_counter) delete [] real_usage_counter;
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };
};

class DSBP_metadata_sets_t {
    public:
        DSBP_metadata_line_t *ways;
};

/// ============================================================================
/// Line Usage Predictor
/// ============================================================================
/// Class for Line Usage Predictor. (SPP, DSBP, ...)
class line_usage_predictor_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        line_usage_predictor_policy_t line_usage_predictor_type;

            /// ====================================================================
            /// DSBP
            /// ====================================================================
            uint32_t DSBP_line_number;          /// Cache Metadata
            uint32_t DSBP_associativity;        /// Cache Metadata

            uint32_t DSBP_sub_block_size;
            uint32_t DSBP_usage_counter_bits;

            /// PHT
            uint32_t DSBP_PHT_line_number;
            uint32_t DSBP_PHT_associativity;
            replacement_t DSBP_PHT_replacement_policy;
            /// ====================================================================

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================

            /// ====================================================================
            /// DSBP
            /// ====================================================================
            DSBP_metadata_sets_t *DSBP_sets;
            uint32_t DSBP_total_sets;
            uint32_t DSBP_sub_block_total;
            uint32_t DSBP_usage_counter_max;

            DSBP_PHT_sets_t *DSBP_PHT_sets;
            uint32_t DSBP_PHT_total_sets;
            uint64_t DSBP_PHT_index_bits_mask;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
            uint64_t stat_DSBP_line_sub_block_disable_always;
            uint64_t stat_DSBP_line_sub_block_disable_turnoff;
            uint64_t stat_DSBP_line_sub_block_normal_correct;
            uint64_t stat_DSBP_line_sub_block_normal_over;
            uint64_t stat_DSBP_line_sub_block_learn;
            uint64_t stat_DSBP_line_sub_block_wrong_first;

            uint64_t stat_DSBP_PHT_access;
            uint64_t stat_DSBP_PHT_hit;
            uint64_t stat_DSBP_PHT_miss;

            uint64_t *stat_accessed_sub_block;
            uint64_t *stat_active_sub_block_per_access;     /// Number of active sub_blocks on the line during one access
            uint64_t *stat_active_sub_block_per_cycle;      /// Number of cycles with a set of sub_blocks enabled

            uint64_t stat_sub_block_touch_0;
            uint64_t stat_sub_block_touch_1;
            uint64_t stat_sub_block_touch_2_3;
            uint64_t stat_sub_block_touch_4_7;
            uint64_t stat_sub_block_touch_8_15;
            uint64_t stat_sub_block_touch_16_127;
            uint64_t stat_sub_block_touch_128_bigger;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        line_usage_predictor_t();
        ~line_usage_predictor_t();
        inline const char* get_type_component_label() {
            return "LINE_USAGE_PREDICTOR";
        };

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();

        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end);
        void fill_package_sub_blocks(memory_package_t *package);
        bool check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way);

        /// Cache Operations
        void line_hit(memory_package_t *package, uint32_t index, uint32_t way);
        void line_miss(memory_package_t *package, uint32_t index, uint32_t way);
        void sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way);
        void line_copy_back(memory_package_t *package, uint32_t index, uint32_t way);
        void line_eviction(uint32_t index, uint32_t way);
        void compute_static_energy(uint32_t index, uint32_t way);

        INSTANTIATE_GET_SET(line_usage_predictor_policy_t, line_usage_predictor_type);

            /// ====================================================================
            /// DSBP
            /// ====================================================================
            std::string DSBP_metadata_line_to_string(DSBP_metadata_line_t *DSBP_metadata_line);

            INSTANTIATE_GET_SET(DSBP_metadata_sets_t*, DSBP_sets);
            INSTANTIATE_GET_SET(uint32_t, DSBP_line_number);
            INSTANTIATE_GET_SET(uint32_t, DSBP_associativity);
            INSTANTIATE_GET_SET(uint32_t, DSBP_total_sets);


            /// PHT
            DSBP_PHT_line_t* DSBP_PHT_find_line(uint64_t pc, uint64_t memory_address);
            DSBP_PHT_line_t* DSBP_PHT_evict_address(uint64_t pc, uint64_t memory_address);
            std::string DSBP_PHT_line_to_string(DSBP_PHT_line_t *PHT_line);

            INSTANTIATE_GET_SET(uint32_t, DSBP_sub_block_size);
            INSTANTIATE_GET_SET(uint32_t, DSBP_sub_block_total);
            INSTANTIATE_GET_SET(uint32_t, DSBP_usage_counter_bits);
            INSTANTIATE_GET_SET(uint32_t, DSBP_PHT_line_number);
            INSTANTIATE_GET_SET(uint32_t, DSBP_PHT_associativity);
            INSTANTIATE_GET_SET(replacement_t, DSBP_PHT_replacement_policy);
            INSTANTIATE_GET_SET(uint32_t, DSBP_PHT_total_sets);

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_disable_always);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_disable_turnoff);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_normal_correct);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_normal_over);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_learn);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_wrong_first);

            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_PHT_access);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_PHT_hit);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_PHT_miss);

            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_0);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_1);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_2_3);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_4_7);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_8_15);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_16_127);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_touch_128_bigger);
};



/// ============================================================================
/// Cache Memories
/// ============================================================================
class cache_line_t {
    public:
        uint64_t tag;
        protocol_status_t status;
        uint64_t last_access;
        uint64_t usage_counter;
        bool dirty;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_line_t() {
            this->tag = 0;
            this->status = PROTOCOL_STATUS_I;
            this->last_access = 0;
            this->usage_counter = 0;
            this->dirty = true;
        };
        ~cache_line_t() {
        };
};

class cache_set_t {
    public:
        cache_line_t *ways;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_set_t() {
        };
        ~cache_set_t() {
            /// De-Allocate memory to prevent memory leak
            if (this->ways) delete [] ways;
            // ~ utils_t::template_delete_array<cache_line_t>(ways);
        };
};

/// Cache Memory.
class cache_memory_t : public interconnection_interface_t {
    public:
        prefetch_t prefetcher;                          /// Prefetcher
        line_usage_predictor_t line_usage_predictor;    /// Line_Usage_Predictor

    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t cache_id;      /// Id corresponds to the cache_memory_array[Id]
        uint32_t bank_number;
        uint32_t total_banks;
        cache_mask_t address_mask_type;

        uint32_t hierarchy_level;
        uint32_t line_size;
        uint32_t line_number;
        uint32_t associativity;
        replacement_t replacement_policy;

        uint32_t penalty_read;
        uint32_t penalty_write;

        uint32_t mshr_buffer_request_reserved_size;
        uint32_t mshr_buffer_copyback_reserved_size;
        uint32_t mshr_buffer_prefetch_reserved_size;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t bank_bits_mask;        /// NUCA bank mask
        uint64_t bank_bits_shift;

        uint64_t offset_bits_mask;      /// Offset mask
        uint64_t not_offset_bits_mask;  /// Offset mask
        uint64_t offset_bits_shift;

        uint64_t index_bits_mask;       /// Index mask
        uint64_t index_bits_shift;

        uint64_t tag_bits_mask;         /// Tag mask
        uint64_t tag_bits_shift;

        uint32_t total_sets;
        cache_set_t *sets;              /// Internal Memory Storage

        memory_package_t *mshr_buffer;  /// Buffer of Missed Requests
        uint32_t mshr_buffer_size;

        uint64_t read_ready;            /// Ready to receive new READ
        uint64_t write_ready;           /// Ready to receive new WRITE

        container_ptr_cache_memory_t *higher_level_cache;    /// Higher Level Caches
        container_ptr_cache_memory_t *lower_level_cache;     /// Lower Level Caches

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_accesses;
        uint64_t stat_invalidation;
        uint64_t stat_invalidation_copyback;
        uint64_t stat_eviction;
        uint64_t stat_eviction_copyback;

        uint64_t stat_instruction_hit;
        uint64_t stat_read_hit;
        uint64_t stat_prefetch_hit;
        uint64_t stat_write_hit;
        uint64_t stat_copyback_hit;

        uint64_t stat_instruction_miss;
        uint64_t stat_read_miss;
        uint64_t stat_prefetch_miss;
        uint64_t stat_write_miss;
        uint64_t stat_copyback_miss;

        uint64_t stat_min_instruction_wait_time;
        uint64_t stat_max_instruction_wait_time;
        uint64_t stat_acumulated_instruction_wait_time;

        uint64_t stat_min_read_wait_time;
        uint64_t stat_max_read_wait_time;
        uint64_t stat_acumulated_read_wait_time;

        uint64_t stat_min_prefetch_wait_time;
        uint64_t stat_max_prefetch_wait_time;
        uint64_t stat_acumulated_prefetch_wait_time;

        uint64_t stat_min_write_wait_time;
        uint64_t stat_max_write_wait_time;
        uint64_t stat_acumulated_write_wait_time;

        uint64_t stat_min_copyback_wait_time;
        uint64_t stat_max_copyback_wait_time;
        uint64_t stat_acumulated_copyback_wait_time;

        uint64_t stat_full_mshr_buffer_request;
        uint64_t stat_full_mshr_buffer_copyback;
        uint64_t stat_full_mshr_buffer_prefetch;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_memory_t();
        ~cache_memory_t();

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        /// MASKS
        void set_masks();
        /// Check if MSHR supports the higher levels MSHR
        void check_mshr_size();

        inline uint64_t get_tag(uint64_t addr) {
            return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }

        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }

        inline uint64_t get_offset(uint64_t addr) {
            return (addr & this->offset_bits_mask) >> this->offset_bits_shift;
        }

        inline uint32_t cmp_tag_index_bank(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }


        inline void add_higher_level_cache(cache_memory_t *cache_memory) {
            this->higher_level_cache->push_back(cache_memory);
        }
        INSTANTIATE_GET_SET(container_ptr_cache_memory_t*, higher_level_cache)


        inline void add_lower_level_cache(cache_memory_t *cache_memory) {
            this->lower_level_cache->push_back(cache_memory);
        }
        INSTANTIATE_GET_SET(container_ptr_cache_memory_t*, lower_level_cache)

        cache_line_t* find_line(uint64_t memory_address, uint32_t& index, uint32_t& way);
        cache_line_t* evict_address(uint64_t memory_address, uint32_t& index, uint32_t& way);
        void change_address(cache_line_t *line, uint64_t new_memory_address);
        void change_status(cache_line_t *line, protocol_status_t status);
        void update_last_access(cache_line_t *line);

        /// Methods called by the directory to add statistics and others
        void cache_hit(memory_package_t *package);
        void cache_miss(memory_package_t *package);
        void cache_invalidate(uint64_t memory_address, bool is_copyback);
        void cache_evict(uint64_t memory_address, bool is_copyback);

        INSTANTIATE_GET_SET(uint32_t, cache_id)
        INSTANTIATE_GET_SET(uint32_t, bank_number)
        INSTANTIATE_GET_SET(uint32_t, total_banks)
        INSTANTIATE_GET_SET(cache_mask_t, address_mask_type)

        INSTANTIATE_GET_SET(uint32_t, hierarchy_level)
        INSTANTIATE_GET_SET(uint32_t, line_size)
        INSTANTIATE_GET_SET(uint32_t, line_number)
        INSTANTIATE_GET_SET(uint32_t, associativity)
        INSTANTIATE_GET_SET(uint32_t, total_sets)
        INSTANTIATE_GET_SET(replacement_t, replacement_policy)
        INSTANTIATE_GET_SET(uint32_t, penalty_read)
        INSTANTIATE_GET_SET(uint32_t, penalty_write)
        INSTANTIATE_GET_SET(memory_package_t*, mshr_buffer)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_request_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_copyback_reserved_size)
        INSTANTIATE_GET_SET(uint32_t, mshr_buffer_prefetch_reserved_size)


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_invalidation_copyback)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction_copyback)


        INSTANTIATE_GET_SET_ADD(uint64_t, stat_instruction_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_prefetch_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback_hit)

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_read_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_write_miss)
        INSTANTIATE_GET_SET(uint64_t, stat_copyback_miss)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_request);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_copyback);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_mshr_buffer_prefetch);


        inline void add_stat_instruction_miss(uint64_t born_cycle) {
            this->stat_instruction_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_miss(uint64_t born_cycle) {
            this->stat_read_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_miss(uint64_t born_cycle) {
            this->stat_prefetch_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };

        inline void add_stat_write_miss(uint64_t born_cycle) {
            this->stat_write_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_miss(uint64_t born_cycle) {
            this->stat_copyback_miss++;
            uint64_t new_time = (sinuca_engine.get_global_cycle() - born_cycle);
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };
};

/// ============================================================================
/// Main Memories
/// ============================================================================
class main_memory_t : public interconnection_interface_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t channel_number;
        uint32_t total_channels;
        uint32_t banks_per_channel;
        main_memory_mask_t address_mask_type;

        uint32_t line_size;
        uint32_t data_bus_latency;

        uint32_t read_buffer_size;
        uint32_t write_buffer_size;

        uint32_t bank_penalty_ras;
        uint32_t bank_penalty_cas;

        uint32_t row_buffer_size;
        row_buffer_t row_buffer_policy;
        write_priority_t write_priority_policy;

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================
        uint64_t row_bits_mask;
        uint64_t row_bits_shift;

        uint64_t bank_bits_mask;
        uint64_t bank_bits_shift;

        uint64_t channel_bits_mask;
        uint64_t channel_bits_shift;

        uint64_t column_bits_mask;
        uint64_t not_column_bits_mask;
        uint64_t column_bits_shift;

        uint64_t data_bus_ready;
        uint64_t read_ready;    /// Ready to receive new READ
        uint64_t write_ready;   /// Ready to receive new WRITE

        memory_package_t **read_buffer;
        memory_package_t **write_buffer;
        memory_package_t *row_buffer;

        uint32_t *read_buffer_position_used;
        uint32_t *write_buffer_position_used;

        bool drain_write;

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_accesses;
        uint64_t stat_open_new_row;
        uint64_t stat_full_read_buffer;
        uint64_t stat_full_write_buffer;

        uint64_t stat_instruction_completed;
        uint64_t stat_read_completed;
        uint64_t stat_prefetch_completed;
        uint64_t stat_write_completed;
        uint64_t stat_copyback_completed;

        uint64_t stat_min_instruction_wait_time;
        uint64_t stat_max_instruction_wait_time;
        uint64_t stat_acumulated_instruction_wait_time;

        uint64_t stat_min_read_wait_time;
        uint64_t stat_max_read_wait_time;
        uint64_t stat_acumulated_read_wait_time;

        uint64_t stat_min_prefetch_wait_time;
        uint64_t stat_max_prefetch_wait_time;
        uint64_t stat_acumulated_prefetch_wait_time;

        uint64_t stat_min_write_wait_time;
        uint64_t stat_max_write_wait_time;
        uint64_t stat_acumulated_write_wait_time;

        uint64_t stat_min_copyback_wait_time;
        uint64_t stat_max_copyback_wait_time;
        uint64_t stat_acumulated_copyback_wait_time;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        main_memory_t();
        ~main_memory_t();

        /// ====================================================================
        /// Inheritance
        /// ====================================================================
        /// interconnection_interface_t
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port);
        void print_structures();
        void panic();
        void periodic_check();
        /// ====================================================================
        /// statistics_t
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        /// MASKS
        void set_masks();

        inline uint64_t get_row(uint64_t addr) {
            return (addr & this->row_bits_mask) >> this->row_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }

        inline uint64_t get_channel(uint64_t addr) {
            return (addr & this->channel_bits_mask) >> this->channel_bits_shift;
        }

        inline uint64_t get_column(uint64_t addr) {
            return (addr & this->column_bits_mask) >> this->column_bits_shift;
        }

        inline uint32_t cmp_row_bank_channel(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_column_bits_mask) == (memory_addressB & this->not_column_bits_mask);
        }

        void find_cas_and_ras(memory_package_t **input_buffer, uint32_t input_buffer_size, uint32_t bank, int32_t& cas_position, int32_t& ras_position);

        INSTANTIATE_GET_SET(uint32_t, channel_number)
        INSTANTIATE_GET_SET(uint32_t, total_channels)
        INSTANTIATE_GET_SET(uint32_t, banks_per_channel)
        INSTANTIATE_GET_SET(main_memory_mask_t, address_mask_type)

        INSTANTIATE_GET_SET(uint32_t, line_size)

        INSTANTIATE_GET_SET(uint32_t, data_bus_latency)
        INSTANTIATE_GET_SET(uint32_t, read_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, write_buffer_size)

        INSTANTIATE_GET_SET(uint64_t, data_bus_ready)
        INSTANTIATE_GET_SET(uint64_t, read_ready)
        INSTANTIATE_GET_SET(uint64_t, write_ready)

        INSTANTIATE_GET_SET(uint32_t, bank_penalty_ras)
        INSTANTIATE_GET_SET(uint32_t, bank_penalty_cas)

        INSTANTIATE_GET_SET(uint32_t, row_buffer_size)
        INSTANTIATE_GET_SET(row_buffer_t, row_buffer_policy)
        INSTANTIATE_GET_SET(write_priority_t, write_priority_policy)

        INSTANTIATE_GET_SET(bool, drain_write)
        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_open_new_row)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_read_buffer)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_write_buffer)

        INSTANTIATE_GET_SET(uint64_t, stat_instruction_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_read_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_prefetch_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_write_completed)
        INSTANTIATE_GET_SET(uint64_t, stat_copyback_completed)

        inline void add_stat_instruction_completed(uint64_t born_cycle) {
            this->stat_instruction_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_instruction_wait_time += new_time;
            if (this->stat_min_instruction_wait_time > new_time) this->stat_min_instruction_wait_time = new_time;
            if (this->stat_max_instruction_wait_time < new_time) this->stat_max_instruction_wait_time = new_time;
        };

        inline void add_stat_read_completed(uint64_t born_cycle) {
            this->stat_read_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_read_wait_time += new_time;
            if (this->stat_min_read_wait_time > new_time) this->stat_min_read_wait_time = new_time;
            if (this->stat_max_read_wait_time < new_time) this->stat_max_read_wait_time = new_time;
        };

        inline void add_stat_prefetch_completed(uint64_t born_cycle) {
            this->stat_prefetch_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_prefetch_wait_time += new_time;
            if (this->stat_min_prefetch_wait_time > new_time) this->stat_min_prefetch_wait_time = new_time;
            if (this->stat_max_prefetch_wait_time < new_time) this->stat_max_prefetch_wait_time = new_time;
        };


        inline void add_stat_write_completed(uint64_t born_cycle) {
            this->stat_write_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_write_wait_time += new_time;
            if (this->stat_min_write_wait_time > new_time) this->stat_min_write_wait_time = new_time;
            if (this->stat_max_write_wait_time < new_time) this->stat_max_write_wait_time = new_time;
        };

        inline void add_stat_copyback_completed(uint64_t born_cycle) {
            this->stat_copyback_completed++;
            uint64_t new_time = sinuca_engine.get_global_cycle() - born_cycle;
            this->stat_acumulated_copyback_wait_time += new_time;
            if (this->stat_min_copyback_wait_time > new_time) this->stat_min_copyback_wait_time = new_time;
            if (this->stat_max_copyback_wait_time < new_time) this->stat_max_copyback_wait_time = new_time;
        };

};

/// ============================================================================
/// Utils
/// ============================================================================
 /*! Provides useful generic methods used in multiple components like
  * constructors and destructors, number conversors, pretty print outs.
  */
class utils_t {
    public:
        /// ====================================================================
        template <class TYPE>
        static void template_delete_variable(TYPE *variable) {
            /// Deallocate
            if (variable) {
                delete variable;
            }
        };
        /// ====================================================================
        template <class TYPE>
        static void template_delete_array(TYPE *array) {
            /// Deallocate
            if (array) {
                delete[] array;
            }
        };
        /// ====================================================================
        template <class TYPE>
        static void template_delete_matrix(TYPE **matrix, uint32_t count) {
            /// Deallocate
            if (count != 0 && matrix[0] != NULL) {
                delete[] matrix[0];
                delete[] matrix;
            }
        };


        /// ====================================================================
        template <class TYPE>
        static TYPE* template_allocate_array(uint32_t count) {
            ERROR_ASSERT_PRINTF(count > 0, "Allocating array of %u positions\n", count);
            /// Allocate
            TYPE *var = new TYPE[count];
            return var;
        };

        /// ====================================================================
        template <class TYPE>
        static TYPE* template_allocate_initialize_array(uint32_t count, TYPE init) {
            ERROR_ASSERT_PRINTF(count > 0, "Allocating array of %u positions\n", count);
            /// Allocate
            TYPE *var = new TYPE[count];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Initialize
            for (uint32_t position = 0; position < count; position++) {
                var[position] = init;
            };
            return var;
        };

        /// ====================================================================
        template <class TYPE>
        static TYPE** template_allocate_matrix(uint32_t count_x, uint32_t count_y) {
            ERROR_ASSERT_PRINTF(count_x > 0, "Allocating matrix of %u(x) positions\n", count_x);
            ERROR_ASSERT_PRINTF(count_y > 0, "Allocating matrix of %u(y) positions\n", count_y);
            /// Allocate the pointers
            TYPE **var = new TYPE*[count_x];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Allocate all contiguously
            var[0] = new TYPE[count_x*count_y];
            ERROR_ASSERT_PRINTF(var[0] != NULL, "Could not allocate memory\n");
            /// Distribute over the positions
            for (uint32_t line = 1; line < count_x; line++) {
                var[line] = var[0] + (count_y * line);
            };
            return var;
        };

        /// ====================================================================
        template <class TYPE>
        static TYPE** template_allocate_initialize_matrix(uint32_t count_x, uint32_t count_y, TYPE init) {
            ERROR_ASSERT_PRINTF(count_x > 0, "Allocating matrix of %u(x) positions\n", count_x);
            ERROR_ASSERT_PRINTF(count_y > 0, "Allocating matrix of %u(y) positions\n", count_y);
            /// Allocate the pointers
            TYPE **var = new TYPE*[count_x];
            ERROR_ASSERT_PRINTF(var != NULL, "Could not allocate memory\n");
            /// Allocate all contiguously
            var[0] = new TYPE[count_x*count_y];
            ERROR_ASSERT_PRINTF(var[0] != NULL, "Could not allocate memory\n");
            /// Initialize
            for (uint32_t position = 0; position < count_x*count_y; position++) {
                var[0][position] = init;
            };
            /// Distribute over the positions
            for (uint32_t line = 1; line < count_x; line++) {
                var[line] = var[0] + (count_y * line);
            };
            return var;
        };

        static uint64_t get_power_of_two(uint64_t n);
        static uint8_t check_if_power_of_two(uint64_t n);
        static uint64_t hash_function(hash_function_t type, uint64_t input1, uint64_t input2, uint64_t bit_size);
        static uint64_t fill_bit(uint32_t start, uint32_t end);

        static const char *print_mask_of_bits(uint32_t line_size, uint32_t line_number, uint32_t assoc);
        static const char *int32_to_char(int32_t input_int);
        static const char *uint32_to_char(uint32_t input_int);
        static const char *uint64_to_char(uint64_t input_int);
        static const char *int64_to_char(int64_t input_int);

        static std::string progress_pretty(uint64_t actual, uint64_t total);
        static std::string connections_pretty(cache_memory_t *cache_memory, uint32_t level);

        static std::string address_to_binary(uint64_t address);
        static std::string uint32_to_string(uint32_t input_int);
        static std::string uint64_to_string(uint64_t input_int);
        static std::string int64_to_string(int64_t input_int);

        static void process_mem_usage(double& stat_vm_usage, double& resident_set);
};

#endif  // _SINUCA_SINUCA_HPP_
