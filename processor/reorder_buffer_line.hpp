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
class reorder_buffer_line_t {
    public:
        uop_package_t uop;                          /// uOP stored
        processor_stage_t stage;                    /// Stage of the uOP
        uint32_t wait_deps_number;                  /// Must wait BEFORE execution
        reorder_buffer_line_t* *deps_ptr_array;     /// Elements to wake-up AFTER execution

        /// ====================================================================
        /// Methods
        /// ====================================================================
        reorder_buffer_line_t() {
            this->uop.package_clean();
            this->stage = PROCESSOR_STAGE_DECODE;
            this->wait_deps_number = 0;
            this->deps_ptr_array = NULL;
        }

        ~reorder_buffer_line_t() {
            if (this->deps_ptr_array) delete [] deps_ptr_array;
            // ~ utils_t::template_delete_array<reorder_buffer_line_t*>(deps_ptr_array);
        }
};
