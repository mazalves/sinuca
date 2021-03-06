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
#include <execinfo.h>
#include <cxxabi.h>

/// C++ Includes
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <limits>
#include <vector>
#include <string>
#include <unordered_map>

/// Embedded Libraries
#include "./extra_libs/include/zlib.h"
#include "./extra_libs/include/libconfig.h++"

// ============================================================================
/// Classes
// ============================================================================
class trace_reader_t;
class sinuca_engine_t;
/// Packages
class opcode_package_t;
class uop_package_t;
class memory_package_t;
/// Internal for (almost) all components
class token_t;
class interconnection_interface_t;
/// Interconnection
class interconnection_controller_t;
class interconnection_router_t;
/// Branch Predictor
class branch_target_buffer_line_t;
class branch_target_buffer_set_t;
class branch_predictor_t;
class branch_predictor_two_level_gag_t;
class branch_predictor_two_level_gas_t;
class branch_predictor_two_level_pag_t;
class branch_predictor_two_level_pas_t;
class branch_predictor_bi_modal_t;
class branch_predictor_static_taken_t;
class branch_predictor_perfect_t;
class branch_predictor_disable_t;
/// Processor
class memory_order_buffer_line_t;
class reorder_buffer_line_t;
class processor_t;
/// Directory
class directory_line_t;
class directory_controller_t;
/// Prefetch
class prefetch_t;
class stride_table_line_t;
class prefetch_stride_t;
class stream_table_line_t;
class prefetch_stream_t;
class prefetch_disable_t;
/// Line Usage Predictor
class line_usage_predictor_t;
class line_usage_predictor_disable_t;
/// === DEWP
class aht_line_t;
class aht_set_t;
class dewp_metadata_line_t;
class dewp_metadata_set_t;
class line_usage_predictor_dewp_t;
class line_usage_predictor_dewp_oracle_t;
/// === SDP
class skewed_metadata_line_t;
class skewed_metadata_set_t;
class line_usage_predictor_skewed_t;
/// Cache Memory
class cache_line_t;
class cache_set_t;
class cache_memory_t;
/// Main Memory
class memory_channel_t;
class memory_controller_t;
/// Useful static methods
class utils_t;
template<class CB_TYPE> class circular_buffer_t;

// ============================================================================
/// Global SINUCA_ENGINE instantiation
// ============================================================================
extern sinuca_engine_t sinuca_engine;

// ============================================================================
/// Definitions for Log, Debug, Warning, Error and Statistics
// ============================================================================
#define HEART_BEAT      10000000  /// Period to inform the Progress
#define MAX_ALIVE_TIME   1000000  /// Max Time for a request to be solved
#define PERIODIC_CHECK  10000000  /// Period between the Periodic Check
// ~ #define PERIODIC_CHECK  1  /// Period between the Periodic Check

// ~ #define INITIALIZE_DEBUG 100000    /// Cycle to start the DEBUG_PRINTF (0 to disable)
#define INITIALIZE_DEBUG 0    /// Cycle to start the DEBUG_PRINTF (0 to disable)
#define FINALIZE_DEBUG 0      /// Cycle to stop the DEBUG_PRINTF (0 to disable)

/// Class specific definitions
#define INFINITE (std::numeric_limits<uint32_t>::max())             /// interconnection_controller_t
#define UNDESIRABLE (sinuca_engine.get_interconnection_interface_array_size()) /// interconnection_controller_t
#define TRACE_LINE_SIZE 512     /// trace_reader_t (should not be smaller than on trace line)
#define CONVERSION_SIZE 65
#define MAX_UOP_DECODED 5       /// processor_t (Max number of uops from one opcode)
#define MAX_REGISTERS 6         /// opcode_package_t uop_package_t  (Max number of register (read or write) for one opcode/uop)
#define MAX_ASSEMBLY_SIZE 32    /// In general 20 is enough
#define MAX_CORES 32            /// Define the max number of cores for the set affinity
#define PAGE_SIZE 4096          /// Pages have always 4KB

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
    #define CACHE_DEBUG
    // ~ #define PREFETCHER_DEBUG
    // ~ #define LINE_USAGE_PREDICTOR_DEBUG
    #define MEMORY_CONTROLLER_DEBUG
    #define ROUTER_DEBUG
    // ~ #define INTERCONNECTION_CTRL_DEBUG
    #define DIRECTORY_CTRL_DEBUG
    // ~ #define SHOW_FREE_PACKAGE
    #define DEBUG_PRINTF(...)   {\
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
                                            SINUCA_PRINTF("ERROR: Cycle: %" PRIu64 "\n", sinuca_engine.get_global_cycle());\
                                            void* stack_ptrs[20]; \
                                            int stack_count = backtrace(stack_ptrs, 20); \
                                            SINUCA_PRINTF("\n");\
                                            SINUCA_PRINTF("ERROR: Stack size:%d\n", stack_count); \
                                            char** stack_funcNames = backtrace_symbols(stack_ptrs, stack_count);\
                                            for( int stack_it = 0; stack_it < stack_count; stack_it++ ) { \
                                                char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;\
                                                for (char *p = stack_funcNames[stack_it]; *p; ++p) {\
                                                    if (*p == '(') {        mangled_name = p;       }\
                                                    else if (*p == '+'){    offset_begin = p;       }\
                                                    else if (*p == ')'){    offset_end = p; break;  }\
                                                }\
                                                if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {\
                                                    *mangled_name++ = '\0';\
                                                    *offset_begin++ = '\0';\
                                                    *offset_end++ = '\0';\
                                                }\
                                                int status;\
                                                char * real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);\
                                                if (status == 0) {SINUCA_PRINTF("ERROR: Call(%d):%s\n", stack_it, real_name); }\
                                                else { SINUCA_PRINTF("ERROR: Call(%d):%s\n", stack_it, mangled_name); }\
                                            }\
                                            SINUCA_PRINTF("\n");\
                                            free(stack_funcNames); \
                                        }


    #define ERROR_ASSERT_PRINTF(v, ...) if (!(v)) {\
                                            ERROR_INFORMATION();\
                                            SINUCA_PRINTF("ERROR_ASSERT: %s\n", #v);\
                                            SINUCA_PRINTF("\nERROR: ");\
                                            SINUCA_PRINTF(__VA_ARGS__);\
                                            exit(EXIT_FAILURE);\
                                        }

    #define ERROR_PRINTF(...)           {\
                                            ERROR_INFORMATION();\
                                            SINUCA_PRINTF("\nERROR: ");\
                                            SINUCA_PRINTF(__VA_ARGS__);\
                                            exit(EXIT_FAILURE);\
                                        }
#else
    #define ERROR_INFORMATION(...)
    #define ERROR_ASSERT_PRINTF(v, ...)
    #define ERROR_PRINTF(...)
#endif

// ============================================================================
/// MACROS to create get_ and set_ methods for variables.
// ============================================================================
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

// ============================================================================
/// TYPES
// ============================================================================
typedef std::vector <uint64_t>                      container_uint64_t;
typedef std::vector <token_t>                       container_token_t;

typedef std::vector <const char*>                   container_ptr_const_char_t;
typedef std::vector <memory_package_t*>             container_ptr_memory_package_t;
typedef std::vector <reorder_buffer_line_t*>        container_ptr_reorder_buffer_line_t;
typedef std::vector <directory_line_t*>             container_ptr_directory_line_t;
typedef std::vector <cache_memory_t*>               container_ptr_cache_memory_t;
typedef std::vector <memory_controller_t*>          container_ptr_memory_controller_t;


/// Sinuca Headers
#include "./enumerations.hpp"
#include "./trace_reader/trace_reader.hpp"

#include "./packages/opcode_package.hpp"
#include "./packages/uop_package.hpp"
#include "./packages/memory_package.hpp"

#include "./sinuca_engine.hpp"

// ============================================================================
/// Interconnection Components
// ============================================================================
#include "./interconnection/token.hpp"
#include "./interconnection/interconnection_interface.hpp"

#include "./interconnection/edge.hpp"
#include "./interconnection/routing_table_element.hpp"
#include "./interconnection/interconnection_controller.hpp"
#include "./interconnection/interconnection_router.hpp"

// ============================================================================
/// Processor Cores Components
// ============================================================================
#include "./branch_predictor/branch_target_buffer_line.hpp"
#include "./branch_predictor/branch_target_buffer_set.hpp"

#include "./branch_predictor/branch_predictor.hpp"
#include "./branch_predictor/branch_predictor_two_level_gag.hpp"
#include "./branch_predictor/branch_predictor_two_level_gas.hpp"
#include "./branch_predictor/branch_predictor_two_level_pag.hpp"
#include "./branch_predictor/branch_predictor_two_level_pas.hpp"
#include "./branch_predictor/branch_predictor_bi_modal.hpp"
#include "./branch_predictor/branch_predictor_static_taken.hpp"
#include "./branch_predictor/branch_predictor_perfect.hpp"
#include "./branch_predictor/branch_predictor_disable.hpp"

#include "./processor/memory_order_buffer_line.hpp"
#include "./processor/reorder_buffer_line.hpp"
#include "./processor/processor.hpp"

// ============================================================================
/// Line Usage Predictor
// ============================================================================
#include "./line_usage_predictor/line_usage_predictor.hpp"
#include "./line_usage_predictor/line_usage_predictor_disable.hpp"

#include "./line_usage_predictor/aht_line.hpp"
#include "./line_usage_predictor/aht_set.hpp"
#include "./line_usage_predictor/dewp_metadata_line.hpp"
#include "./line_usage_predictor/dewp_metadata_set.hpp"
#include "./line_usage_predictor/line_usage_predictor_dewp.hpp"
#include "./line_usage_predictor/line_usage_predictor_dewp_oracle.hpp"

#include "./line_usage_predictor/skewed_metadata_line.hpp"
#include "./line_usage_predictor/skewed_metadata_set.hpp"
#include "./line_usage_predictor/line_usage_predictor_skewed.hpp"

// ============================================================================
/// Memory Devices
// ============================================================================
#include "./directory/directory_line.hpp"
#include "./directory/directory_controller.hpp"

#include "./prefetch/stride_table_line.hpp"
#include "./prefetch/stream_table_line.hpp"

#include "./prefetch/prefetcher.hpp"
#include "./prefetch/prefetcher_stride.hpp"
#include "./prefetch/prefetcher_stream.hpp"
#include "./prefetch/prefetcher_disable.hpp"

#include "./cache_memory/cache_line.hpp"
#include "./cache_memory/cache_set.hpp"
#include "./cache_memory/cache_memory.hpp"

#include "./main_memory/memory_channel.hpp"
#include "./main_memory/memory_controller.hpp"

// ============================================================================
/// Useful Methods
// ============================================================================
#include "./utils.hpp"
#include "./circular_buffer.hpp"

#endif  // _SINUCA_SINUCA_HPP_
