/// ============================================================================
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
/// ============================================================================
#include "../sinuca.hpp"

#ifdef PREFETCHER_DEBUG
    #define PREFETCHER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define PREFETCHER_DEBUG_PRINTF(...)
#endif


/// ============================================================================
prefetch_disable_t::prefetch_disable_t() {
    this->set_prefetcher_type(PREFETCHER_DISABLE);
};

/// ============================================================================
prefetch_disable_t::~prefetch_disable_t() {
    /// De-Allocate memory to prevent memory leak
};

/// ============================================================================
void prefetch_disable_t::allocate() {
    prefetch_t::allocate();
};


/// ============================================================================
void prefetch_disable_t::treat_prefetch(memory_package_t *package) {
    (void)package;
};


/// ============================================================================
void prefetch_disable_t::print_structures() {
    prefetch_t::print_structures();

};

/// ============================================================================
void prefetch_disable_t::panic() {
    prefetch_t::panic();

    this->print_structures();
};

/// ============================================================================
void prefetch_disable_t::periodic_check(){
    prefetch_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void prefetch_disable_t::reset_statistics() {
    prefetch_t::reset_statistics();

};

/// ============================================================================
void prefetch_disable_t::print_statistics() {
    prefetch_t::print_statistics();

};

/// ============================================================================
void prefetch_disable_t::print_configuration() {
    prefetch_t::print_configuration();

};
