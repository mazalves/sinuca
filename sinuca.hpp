/// ============================================================================
///
/// Copyright (C) 2010, 2011, 2012
/// Marco Antonio Zanata Alves
///
/// GPPD - Parallel and Distributed Processing Group
/// Universidade Federal do Rio Grande do Sul
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License as published by the
/// Free Software Foundation; either version 2 of the License, or (at your
/// option) any later version.
///
/// This program is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along
/// with this program; if not, write to the Free Software Foundation, Inc.,
/// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
///
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
#include <fstream>
#include <limits>
#include <vector>
#include <string>

/// Embedded Libraries
#include "./libs/include/zlib.h"
#include "./libs/include/libconfig.h++"

/// ============================================================================
/// Classes
/// ============================================================================
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
/// Processor
class branch_predictor_t;
class reorder_buffer_line_t;
class processor_t;
/// Directory
class directory_line_t;
class directory_controller_t;
/// Cache and Main Memory
class prefetch_t;
class prefetch_stride_t;
class prefetch_disable_t;

class line_usage_predictor_t;
class cache_line_t;
class cache_set_t;
class cache_memory_t;

class memory_channel_t;
class memory_controller_t;
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
#define TRACE_LINE_SIZE 512     /// trace_reader_t (should not be smaller than on trace line)
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
    // ~ #define LINE_USAGE_PREDICTOR_DEBUG
    // ~ #define MEMORY_CONTROLLER_DEBUG
    // ~ #define ROUTER_DEBUG
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
typedef std::vector <const char*>                   container_ptr_const_char_t;

typedef std::vector <token_t>                       container_token_t;
typedef std::vector <opcode_package_t>              container_opcode_package_t;
typedef std::vector <container_opcode_package_t>    container_static_dictionary_t;

typedef std::vector <memory_package_t*>             container_ptr_memory_package_t;
typedef std::vector <reorder_buffer_line_t*>        container_ptr_reorder_buffer_line_t;
typedef std::vector <directory_line_t*>  container_ptr_directory_line_t;
typedef std::vector <cache_memory_t*>               container_ptr_cache_memory_t;
typedef std::vector <memory_controller_t*>          container_ptr_memory_controller_t;


/// Sinuca Headers
#include "./enumerations.hpp"
#include "./trace_reader/trace_reader.hpp"

#include "./packages/opcode_package.hpp"
#include "./packages/uop_package.hpp"
#include "./packages/memory_package.hpp"

#include "./sinuca_engine.hpp"

/// ============================================================================
/// Interconnection Components
/// ============================================================================
#include "./interconnection/token.hpp"
#include "./interconnection/interconnection_interface.hpp"

#include "./interconnection/edge.hpp"
#include "./interconnection/routing_table_element.hpp"
#include "./interconnection/interconnection_controller.hpp"
#include "./interconnection/interconnection_router.hpp"

/// ============================================================================
/// Processor Cores Components
/// ============================================================================
#include "./branch_predictor/branch_target_buffer_line.hpp"
#include "./branch_predictor/branch_target_buffer_set.hpp"
#include "./branch_predictor/branch_predictor.hpp"

#include "./processor/reorder_buffer_line.hpp"
#include "./processor/processor.hpp"


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

        /// Static Energy
        uint64_t *clock_become_alive;
        uint64_t *clock_become_dead;

        /// Copyback
        bool is_dirty;
        uint64_t *written_sub_blocks;

        /// Dead Flag
        uint32_t active_sub_blocks;
        bool is_dead;

        /// Dead Statistics
        uint64_t stat_total_dead_cycles;

        DSBP_metadata_line_t() {
            this->valid_sub_blocks = NULL;
            this->real_usage_counter = NULL;
            this->usage_counter = NULL;
            this->overflow = NULL;
            this->learn_mode = 0;
            this->PHT_pointer = NULL;
            this->clock_become_alive = NULL;
            this->clock_become_dead = NULL;
            this->is_dirty = false;
            this->written_sub_blocks = NULL;
            this->active_sub_blocks = 0;
            this->is_dead = true;
            this->stat_total_dead_cycles = 0;
        };
        ~DSBP_metadata_line_t() {
            if (this->valid_sub_blocks) delete [] valid_sub_blocks;
            if (this->real_usage_counter) delete [] real_usage_counter;
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_total_dead_cycles);
};

class DSBP_metadata_sets_t {
    public:
        DSBP_metadata_line_t *ways;
};


//##############################################################################


class DREC_PHT_line_t {
    public:
        uint64_t pc;
        uint64_t offset;
        uint64_t last_access;
        bool pointer;
        uint64_t *usage_counter;
        bool *overflow;

        DREC_PHT_line_t() {
            this->pc = 0;
            this->offset = 0;
            this->last_access = 0;
            this->pointer = 0;
            this->usage_counter = NULL;
            this->overflow = NULL;
        };
        ~DREC_PHT_line_t() {
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };
};

class DREC_PHT_sets_t {
    public:
        DREC_PHT_line_t *ways;
};


class DREC_metadata_line_t {
    public:
        line_sub_block_t *valid_sub_blocks;
        uint64_t *real_usage_counter;
        uint64_t *usage_counter;
        bool *overflow;
        bool learn_mode;
        DREC_PHT_line_t *PHT_pointer;

        /// Static Energy
        uint64_t *clock_become_alive;
        uint64_t *clock_become_dead;

        /// Copyback
        bool is_dirty;
        uint64_t *written_sub_blocks;

        /// Dead Flag
        uint32_t active_sub_blocks;
        bool is_dead;

        /// Dead Statistics
        uint64_t stat_total_dead_cycles;

        DREC_metadata_line_t() {
            this->valid_sub_blocks = NULL;
            this->real_usage_counter = NULL;
            this->usage_counter = NULL;
            this->overflow = NULL;
            this->learn_mode = 0;
            this->PHT_pointer = NULL;
            this->clock_become_alive = NULL;
            this->clock_become_dead = NULL;
            this->is_dirty = false;
            this->written_sub_blocks = NULL;
            this->active_sub_blocks = 0;
            this->is_dead = true;
            this->stat_total_dead_cycles = 0;
        };
        ~DREC_metadata_line_t() {
            if (this->valid_sub_blocks) delete [] valid_sub_blocks;
            if (this->real_usage_counter) delete [] real_usage_counter;
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_total_dead_cycles);
};

class DREC_metadata_sets_t {
    public:
        DREC_metadata_line_t *ways;
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
            /// DREC
            /// ====================================================================
            uint32_t DREC_line_number;          /// Cache Metadata
            uint32_t DREC_associativity;        /// Cache Metadata

            uint32_t DREC_sub_block_size;
            uint32_t DREC_usage_counter_bits;

            /// PHT Misses
            uint32_t DREC_PHTM_line_number;
            uint32_t DREC_PHTM_associativity;
            replacement_t DREC_PHTM_replacement_policy;

            /// PHT Copy Back
            uint32_t DREC_PHTCB_line_number;
            uint32_t DREC_PHTCB_associativity;
            replacement_t DREC_PHTCB_replacement_policy;
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
            /// DREC
            /// ====================================================================
            DREC_metadata_sets_t *DREC_sets;
            uint32_t DREC_total_sets;
            uint32_t DREC_sub_block_total;
            uint32_t DREC_usage_counter_max;

            DREC_PHT_sets_t *DREC_PHTM_sets;
            uint32_t DREC_PHTM_total_sets;
            uint64_t DREC_PHTM_index_bits_mask;

            DREC_PHT_sets_t *DREC_PHTCB_sets;
            uint32_t DREC_PHTCB_total_sets;
            uint64_t DREC_PHTCB_index_bits_mask;


        /// ====================================================================
        /// Statistics related
        /// ====================================================================
            uint64_t stat_DSBP_line_sub_block_disable_always;
            uint64_t stat_DSBP_line_sub_block_disable_turnoff;
            uint64_t stat_DSBP_line_sub_block_normal_correct;
            uint64_t stat_DSBP_line_sub_block_normal_over;
            uint64_t stat_DSBP_line_sub_block_learn;
            uint64_t stat_DSBP_line_sub_block_wrong_first;
            uint64_t stat_DSBP_line_sub_block_copyback;

            uint64_t stat_line_miss;
            uint64_t stat_sub_block_miss;
            uint64_t stat_copyback;
            uint64_t stat_eviction;

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

            /// Number of sub_blocks written
            uint64_t *stat_written_sub_blocks_per_line;
            uint64_t stat_dirty_lines_predicted_dead;
            uint64_t stat_clean_lines_predicted_dead;
            uint64_t stat_written_lines_miss_predicted;

            /// Number of times each sub_block was written before eviction
            uint64_t stat_writes_per_sub_blocks_1;
            uint64_t stat_writes_per_sub_blocks_2;
            uint64_t stat_writes_per_sub_blocks_3;
            uint64_t stat_writes_per_sub_blocks_4;
            uint64_t stat_writes_per_sub_blocks_5;
            uint64_t stat_writes_per_sub_blocks_10;
            uint64_t stat_writes_per_sub_blocks_100;
            uint64_t stat_writes_per_sub_blocks_bigger;

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
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        void allocate_token_list();
        bool check_token_list(memory_package_t *package);
        uint32_t check_token_space(memory_package_t *package);
        void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
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
        void line_insert_copyback(memory_package_t *package, cache_memory_t *cache_memory, cache_line_t *cache_line, uint32_t index, uint32_t way);
        void line_get_copyback(memory_package_t *package, uint32_t index, uint32_t way);
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
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_DSBP_line_sub_block_copyback);

            INSTANTIATE_GET_SET_ADD(uint64_t, stat_line_miss);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_sub_block_miss);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_copyback);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_eviction);

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


            INSTANTIATE_GET_SET_ADD(uint64_t, stat_dirty_lines_predicted_dead);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_clean_lines_predicted_dead);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_written_lines_miss_predicted);

            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_1);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_2);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_3);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_4);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_5);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_10);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_100);
            INSTANTIATE_GET_SET_ADD(uint64_t, stat_writes_per_sub_blocks_bigger);
};


/// ============================================================================
/// Memory Devices
/// ============================================================================
#include "./directory/directory_line.hpp"
#include "./directory/directory_controller.hpp"

#include "./prefetch/reference_prediction_line.hpp"

#include "./prefetch/prefetcher.hpp"
#include "./prefetch/prefetcher_stride.hpp"
#include "./prefetch/prefetcher_disable.hpp"


#include "./cache_memory/cache_line.hpp"
#include "./cache_memory/cache_set.hpp"
#include "./cache_memory/cache_memory.hpp"

#include "./main_memory/memory_channel.hpp"
#include "./main_memory/memory_controller.hpp"

/// ============================================================================
/// Utils Methods
/// ============================================================================
#include "./utils.hpp"

#endif  // _SINUCA_SINUCA_HPP_
