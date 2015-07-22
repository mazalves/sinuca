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
#ifndef _SINUCA_ENUMERATOR_HPP_
#define _SINUCA_ENUMERATOR_HPP_

// ==============================================================================
/// Enumerations
// ==============================================================================

// ============================================================================
/// Enumerates the INSTRUCTION (Opcode and Uop) operation type
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
    /// NOT IDENTIFIED
    INSTRUCTION_OPERATION_OTHER,
    /// SYNCHRONIZATION
    INSTRUCTION_OPERATION_BARRIER,
    /// HMC
    INSTRUCTION_OPERATION_HMC_ALU,     //#12 No Answer is sent back
    INSTRUCTION_OPERATION_HMC_ALUR     //#13 Request Pay-Load
};
const char* get_enum_instruction_operation_char(instruction_operation_t type);

// ============================================================================
/// Enumerates the MEMORY operation type
enum memory_operation_t {
    MEMORY_OPERATION_INST,
    MEMORY_OPERATION_READ,
    MEMORY_OPERATION_WRITE,
    MEMORY_OPERATION_PREFETCH,
    MEMORY_OPERATION_WRITEBACK,
    /// HMC
    MEMORY_OPERATION_HMC_ALU,
    MEMORY_OPERATION_HMC_ALUR,

    MEMORY_OPERATION_HMC_LOAD,
    MEMORY_OPERATION_HMC_STORE
};
const char *get_enum_memory_operation_char(memory_operation_t type);

// ============================================================================
/// Enumerates the package status when it arrives on the components
enum package_state_t {
    PACKAGE_STATE_FREE,
    PACKAGE_STATE_UNTREATED,
    PACKAGE_STATE_READY,
    PACKAGE_STATE_WAIT,
    PACKAGE_STATE_TRANSMIT
};
const char *get_enum_package_state_char(package_state_t type);

// ============================================================================
/// Enumarates the data and instruction ports inside the processor
enum processor_port_t {
    PROCESSOR_PORT_DATA_CACHE,
    PROCESSOR_PORT_INST_CACHE,
    PROCESSOR_PORT_NUMBER
};

// ============================================================================
/// Enumerates the coherence protocol status
enum protocol_status_t {
    PROTOCOL_STATUS_M,
    PROTOCOL_STATUS_O,
    PROTOCOL_STATUS_E,
    PROTOCOL_STATUS_S,
    PROTOCOL_STATUS_I
};
const char *get_enum_protocol_status_char(protocol_status_t type);

// ============================================================================
/// Enumerates the types of components
enum component_t {
    COMPONENT_PROCESSOR,
    COMPONENT_CACHE_MEMORY,
    COMPONENT_MEMORY_CONTROLLER,
    COMPONENT_INTERCONNECTION_ROUTER,
    COMPONENT_DIRECTORY_CONTROLLER,
    COMPONENT_INTERCONNECTION_CONTROLLER,
    COMPONENT_NUMBER
};
const char *get_enum_component_char(component_t type);

// ============================================================================
/// Enumerates the types of hash function
enum hash_function_t {
    HASH_FUNCTION_XOR_SIMPLE,
    HASH_FUNCTION_INPUT1_ONLY,
    HASH_FUNCTION_INPUT2_ONLY,
    HASH_FUNCTION_INPUT1_2BITS,
    HASH_FUNCTION_INPUT1_4BITS,
    HASH_FUNCTION_INPUT1_8BITS,
    HASH_FUNCTION_INPUT1_16BITS,
    HASH_FUNCTION_INPUT1_32BITS
};
const char *get_enum_hash_function_char(hash_function_t type);

// ============================================================================
/// Enumerates the type of branch predictor
enum branch_predictor_policy_t {
    BRANCH_PREDICTOR_TWO_LEVEL_GAG,
    BRANCH_PREDICTOR_TWO_LEVEL_GAS,
    BRANCH_PREDICTOR_TWO_LEVEL_PAG,
    BRANCH_PREDICTOR_TWO_LEVEL_PAS,
    BRANCH_PREDICTOR_BI_MODAL,
    BRANCH_PREDICTOR_STATIC_TAKEN,
    BRANCH_PREDICTOR_PERFECT,
    BRANCH_PREDICTOR_DISABLE
};
const char *get_enum_branch_predictor_policy_char(branch_predictor_policy_t type);

// ============================================================================
/// Enumerates the processor stages, used to indicate when the branch will be solved
enum processor_stage_t {
    PROCESSOR_STAGE_FETCH,
    PROCESSOR_STAGE_DECODE,
    PROCESSOR_STAGE_RENAME,
    PROCESSOR_STAGE_DISPATCH,
    PROCESSOR_STAGE_EXECUTION,
    PROCESSOR_STAGE_COMMIT
};
const char *get_enum_processor_stage_char(processor_stage_t type);

// ============================================================================
/// Enumerates the synchronization type required by the dynamic trace.
enum sync_t {
    SYNC_BARRIER,
    SYNC_WAIT_CRITICAL_START,
    SYNC_CRITICAL_START,
    SYNC_CRITICAL_END,
    SYNC_FREE
};
const char *get_enum_sync_char(sync_t type);

// ============================================================================
/// Enumerates the way to treat memory dependencies.
enum disambiguation_t {
    DISAMBIGUATION_PERFECT,
    DISAMBIGUATION_DISABLE
};
const char *get_enum_disambiguation_char(disambiguation_t type);

// ============================================================================
/// Enumerates the selection policy to pick a sender or next to be treated.
enum selection_t {
    SELECTION_RANDOM,
    SELECTION_ROUND_ROBIN,
    SELECTION_BUFFER_LEVEL
};
const char *get_enum_selection_char(selection_t type);

// ============================================================================
/// Enumerates the routing algorithm
enum routing_algorithm_t {
    ROUTING_ALGORITHM_XY,
    ROUTING_ALGORITHM_ODD_EVEN,
    ROUTING_ALGORITHM_FLOYD_WARSHALL
};
const char *get_enum_routing_algorithm_char(routing_algorithm_t type);

// ============================================================================
/// Enumerates the cache replacement policy
enum replacement_t {
    REPLACEMENT_LRU,                /// way list order: MRU -> LRU
    REPLACEMENT_DEAD_OR_LRU,        /// way list order: MRU -> LRU with DSBP line_usage_predictor
    REPLACEMENT_INVALID_OR_LRU,     /// way list order: MRU -> LRU with priority to Invalid Lines
    REPLACEMENT_RANDOM,             /// way list order: arbitrary
    REPLACEMENT_FIFO,               /// way list order: oldest block -> newest block
    REPLACEMENT_LRF                 /// way list order: Least recently filled
};
const char *get_enum_replacement_char(replacement_t type);

// ============================================================================
/// Directory coherence protocol
enum coherence_protocol_t {
    COHERENCE_PROTOCOL_MOESI
};
const char *get_enum_coherence_protocol_char(coherence_protocol_t type);

// ============================================================================
/// Enumerates the directory inclusiveness protocol
enum inclusiveness_t {
    INCLUSIVENESS_NON_INCLUSIVE,
    INCLUSIVENESS_INCLUSIVE_LLC,
    INCLUSIVENESS_INCLUSIVE_ALL
};
const char *get_enum_inclusiveness_char(inclusiveness_t type);

// ============================================================================
/// Enumerates the prefetcher type
enum prefetch_policy_t {
    PREFETCHER_STRIDE,
    PREFETCHER_STREAM,
    PREFETCHER_DISABLE
};
const char *get_enum_prefetch_policy_char(prefetch_policy_t type);

// ============================================================================
/// Enumerates the prefetcher stride state
enum prefetch_stride_state_t {
    PREFETCHER_STRIDE_STATE_INIT,
    PREFETCHER_STRIDE_STATE_TRANSIENT,
    PREFETCHER_STRIDE_STATE_STEADY,
    PREFETCHER_STRIDE_STATE_NO_PRED
};
const char *get_enum_prefetch_stride_state_char(prefetch_stride_state_t type);

// ============================================================================
/// Enumerates the prefetcher stream state
enum prefetch_stream_state_t {
    PREFETCHER_STREAM_STATE_INVALID,
    PREFETCHER_STREAM_STATE_ALLOCATED,
    PREFETCHER_STREAM_STATE_TRAINING,
    PREFETCHER_STREAM_STATE_MONITOR_AND_REQUEST
};
const char *get_enum_prefetch_stream_state_char(prefetch_stream_state_t type);

// ============================================================================
/// Enumerates the prefetcher full buffer policy
enum full_buffer_t {
    FULL_BUFFER_OVERRIDE,
    FULL_BUFFER_STOP
};
const char *get_enum_full_buffer_char(full_buffer_t type);

// ============================================================================
/// Enumerates the directory line lock status
enum lock_t {
    LOCK_FREE,
    LOCK_READ,
    LOCK_WRITE
};
const char *get_enum_lock_char(lock_t type);

// ============================================================================
/// Enumerates the memory cache address mask
enum cache_mask_t {
    CACHE_MASK_TAG_INDEX_OFFSET,
    CACHE_MASK_TAG_INDEX_BANK_OFFSET,
    CACHE_MASK_TAG_BANK_INDEX_OFFSET
};
const char *get_enum_cache_mask_char(cache_mask_t type);

// ============================================================================
/// Enumerates the main memory address mask
enum memory_controller_mask_t {
    MEMORY_CONTROLLER_MASK_ROW_BANK_COLROW_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_COLROW_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_CTRL_COLROW_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_BANK_COLROW_CHANNEL_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_BANK_COLROW_CTRL_CHANNEL_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_CTRL_BANK_COLROW_COLBYTE,
    MEMORY_CONTROLLER_MASK_ROW_COLROW_BANK_CHANNEL_COLBYTE
};
const char *get_enum_memory_controller_mask_char(memory_controller_mask_t type);

// ============================================================================
/// Enumerates the memory controller commands to the DRAM
enum memory_controller_command_t {
    MEMORY_CONTROLLER_COMMAND_PRECHARGE,
    MEMORY_CONTROLLER_COMMAND_ROW_ACCESS,
    MEMORY_CONTROLLER_COMMAND_COLUMN_READ,
    MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE,
    MEMORY_CONTROLLER_COMMAND_NUMBER
};
const char *get_enum_memory_controller_command_char(memory_controller_command_t type);

// ============================================================================
/// Enumerates the policies to set the priority during the Row Buffer access
enum request_priority_t {
    REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST,
    REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE
};
const char *get_enum_request_priority_char(request_priority_t type);

// ============================================================================
/// Enumerates the policies to give privilege of some operations over others
enum write_priority_t {
    WRITE_PRIORITY_SERVICE_AT_NO_READ,
    WRITE_PRIORITY_DRAIN_WHEN_FULL
};
const char *get_enum_write_priority_char(write_priority_t type);

// ============================================================================
/// Enumerates the line usage predictor type
enum line_usage_predictor_policy_t {
    LINE_USAGE_PREDICTOR_POLICY_DISABLE,
    LINE_USAGE_PREDICTOR_POLICY_DEWP,
    LINE_USAGE_PREDICTOR_POLICY_DEWP_ORACLE,
    LINE_USAGE_PREDICTOR_POLICY_SKEWED
};
const char *get_enum_line_usage_predictor_policy_char(line_usage_predictor_policy_t type);

// ============================================================================
/// Enumerates the valid sub-block type
enum line_prediction_t {
    LINE_PREDICTION_TURNOFF,
    LINE_PREDICTION_NORMAL,
    LINE_PREDICTION_LEARN,
    LINE_PREDICTION_WRONG_FIRST,
    LINE_PREDICTION_WRITEBACK
};
const char *get_enum_line_prediction_t_char(line_prediction_t type);


#endif  // _SINUCA_ENUMERATOR_HPP_
