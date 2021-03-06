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

/// Class for Branch Predictor
class branch_predictor_two_level_gag_t : public branch_predictor_t {
    private:
        // ====================================================================
        /// Set by sinuca_configurator
        // ====================================================================
        uint32_t btb_line_number;                   /// Branch Target Buffer Size
        uint32_t btb_associativity;                 /// Branch Target Buffer Associativity
        replacement_t btb_replacement_policy;       /// Branch Target Buffer Replacement Policy

        uint32_t gpht_line_number;                  /// Number of GPHT lines

        hash_function_t gpht_index_hash;            /// Index hash function (SIGNATURE xor PC)

        uint32_t fsm_bits;                          /// Finite State Machine size

        // ====================================================================
        /// Set by this->allocate()
        // ====================================================================
        branch_target_buffer_set_t *btb;    /// Branch Target Buffer
        uint32_t btb_total_sets;            /// Branch Target Buffer Associativity

        uint64_t btb_index_bits_mask;       /// Index mask
        uint64_t btb_index_bits_shift;

        uint64_t btb_tag_bits_mask;         /// Tag mask
        uint64_t btb_tag_bits_shift;

        uint32_t gbhr;                      /// Global Branch History Register

        uint32_t *gpht;                     /// Global Patern History Table
        uint32_t gpht_index_bits;           /// Global Patern History Table - Bits for Index
        uint64_t gpht_index_bits_mask;      /// Global Patern History Table - Index Mask

        uint32_t fsm_max_counter;           /// FSM mask
        uint32_t fsm_taken_threshold;       /// FSM mask to check if taken
        // ====================================================================
        /// Statistics related
        // ====================================================================
        uint64_t stat_btb_accesses;
        uint64_t stat_btb_hit;
        uint64_t stat_btb_miss;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        branch_predictor_two_level_gag_t();
        ~branch_predictor_two_level_gag_t();
        inline const char* get_type_component_label() {
            return "BRANCH_PREDICTOR";
        };

        // ====================================================================
        /// Inheritance from interconnection_interface_t
        // ====================================================================
        /// Basic Methods
        void allocate();
        // ~ void clock(uint32_t sub_cycle);
        // ~ int32_t send_package(memory_package_t *package);
        // ~ bool receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency);
        /// Token Controller Methods
        // ~ bool pop_token_credit(uint32_t src_id, memory_operation_t memory_operation);
        // ~ uint32_t check_token_space(memory_package_t *package);
        /// Debug Methods
        void periodic_check();
        void print_structures();
        void panic();
        /// Statistics Methods
        void reset_statistics();
        void print_statistics();
        void print_configuration();
        // ====================================================================

        uint32_t btb_evict_address(uint64_t opcode_address);
        bool btb_find_update_address(uint64_t opcode_address);

        inline uint64_t btb_get_tag(uint64_t addr) {
            return (addr & this->btb_tag_bits_mask) >> this->btb_tag_bits_shift;
        }

        inline uint64_t btb_get_index(uint64_t addr) {
            return (addr & this->btb_index_bits_mask) >> this->btb_index_bits_shift;
        }

        bool gpht_find_update_prediction(const opcode_package_t& actual_opcode, const opcode_package_t& next_opcode);
        processor_stage_t predict_branch(const opcode_package_t& actual_opcode, const opcode_package_t& next_opcode);


        INSTANTIATE_GET_SET(uint32_t, btb_line_number)
        INSTANTIATE_GET_SET(uint32_t, btb_associativity)
        INSTANTIATE_GET_SET(uint32_t, btb_total_sets)
        INSTANTIATE_GET_SET(replacement_t, btb_replacement_policy)

        INSTANTIATE_GET_SET(uint32_t, gpht_line_number)
        INSTANTIATE_GET_SET(uint32_t, gpht_index_bits)
        INSTANTIATE_GET_SET(uint64_t, gpht_index_bits_mask)
        INSTANTIATE_GET_SET(hash_function_t, gpht_index_hash)

        INSTANTIATE_GET_SET(uint32_t, fsm_bits)
        INSTANTIATE_GET_SET(uint32_t, fsm_max_counter)
        INSTANTIATE_GET_SET(uint32_t, fsm_taken_threshold)

        // ====================================================================
        /// Statistics related
        // ====================================================================
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_accesses)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, stat_btb_miss)
};
