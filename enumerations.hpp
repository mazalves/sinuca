//==============================================================================
//
// Copyright (C) 2010, 2011
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
/// Main header file: sinuca.h
/// This is the main header file, it contains all the prototypes and
/// describes the relations between classes.
#ifndef _SINUCA_ENUMERATOR_HPP_
#define _SINUCA_ENUMERATOR_HPP_

///==============================================================================
/// Enumarations
///==============================================================================

/// ============================================================================
/// Enumarates the INSTRUCTION operation type as NOP | INT_* | FLOAT_* | BRANCH | MEM_* | OTHER
enum instruction_operation_t {
    /// NOP
    INSTRUCTION_OPERATION_NOP,
    /// INTEGERS
    INSTRUCTION_OPERATION_INT_ALU,
    INSTRUCTION_OPERATION_INT_MUL,
    INSTRUCTION_OPERATION_INT_DIV,
    /// FLOAT POINT
    INSTRUCTION_OPERATION_FP_ALU,
    INSTRUCTION_OPERATION_FP_MUL,
    INSTRUCTION_OPERATION_FP_DIV,
    /// BRANCHES
    INSTRUCTION_OPERATION_BRANCH,
    /// MEMORY OPERATIONS
    INSTRUCTION_OPERATION_MEM_LOAD,
    INSTRUCTION_OPERATION_MEM_STORE,
    /// NOT INDENTIFIED
    INSTRUCTION_OPERATION_OTHER,
    /// SYNCRONIZATION
    INSTRUCTION_OPERATION_BARRIER
};
const char* get_enum_instruction_operation_char(instruction_operation_t type);


/// ============================================================================
/// Enumarates the MEMORY operation type as READ | WRITE | INSTRUCTION | ...
enum memory_operation_t {
    MEMORY_OPERATION_INST,
    MEMORY_OPERATION_READ,
    MEMORY_OPERATION_WRITE,
    MEMORY_OPERATION_PREFETCH,
    MEMORY_OPERATION_COPYBACK
};
const char *get_enum_memory_operation_char(memory_operation_t type);

/// ============================================================================
/// Used to describe the package status when it arrives on the components
enum package_state_t {
    PACKAGE_STATE_FREE,
    PACKAGE_STATE_UNTREATED,
    PACKAGE_STATE_READY,
    PACKAGE_STATE_WAIT,
    PACKAGE_STATE_TRANSMIT
};
const char *get_enum_package_state_char(package_state_t type);

/// ============================================================================
/// Enumarates the protocol status as M/O/E/S/I/V.
enum protocol_status_t {
    PROTOCOL_STATUS_M,
    PROTOCOL_STATUS_O,
    PROTOCOL_STATUS_E,
    PROTOCOL_STATUS_S,
    PROTOCOL_STATUS_I
};
const char *get_enum_protocol_status_char(protocol_status_t type);

/// ============================================================================
/// Open the file parser_components.h and gets the types of components
enum component_t {
    COMPONENT_PROCESSOR,
    COMPONENT_CACHE_MEMORY,
    COMPONENT_MAIN_MEMORY,
    COMPONENT_INTERCONNECTION_ROUTER,
    COMPONENT_DIRECTORY_CONTROLLER,
    COMPONENT_INTERCONNECTION_CONTROLLER,
    COMPONENT_NUMBER
};
const char *get_enum_component_char(component_t type);

/// ============================================================================
/// Enumarates the types of hash function
enum hash_function_t {
    HASH_FUNCTION_XOR_SIMPLE,
    HASH_FUNCTION_INPUT1_ONLY,
    HASH_FUNCTION_INPUT2_ONLY
};
const char *get_enum_hash_function_char(hash_function_t type);

/// ============================================================================
/// Enumarates the type of branch predictor
enum branch_predictor_policy_t {
    BRANCH_PREDICTOR_TWO_LEVEL,
    BRANCH_PREDICTOR_STATIC_TAKEN,
    BRANCH_PREDICTOR_DISABLE
};
const char *get_enum_branch_predictor_policy_char(branch_predictor_policy_t type);

/// ============================================================================
/// Enumarates the processor stages, used to indicate when the branch will be solved.
enum processor_stage_t {
    PROCESSOR_STAGE_FETCH,
    PROCESSOR_STAGE_DECODE,
    PROCESSOR_STAGE_RENAME,
    PROCESSOR_STAGE_DISPATCH,
    PROCESSOR_STAGE_EXECUTION,
    PROCESSOR_STAGE_COMMIT
};
const char *get_enum_processor_stage_char(processor_stage_t type);

/// ============================================================================
/// Enumarates the synchronization type required by the dynamic trace.
enum processor_port_t {
    PROCESSOR_PORT_DATA_CACHE,
    PROCESSOR_PORT_INST_CACHE,
    PROCESSOR_PORT_NUMBER
};

/// ============================================================================
/// Enumarates the synchronization type required by the dynamic trace.
enum sync_t {
    SYNC_BARRIER,
    SYNC_WAIT_CRITICAL_START,
    SYNC_CRITICAL_START,
    SYNC_CRITICAL_END,
    SYNC_FREE
};
const char *get_enum_sync_char(sync_t type);

/// ============================================================================
/// Enumarates the selection policy to pick a sender or next to be treated.
enum selection_t {
    SELECTION_RANDOM,
    SELECTION_ROUND_ROBIN
};
const char *get_enum_selection_char(selection_t type);

/// ============================================================================
/// Enumarates the router selection policy to pick a sender.
enum routing_algorithm_t {
    ROUTING_ALGORITHM_XY,
    ROUTING_ALGORITHM_ODD_EVEN,
    ROUTING_ALGORITHM_FLOYD_WARSHALL
};
const char *get_enum_routing_algorithm_char(routing_algorithm_t type);

/// ============================================================================
/// Cache replacement policy
enum replacement_t {
    REPLACEMENT_LRU,     /// way list order: MRU -> LRU
    REPLACEMENT_RANDOM,  /// way list order: arbitrary
    REPLACEMENT_FIFO,    /// way list order: oldest block -> newest block
    REPLACEMENT_LRF      /// way list order: Least recently filled
};
const char *get_enum_replacement_char(replacement_t type);

/// ============================================================================
/// Directory coherence protocol
enum coherence_protocol_t {
    COHERENCE_PROTOCOL_MOESI
};
const char *get_enum_coherence_protocol_char(coherence_protocol_t type);

/// ============================================================================
/// Directory coherence protocol
enum inclusiveness_t {
    INCLUSIVENESS_NON_INCLUSIVE
};
const char *get_enum_inclusiveness_char(inclusiveness_t type);

/// ============================================================================
/// Prefetcher type
enum prefetch_policy_t {
    PREFETCHER_STREAM,
    PREFETCHER_DISABLE
};
const char *get_enum_prefetch_policy_char(prefetch_policy_t type);

/// ============================================================================
/// Prefetcher type
enum full_buffer_t {
    FULL_BUFFER_OVERRIDE,
    FULL_BUFFER_STOP
};
const char *get_enum_full_buffer_char(full_buffer_t type);


/// ============================================================================
/// Directory Lock type
enum lock_t {
    LOCK_FREE,
    LOCK_READ,
    LOCK_WRITE
};
const char *get_enum_lock_char(lock_t type);

/// ============================================================================
/// How the memory cache will create its address mask
enum cache_mask_t {
    CACHE_MASK_TAG_INDEX_OFFSET,
    CACHE_MASK_TAG_INDEX_BANK_OFFSET
};
const char *get_enum_cache_mask_char(cache_mask_t type);

/// ============================================================================
/// How the main memory will create its address mask
enum main_memory_mask_t {
    MAIN_MEMORY_MASK_ROW_BANK_COLUMN,
    MAIN_MEMORY_MASK_ROW_BANK_CHANNEL_COLUMN
};
const char *get_enum_main_memory_mask_char(main_memory_mask_t type);

/// ============================================================================
/// Policy to set the priority during the Row Buffer access
enum row_buffer_t {
    ROW_BUFFER_HITS_FIRST,
    ROW_BUFFER_FIFO
};
const char *get_enum_row_buffer_char(row_buffer_t type);

/// ============================================================================
/// Policy to give privilege of some operations over others
enum write_priority_t {
    WRITE_PRIORITY_SERVICE_AT_NO_READ,
    WRITE_PRIORITY_DRAIN_WHEN_FULL,
};
const char *get_enum_write_priority_char(write_priority_t type);

#endif  // _SINUCA_ENUMERATOR_HPP_
