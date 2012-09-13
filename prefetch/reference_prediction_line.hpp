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
class reference_prediction_line_t {
    public:
        uint64_t last_memory_address;           /// Last Memory Request which matched into this stride
        int64_t memory_address_difference;      /// Difference between one access to another
        uint32_t relevance_count;               /// Number of Memory Requests which matched into this stride
        uint32_t prefetch_ahead;                /// Number of prefetchs ahead, already done
        uint64_t cycle_last_activation;         /// Last time a Memory Request matched into this stride
        uint64_t cycle_last_request;            /// Last prefetch done

        /// ====================================================================
        /// Methods
        /// ====================================================================
        reference_prediction_line_t() {
            this->last_memory_address = 0;
            this->memory_address_difference = 0;
            this->relevance_count = 0;
            this->cycle_last_activation = 0;
            this->prefetch_ahead = 0;
            this->cycle_last_request = 0;
        };
        ~reference_prediction_line_t() {
        };
};
