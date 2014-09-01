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


class prefetch_t : public interconnection_interface_t {
    protected:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        prefetch_policy_t prefetcher_type;  /// Prefetch policy choosen by the user
        full_buffer_t full_buffer_type;
        uint32_t request_buffer_size;


        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        memory_package_t *request_buffer;  /// Prefetch transactions waiting for room on the MSHR (High Level Input)
        uint32_t request_buffer_position_start;
        uint32_t request_buffer_position_end;
        uint32_t request_buffer_position_used;

        uint64_t offset_bits_mask;
        uint64_t not_offset_bits_mask;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_created_prefetches;
        uint64_t stat_dropped_prefetches;
        uint64_t stat_full_buffer;

        uint64_t stat_upstride_prefetches;
        uint64_t stat_downstride_prefetches;

        uint64_t stat_request_matches;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        prefetch_t();
        ~prefetch_t();
        inline const char* get_type_component_label() {
            return "PREFETCH";
        };

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        bool check_token_list(memory_package_t *package);
        void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        // ====================================================================

        /// REQUEST_BUFFER =====================================================
        int32_t request_buffer_insert();
        memory_package_t* request_buffer_get_older();
        void request_buffer_remove();

        virtual void treat_prefetch(memory_package_t *package)=0;

        inline bool cmp_index_tag(uint64_t memory_addressA, uint64_t memory_addressB) {
            return (memory_addressA & this->not_offset_bits_mask) == (memory_addressB & this->not_offset_bits_mask);
        }

        INSTANTIATE_GET_SET(prefetch_policy_t, prefetcher_type)
        INSTANTIATE_GET_SET(full_buffer_t, full_buffer_type)

        INSTANTIATE_GET_SET(uint64_t, offset_bits_mask)
        INSTANTIATE_GET_SET(uint64_t, not_offset_bits_mask)

        INSTANTIATE_GET_SET(memory_package_t*, request_buffer)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_size)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_start)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_end)
        INSTANTIATE_GET_SET(uint32_t, request_buffer_position_used)

        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_created_prefetches);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_dropped_prefetches);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_full_buffer);

        INSTANTIATE_GET_SET_ADD(uint64_t, stat_upstride_prefetches);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_downstride_prefetches);
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_request_matches);
};
