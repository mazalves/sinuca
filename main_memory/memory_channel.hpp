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
class memory_channel_t {
    public:
        memory_package_t **read_buffer;    /// Channel x Bank x Position
        memory_package_t **write_buffer;   /// Channel x Bank x Position

        memory_package_t *row_buffer;      /// Channel x Bank
        uint64_t *cas_ready_cycle;      /// Channel x Bank

        uint32_t *read_buffer_position_used;   /// Channel x Bank
        uint32_t *write_buffer_position_used;  /// Channel x Bank

        bool *drain_write;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        memory_channel_t() {
            this->read_buffer = NULL;
            this->write_buffer = NULL;

            this->row_buffer = NULL;
            this->cas_ready_cycle = NULL;

            this->read_buffer_position_used = NULL;
            this->write_buffer_position_used = NULL;

            this->drain_write = NULL;
        };
        ~memory_channel_t() {
            /// De-Allocate memory to prevent memory leak
            if (this->row_buffer) delete [] row_buffer;
            if (this->read_buffer_position_used) delete [] read_buffer_position_used;
            if (this->write_buffer_position_used) delete [] write_buffer_position_used;
            if (this->drain_write) delete [] drain_write;
            if (this->cas_ready_cycle) delete [] cas_ready_cycle;

            if (read_buffer && read_buffer[0]) {
                delete[] read_buffer[0];
                delete[] read_buffer;
            }

            if (write_buffer && write_buffer[0]) {
                delete[] write_buffer[0];
                delete[] write_buffer;
            }
        };
};
