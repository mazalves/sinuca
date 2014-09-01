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

class stream_table_line_t {
    public:
        uint64_t first_address;             /// First address that generated this stream (line_usage_predictor information)

        uint64_t starting_address;          /// Starting address for this stream
        uint64_t ending_address;            /// Last address for this stream
        uint32_t direction;                 /// Direction of this stream (0 downstream, 1 upstream)

        uint64_t cycle_last_activation;     /// Last time a Memory Request matched into this stream
        uint64_t cycle_last_request;        /// Last prefetch done
        uint64_t last_memory_address;

        prefetch_stream_state_t state;      /// Slot state ( Invalid, Allocated, Training,or  Monitor and Request)

        // ====================================================================
        /// Methods
        // ====================================================================
        stream_table_line_t();
        ~stream_table_line_t();

        void clean();
        std::string content_to_string();

        static std::string print_all(stream_table_line_t *input_array, uint32_t size_array);
};
