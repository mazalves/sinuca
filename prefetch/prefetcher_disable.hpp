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

/// ============================================================================
class prefetch_disable_t : public prefetch_t {
    private:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        prefetch_disable_t();
        ~prefetch_disable_t();

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        // ~ void clock(uint32_t sub_cycle);
        // ~ int32_t send_package(memory_package_t *package);
        // ~ bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
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

        /// REQUEST_BUFFER =====================================================
        int32_t request_buffer_insert();
        memory_package_t* request_buffer_get_older();
        void request_buffer_remove();

        /// TABLE =====================================================
        void stride_table_line_clean(uint32_t stride_buffer_line);
        std::string stride_table_line_to_string(uint32_t stride_buffer_line);
        std::string stride_table_print_all();
        /// ====================================================================

        void treat_prefetch(memory_package_t *package);
};

