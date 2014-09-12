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


// ============================================================================
/// Class for Prefetch.
class prefetch_stride_t : public prefetch_t {
    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t stride_table_size;   /// Prefetch Stream Detector Table Size

        uint32_t prefetch_degree;     /// Maximum number of prefetchs ahead
        uint32_t search_distance;     /// Search distance (upwards and downwards)

        uint32_t next_lines_prefetch;

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        stride_table_line_t *stride_table;
        uint64_t last_request_address;      /// Used to check if the next request follows the last address, to perform next line prefetch
        uint64_t last_prefetch_address;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_init_state;
        uint64_t stat_transient_state;
        uint64_t stat_steady_state;
        uint64_t stat_no_pred_state;

        uint64_t stat_allocate_stride_ok;
        uint64_t stat_allocate_stride_fail;

        uint64_t stat_next_line_prefetches;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        prefetch_stride_t();
        ~prefetch_stride_t();

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
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
        // ====================================================================

        void treat_prefetch(memory_package_t *package);

        INSTANTIATE_GET_SET(uint32_t, stride_table_size)
        INSTANTIATE_GET_SET(uint32_t, prefetch_degree)
        INSTANTIATE_GET_SET(uint32_t, search_distance)
        INSTANTIATE_GET_SET(uint32_t, next_lines_prefetch)

        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_init_state)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_transient_state)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_steady_state)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_no_pred_state)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_allocate_stride_ok)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_allocate_stride_fail)

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_next_line_prefetches)
};

