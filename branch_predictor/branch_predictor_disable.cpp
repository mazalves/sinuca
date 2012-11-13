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
#include "../sinuca.hpp"

#ifdef BRANCH_PREDICTOR_DEBUG
    #define BRANCH_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define BRANCH_PREDICTOR_DEBUG_PRINTF(...)
#endif

/// ============================================================================
branch_predictor_disable_t::branch_predictor_disable_t() {
    this->branch_predictor_type = BRANCH_PREDICTOR_DISABLE;
};

/// ============================================================================
branch_predictor_disable_t::~branch_predictor_disable_t() {
    /// De-Allocate memory to prevent memory leak
};

/// ============================================================================
void branch_predictor_disable_t::allocate() {
    branch_predictor_t::allocate();
};

/// ============================================================================
/// Always consider as it is NOT_TAKEN
processor_stage_t branch_predictor_disable_t::predict_branch(opcode_package_t actual_opcode, opcode_package_t next_opcode) {
    processor_stage_t solve_stage = PROCESSOR_STAGE_FETCH;

    if (actual_opcode.opcode_operation == INSTRUCTION_OPERATION_BRANCH) {
        uint64_t next_sequential_address = actual_opcode.opcode_address + actual_opcode.opcode_size;
        bool is_taken = (next_sequential_address != next_opcode.opcode_address);

        /// If NOT TAKEN, it will never generate extra latency
        if (is_taken){
            solve_stage = PROCESSOR_STAGE_EXECUTION;
        }
        else {
            solve_stage = PROCESSOR_STAGE_FETCH;
        }

        /// Increment the statistics
        this->add_stat_branch_predictor_operation();
        if (solve_stage == PROCESSOR_STAGE_FETCH) {
            this->add_stat_branch_predictor_hit();
        }
        else {
            this->add_stat_branch_predictor_miss();
        }
    }

    return solve_stage;
};

/// ============================================================================
void branch_predictor_disable_t::print_structures() {
    branch_predictor_t::print_structures();
};

/// ============================================================================
void branch_predictor_disable_t::panic() {
    branch_predictor_t::panic();
    this->print_structures();
};

/// ============================================================================
void branch_predictor_disable_t::periodic_check(){
    branch_predictor_t::periodic_check();
    #ifdef BRANCH_PREDICTOR_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void branch_predictor_disable_t::reset_statistics() {
    branch_predictor_t::reset_statistics();
};

/// ============================================================================
void branch_predictor_disable_t::print_statistics() {
    branch_predictor_t::print_statistics();
};

/// ============================================================================
/// ============================================================================
void branch_predictor_disable_t::print_configuration() {
    branch_predictor_t::print_configuration();
};
