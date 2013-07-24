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
class cache_line_t {
    public:
        uint64_t tag;
        protocol_status_t status;
        uint64_t last_access;
        uint64_t usage_counter;
        bool dirty;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_line_t() {
            this->tag = 0;
            this->status = PROTOCOL_STATUS_I;
            this->last_access = 0;
            this->usage_counter = 0;
            this->dirty = false;
        };
        ~cache_line_t() {
        };
};
