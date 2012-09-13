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
class pht_line_t {
    public:
        uint64_t pc;
        uint64_t offset;
        uint64_t last_access;
        bool pointer;
        uint64_t *usage_counter;
        bool *overflow;

        pht_line_t() {
            this->pc = 0;
            this->offset = 0;
            this->last_access = 0;
            this->pointer = 0;
            this->usage_counter = NULL;
            this->overflow = NULL;
        };
        ~pht_line_t() {
            if (this->usage_counter) delete [] usage_counter;
            if (this->overflow) delete [] overflow;
        };
};
