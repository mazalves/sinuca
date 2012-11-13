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
class cache_set_t {
    public:
        cache_line_t *ways;

        /// ====================================================================
        /// Methods
        /// ====================================================================
        cache_set_t() {
            this->ways = NULL;
        };
        ~cache_set_t() {
            /// De-Allocate memory to prevent memory leak
            if (this->ways) delete [] ways;
            // ~ utils_t::template_delete_array<cache_line_t>(ways);
        };
};
