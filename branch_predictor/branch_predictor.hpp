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
/// Class for Branch Predictor
class branch_predictor_t : public interconnection_interface_t {
    protected:
        /// ====================================================================
        /// Set by sinuca_configurator
        /// ====================================================================
        branch_predictor_policy_t branch_predictor_type;   /// Prefetch policy choosen by the user

        /// ====================================================================
        /// Set by this->allocate()
        /// ====================================================================

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        uint64_t stat_branch_predictor_operation;
        uint64_t stat_branch_predictor_hit;
        uint64_t stat_branch_predictor_miss;

    public:
        /// ====================================================================
        /// Methods
        /// ====================================================================
        branch_predictor_t();
        ~branch_predictor_t();
        inline const char* get_type_component_label() {
            return "BRANCH_PREDICTOR";
        };

        /// ====================================================================
        /// Inheritance from interconnection_interface_t
        /// ====================================================================
        /// Basic Methods
        void allocate();
        void clock(uint32_t sub_cycle);
        int32_t send_package(memory_package_t *package);
        bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        void allocate_token_list();
        bool check_token_list(memory_package_t *package);
        uint32_t check_token_space(memory_package_t *package);
        void remove_token_list(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        /// ====================================================================

        virtual processor_stage_t predict_branch(opcode_package_t actual_opcode, opcode_package_t next_opcode)=0;

        INSTANTIATE_GET_SET(branch_predictor_policy_t, branch_predictor_type)

        /// ====================================================================
        /// Statistics related
        /// ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_operation)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_branch_predictor_miss)
};
