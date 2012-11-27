/// ============================================================================
//
// Copyright (C) 2010, 2011,
// Marco Antonio Zanata Alves
//
// Modified by Francis Birck Moreira
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

/// ============================================================================
/// Class for Prefetch.
class prefetch_stream_t : public prefetch_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        uint32_t stream_table_size;

        uint64_t prefetch_distance;          /// Prefetch Range to Detect Stream
        uint32_t prefetch_degree;            /// Maximum number of prefetchs ahead
        uint32_t search_distance;            /// Search distance (upwards and downwards)

        uint32_t lifetime_cycles;            /// Lifetime before a stream can be replaced, in cycles
        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================

        stream_table_line_t *stream_table;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        prefetch_stream_t();
        ~prefetch_stream_t();

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        // ~ int32_t send_package(memory_package_t *package);
        // ~ bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        // ~ void allocate_token_list();
        // ~ bool check_token_list(memory_package_t *package);
        // ~ uint32_t check_token_space(memory_package_t *package);
        // ~ void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        void treat_prefetch(memory_package_t *package);

        INSTANTIATE_GET_SET(uint32_t, stream_table_size)

        INSTANTIATE_GET_SET(uint32_t, prefetch_distance)
        INSTANTIATE_GET_SET(uint32_t, prefetch_degree)
        INSTANTIATE_GET_SET(uint32_t, search_distance)
        INSTANTIATE_GET_SET(uint32_t, lifetime_cycles)
};

