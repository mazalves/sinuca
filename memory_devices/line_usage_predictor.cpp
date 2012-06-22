//==============================================================================
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
//==============================================================================
#include "../sinuca.hpp"

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

//==============================================================================
line_usage_predictor_t::line_usage_predictor_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DISABLE;

    /// ========================================================================
    /// DSBP
    /// ========================================================================
    this->DSBP_sets = NULL;
    this->DSBP_line_number = 0;
    this->DSBP_associativity = 0;
    this->DSBP_total_sets = 0;

    this->DSBP_sub_block_size = 0;
    this->DSBP_sub_block_total = 0;

    this->DSBP_usage_counter_bits = 0;
    this->DSBP_usage_counter_max = 0;

    /// PHT
    this->DSBP_PHT_sets = NULL;
    this->DSBP_PHT_line_number = 0;
    this->DSBP_PHT_associativity = 0;
    this->DSBP_PHT_total_sets = 0;
};

//==============================================================================
line_usage_predictor_t::~line_usage_predictor_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<DSBP_metadata_sets_t>(DSBP_sets);
    utils_t::template_delete_array<DSBP_PHT_sets_t>(DSBP_PHT_sets);
};

//==============================================================================
void line_usage_predictor_t::allocate() {
    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP:

            ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size() / this->get_DSBP_sub_block_size()), "Wrong line_size(%u) or sub_block_size(%u).\n", this->get_DSBP_line_number(), this->get_DSBP_associativity());
            this->set_DSBP_sub_block_total(sinuca_engine.get_global_line_size() / this->get_DSBP_sub_block_size());

            ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_DSBP_line_number() / this->get_DSBP_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_DSBP_line_number(), this->get_DSBP_associativity());
            this->set_DSBP_total_sets(this->get_DSBP_line_number() / this->get_DSBP_associativity());

            this->DSBP_usage_counter_max = pow(2, this->DSBP_usage_counter_bits) - 1;

            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s DSBP %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_DSBP_line_number(), this->get_DSBP_associativity(), this->get_DSBP_total_sets(), this->get_DSBP_sub_block_total());
            this->DSBP_sets = utils_t::template_allocate_array<DSBP_metadata_sets_t>(this->get_DSBP_total_sets());
            for (uint32_t i = 0; i < this->get_DSBP_total_sets(); i++) {
                this->DSBP_sets[i].ways = utils_t::template_allocate_array<DSBP_metadata_line_t>(this->get_DSBP_associativity());

                for (uint32_t j = 0; j < this->get_DSBP_associativity(); j++) {
                    this->DSBP_sets[i].ways[j].valid_sub_blocks = utils_t::template_allocate_initialize_array<line_sub_block_t>(sinuca_engine.get_global_line_size(), LINE_SUB_BLOCK_DISABLE);
                    this->DSBP_sets[i].ways[j].real_usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
                    this->DSBP_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
                    this->DSBP_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
                    this->DSBP_sets[i].ways[j].is_dead = true;
                }
            }

            ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_DSBP_PHT_line_number() / this->get_DSBP_PHT_associativity()), "Wrong line number(%u) or associativity(%u).\n", this->get_DSBP_PHT_line_number(), this->get_DSBP_PHT_associativity());
            this->set_DSBP_PHT_total_sets(this->get_DSBP_PHT_line_number() / this->get_DSBP_PHT_associativity());
            this->DSBP_PHT_sets = utils_t::template_allocate_array<DSBP_PHT_sets_t>(this->get_DSBP_PHT_total_sets());
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s DSBP_PHT %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_DSBP_PHT_line_number(), this->get_DSBP_PHT_associativity(), this->get_DSBP_PHT_total_sets(), this->get_DSBP_sub_block_total());

            /// INDEX MASK
            this->DSBP_PHT_index_bits_mask = 0;
            for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_DSBP_PHT_total_sets()); i++) {
                this->DSBP_PHT_index_bits_mask |= 1 << (i);
            }

            for (uint32_t i = 0; i < this->get_DSBP_PHT_total_sets(); i++) {
                this->DSBP_PHT_sets[i].ways = utils_t::template_allocate_array<DSBP_PHT_line_t>(this->get_DSBP_PHT_associativity());

                for (uint32_t j = 0; j < this->get_DSBP_PHT_associativity(); j++) {
                    this->DSBP_PHT_sets[i].ways[j].pc = 0;
                    this->DSBP_PHT_sets[i].ways[j].offset = 0;
                    this->DSBP_PHT_sets[i].ways[j].pointer = false;

                    this->DSBP_PHT_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
                    this->DSBP_PHT_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
                }
            }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
            // Cache Metadata
            ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size() / this->get_DSBP_sub_block_size()), "Wrong line_size(%u) or sub_block_size(%u).\n", this->get_DSBP_line_number(), this->get_DSBP_associativity());
            this->set_DSBP_sub_block_total(sinuca_engine.get_global_line_size() / this->get_DSBP_sub_block_size());

            ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_DSBP_line_number() / this->get_DSBP_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_DSBP_line_number(), this->get_DSBP_associativity());
            this->set_DSBP_total_sets(this->get_DSBP_line_number() / this->get_DSBP_associativity());

            this->DSBP_usage_counter_max = pow(2, this->DSBP_usage_counter_bits);

            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s DSBP %d(lines) / %d(assoc) = %d (sets) (%d (sub-blocks))\n", this->get_label(), this->get_DSBP_line_number(), this->get_DSBP_associativity(), this->get_DSBP_total_sets(), this->get_DSBP_sub_block_total());
            this->DSBP_sets = utils_t::template_allocate_array<DSBP_metadata_sets_t>(this->get_DSBP_total_sets());
            for (uint32_t i = 0; i < this->get_DSBP_total_sets(); i++) {
                this->DSBP_sets[i].ways = utils_t::template_allocate_array<DSBP_metadata_line_t>(this->get_DSBP_associativity());

                for (uint32_t j = 0; j < this->get_DSBP_associativity(); j++) {
                    this->DSBP_sets[i].ways[j].valid_sub_blocks = utils_t::template_allocate_initialize_array<line_sub_block_t>(sinuca_engine.get_global_line_size(), LINE_SUB_BLOCK_DISABLE);
                    this->DSBP_sets[i].ways[j].real_usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
                    this->DSBP_sets[i].ways[j].usage_counter = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size(), 0);
                    this->DSBP_sets[i].ways[j].overflow = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_global_line_size(), false);
                    this->DSBP_sets[i].ways[j].is_dead = true;
                }
            }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }

    /// ================================================================
    /// Statistics
    /// ================================================================
    this->stat_accessed_sub_block = utils_t::template_allocate_initialize_array<uint64_t>(sinuca_engine.get_global_line_size() + 1, 0);

};

//==============================================================================
void line_usage_predictor_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");

};


//==============================================================================
bool line_usage_predictor_t::receive_package(memory_package_t *package, uint32_t input_port) {
    ERROR_PRINTF("Received package %s into the input_port %u.\n", package->memory_to_string().c_str(), input_port);
    return FAIL;
};

//==============================================================================
void line_usage_predictor_t::print_structures() {
};

// =============================================================================
void line_usage_predictor_t::panic() {
    this->print_structures();
};

//==============================================================================
void line_usage_predictor_t::periodic_check(){
    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

// =============================================================================
// STATISTICS
// =============================================================================
void line_usage_predictor_t::reset_statistics() {
    this->stat_DSBP_line_sub_block_disable_always = 0;
    this->stat_DSBP_line_sub_block_disable_turnoff = 0;
    this->stat_DSBP_line_sub_block_normal_correct = 0;
    this->stat_DSBP_line_sub_block_normal_over = 0;
    this->stat_DSBP_line_sub_block_learn = 0;
    this->stat_DSBP_line_sub_block_wrong_first = 0;

    this->stat_DSBP_PHT_hit = 0;
    this->stat_DSBP_PHT_miss = 0;

    this->stat_sub_block_touch_0 = 0;
    this->stat_sub_block_touch_1 = 0;
    this->stat_sub_block_touch_2_3 = 0;
    this->stat_sub_block_touch_4_7 = 0;
    this->stat_sub_block_touch_8_15 = 0;
    this->stat_sub_block_touch_16_127 = 0;
    this->stat_sub_block_touch_128_bigger = 0;

    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        stat_accessed_sub_block[i] = 0;
    }
};

// =============================================================================
void line_usage_predictor_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_disable_always", stat_DSBP_line_sub_block_disable_always);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_disable_turnoff", stat_DSBP_line_sub_block_disable_turnoff);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_normal_correct", stat_DSBP_line_sub_block_normal_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_normal_over", stat_DSBP_line_sub_block_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_learn", stat_DSBP_line_sub_block_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_line_sub_block_wrong_first", stat_DSBP_line_sub_block_wrong_first);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_PHT_hit", stat_DSBP_PHT_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_DSBP_PHT_miss", stat_DSBP_PHT_miss);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_0", stat_sub_block_touch_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_1", stat_sub_block_touch_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_2_3", stat_sub_block_touch_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_4_7", stat_sub_block_touch_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_8_15", stat_sub_block_touch_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_16_127", stat_sub_block_touch_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_touch_128_bigger", stat_sub_block_touch_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size() + 1; i++) {
        char name[50];
        sprintf(name, "stat_accessed_sub_block_%u", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, stat_accessed_sub_block[i]);
    }
};

// =============================================================================
void line_usage_predictor_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_usage_predictor_type", get_enum_line_usage_predictor_policy_char(line_usage_predictor_type));

    /// ====================================================================
    /// DSBP
    /// ====================================================================
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_usage_counter_bits", DSBP_usage_counter_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_usage_counter_max", DSBP_usage_counter_max);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_sub_block_size", DSBP_sub_block_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_sub_block_total", DSBP_sub_block_total);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_line_number", DSBP_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_associativity", DSBP_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_total_sets", DSBP_total_sets);


    /// PHT
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_PHT_line_number", DSBP_PHT_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_PHT_associativity", DSBP_PHT_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_PHT_total_sets", DSBP_PHT_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "DSBP_PHT_replacement_policy", get_enum_replacement_char(DSBP_PHT_replacement_policy));
};

// =============================================================================
// METHODS
// =============================================================================
// Input:   base_address, size
// Output:  start_sub_block, end_Sub_block
void line_usage_predictor_t::get_start_end_sub_blocks(uint64_t base_address, uint32_t size, uint32_t& sub_block_ini, uint32_t& sub_block_end) {
    uint64_t address_offset = base_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t address_size = address_offset + size;
    if (address_size >= sinuca_engine.get_global_line_size()){
        address_size = sinuca_engine.get_global_line_size();
    }

    sub_block_ini = uint32_t(address_offset / this->DSBP_sub_block_size) * this->DSBP_sub_block_size;
    sub_block_end = uint32_t( (address_size / this->DSBP_sub_block_size) +
                                        uint32_t((address_size % this->DSBP_sub_block_size) != 0) ) * this->DSBP_sub_block_size;
}

// =============================================================================
void line_usage_predictor_t::fill_package_sub_blocks(memory_package_t *package) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->memory_to_string().c_str())

    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP:
        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
        {
            /// Compute the START and END sub_blocks
            uint32_t sub_block_ini, sub_block_end;
            this->get_start_end_sub_blocks(package->memory_address, package->memory_size, sub_block_ini, sub_block_end);

            /// Generates the Request Vector
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                package->sub_blocks[i] = ( i >= sub_block_ini && i < sub_block_end );
            }


            #ifdef LINE_USAGE_PREDICTOR_DEBUG
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t package->sub_blocks[")
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (i % this->DSBP_sub_block_size == 0) {
                        SINUCA_PRINTF("|");
                    }
                    SINUCA_PRINTF("%u ", package->sub_blocks[i])
                }
                SINUCA_PRINTF("]\n")
            #endif
        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }
};

// =============================================================================
bool line_usage_predictor_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_sub_block_is_hit() package:%s\n", package->memory_to_string().c_str())

    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP: {

            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                if (package->sub_blocks[i] == true &&
                this->DSBP_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
                    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
                    return false;
                }
            }

            #ifdef LINE_USAGE_PREDICTOR_DEBUG
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t valid_sub_blocks[")
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                    SINUCA_PRINTF("%u ", this->DSBP_sets[index].ways[way].valid_sub_blocks[i])
                }
                SINUCA_PRINTF("]\n")
            #endif

            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
            return true;
        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
    return true;
};

// =============================================================================
// Mechanism Operations
// =============================================================================
void line_usage_predictor_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->memory_to_string().c_str())

    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP: {
            ERROR_ASSERT_PRINTF(index < this->DSBP_total_sets, "Wrong index %d > total_sets %d", index, this->DSBP_total_sets);
            ERROR_ASSERT_PRINTF(way < this->DSBP_associativity, "Wrong way %d > associativity %d", way, this->DSBP_associativity);

            #ifdef LINE_USAGE_PREDICTOR_DEBUG
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t package->sub_blocks[")
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                    SINUCA_PRINTF("%u ", package->sub_blocks[i])
                }
                SINUCA_PRINTF("]\n")
            #endif

            #ifdef LINE_USAGE_PREDICTOR_DEBUG
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t real_usage_counter[")
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                    SINUCA_PRINTF("%"PRIu64" ", this->DSBP_sets[index].ways[way].real_usage_counter[i])
                }
                SINUCA_PRINTF("]\n")
            #endif

            // Update the METADATA real_usage_counter
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                // Package Requested
                if (package->sub_blocks[i] == true) {
                    this->DSBP_sets[index].ways[way].real_usage_counter[i]++;
                    ERROR_ASSERT_PRINTF(this->DSBP_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");
                }
            }

            /// ================================================================
            /// METADATA Learn Mode
            /// ================================================================
            if (this->DSBP_sets[index].ways[way].learn_mode == true) {
                // Has PHT pointer
                if (this->DSBP_sets[index].ways[way].PHT_pointer != NULL) {

                    // Update the PHT
                    this->DSBP_sets[index].ways[way].PHT_pointer->last_access = sinuca_engine.get_global_cycle();
                    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                        // Package Requested
                        if (package->sub_blocks[i] == true) {
                            // Check the PHT Overflow
                            if (this->DSBP_sets[index].ways[way].PHT_pointer->usage_counter[i] >= this->DSBP_usage_counter_max) {
                                this->DSBP_sets[index].ways[way].PHT_pointer->overflow[i] = true;
                            }
                            else {
                                this->DSBP_sets[index].ways[way].PHT_pointer->usage_counter[i]++;
                            }
                        }
                    }
                }
            }

            /// ================================================================
            /// METADATA Not Learn Mode
            /// ================================================================
            else {
                bool line_is_dead = true;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    // METADATA Not Overflow + METADATA Used Predicted Number of Times
                    if (this->DSBP_sets[index].ways[way].overflow[i] == false &&
                    this->DSBP_sets[index].ways[way].real_usage_counter[i] == this->DSBP_sets[index].ways[way].usage_counter[i]) {
                        // METADATA Turn off the sub_blocks
                        this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
                    }
                    // Check if the line is dead (ALL off)
                    else if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE){
                        line_is_dead = false;
                    }
                }
                this->DSBP_sets[index].ways[way].is_dead = line_is_dead;
            }

        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
            // Update the METADATA real_usage_counter
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                this->DSBP_sets[index].ways[way].real_usage_counter[i] += package->sub_blocks[i];
            }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }
};

// =============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->memory_to_string().c_str())
    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP: {
            ERROR_ASSERT_PRINTF(index < this->DSBP_total_sets, "Wrong index %d > total_sets %d", index, this->DSBP_total_sets);
            ERROR_ASSERT_PRINTF(way < this->DSBP_associativity, "Wrong way %d > associativity %d", way, this->DSBP_associativity);

            DSBP_PHT_line_t *PHT_line = this->DSBP_PHT_find_line(package->opcode_address, package->memory_address);
            ///=================================================================
            /// PHT HIT
            ///=================================================================
            if (PHT_line != NULL) {

                #ifdef LINE_USAGE_PREDICTOR_DEBUG
                    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t PHT - usage_counter[")
                    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                        if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                        SINUCA_PRINTF("%"PRIu64" ", PHT_line->usage_counter[i])
                    }
                    SINUCA_PRINTF("]\n")
                #endif

                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t PHT HIT\n")
                this->add_stat_DSBP_PHT_hit();

                // Update the PHT entry
                PHT_line->last_access = sinuca_engine.get_global_cycle();

                // Disable Learn_mode
                this->DSBP_sets[index].ways[way].learn_mode = false;
                // If no PHT_pointer
                if (PHT_line->pointer == false) {
                    // Create a PHT pointer
                    PHT_line->pointer = true;
                    this->DSBP_sets[index].ways[way].PHT_pointer = PHT_line;
                }
                else {
                    this->DSBP_sets[index].ways[way].PHT_pointer = NULL;
                }

                // Clean the metadata entry
                this->DSBP_sets[index].ways[way].learn_mode = true;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    // Copy the PHT prediction
                    this->DSBP_sets[index].ways[way].usage_counter[i] = PHT_line->usage_counter[i];
                    this->DSBP_sets[index].ways[way].overflow[i] = PHT_line->overflow[i];

                    this->DSBP_sets[index].ways[way].real_usage_counter[i] = 0;
                    this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_DISABLE;
                    if (package->sub_blocks[i] == true || PHT_line->usage_counter[i] > 0)
                        this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
                }

                // Add the last access information
                this->line_hit(package, index, way);

                // Modify the package->sub_blocks (next level request)
                package->memory_size = 0;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
                        package->sub_blocks[i] = true;
                        package->memory_size++;
                    }
                    else {
                        package->sub_blocks[i] = false;
                    }

                }
            }

            ///=================================================================
            /// PHT MISS
            ///=================================================================
            else {
                LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t PHT MISS\n")
                this->add_stat_DSBP_PHT_miss();
                // New PHT entry
                PHT_line = DSBP_PHT_evict_address(package->opcode_address, package->memory_address);
                // Clean the PHT entry
                PHT_line->last_access = sinuca_engine.get_global_cycle();
                PHT_line->pc = package->opcode_address;
                PHT_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
                PHT_line->last_access = sinuca_engine.get_global_cycle();
                PHT_line->pointer = true;
                this->DSBP_sets[index].ways[way].PHT_pointer = PHT_line;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    PHT_line->usage_counter[i] = 0;
                    PHT_line->overflow[i] = false;
                }

                // Clean the metadata entry
                this->DSBP_sets[index].ways[way].learn_mode = true;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    this->DSBP_sets[index].ways[way].real_usage_counter[i] = 0;
                    this->DSBP_sets[index].ways[way].usage_counter[i] = 0;
                    this->DSBP_sets[index].ways[way].overflow[i] = false;
                    this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_LEARN;
                }

                // Add the last access information
                this->line_hit(package, index, way);

                // Modify the package->sub_blocks (next level request)
                package->memory_size = 0;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    package->sub_blocks[i] = true;
                    package->memory_size++;
                }
            }

        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
            package->memory_size = sinuca_engine.get_global_line_size();

            // Clean the metadata entry
            this->DSBP_sets[index].ways[way].learn_mode = true;
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                this->DSBP_sets[index].ways[way].real_usage_counter[i] = 0;
                this->DSBP_sets[index].ways[way].usage_counter[i] = 0;
                this->DSBP_sets[index].ways[way].overflow[i] = true;
                this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_NORMAL;
            }

            // Add the last access information
            this->line_hit(package, index, way);
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
            package->memory_size = sinuca_engine.get_global_line_size();
        break;
    }
};

// =============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->memory_to_string().c_str())
    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP: {
            ERROR_ASSERT_PRINTF(index < this->DSBP_total_sets, "Wrong index %d > total_sets %d", index, this->DSBP_total_sets);
            ERROR_ASSERT_PRINTF(way < this->DSBP_associativity, "Wrong way %d > associativity %d", way, this->DSBP_associativity);

            DSBP_PHT_line_t *PHT_line = this->DSBP_sets[index].ways[way].PHT_pointer;
            ///=================================================================
            /// PHT HIT
            ///=================================================================
            if (PHT_line != NULL && PHT_line->pointer == true) {
                // Enable Learn_mode
                this->DSBP_sets[index].ways[way].learn_mode = true;

                // Update the PHT entry
                PHT_line->last_access = sinuca_engine.get_global_cycle();
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->DSBP_sets[index].ways[way].real_usage_counter[i] <= this->DSBP_usage_counter_max) {
                        PHT_line->usage_counter[i] = this->DSBP_sets[index].ways[way].real_usage_counter[i];
                        PHT_line->overflow[i] = false;
                    }
                    else {
                        PHT_line->usage_counter[i] = this->DSBP_usage_counter_max;
                        PHT_line->overflow[i] = true;
                    }

                    // Enable all sub_blocks
                    this->DSBP_sets[index].ways[way].usage_counter[i] = 0;
                    this->DSBP_sets[index].ways[way].overflow[i] = true;
                    // Enable Valid Sub_blocks
                    if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
                        this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_WRONG_FIRST;
                    }
                }

                // Add the last access information
                this->line_hit(package, index, way);

                // Enable all sub_blocks missing
                package->memory_size = 0;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                        package->sub_blocks[i] = true;
                        package->memory_size++;
                    }
                    else {
                        package->sub_blocks[i] = false;
                    }
                }
            }

            ///=================================================================
            /// PHT MISS
            ///=================================================================
            else {
                // Enable Learn_mode
                this->DSBP_sets[index].ways[way].learn_mode = true;

                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    this->DSBP_sets[index].ways[way].usage_counter[i] = 0;
                    this->DSBP_sets[index].ways[way].overflow[i] = true;
                    // Enable Valid Sub_blocks
                    if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_DISABLE) {
                        this->DSBP_sets[index].ways[way].valid_sub_blocks[i] = LINE_SUB_BLOCK_WRONG_FIRST;
                    }
                }

                // Add the last access information
                this->line_hit(package, index, way);

                // Enable all sub_blocks missing
                package->memory_size = 0;
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] == LINE_SUB_BLOCK_WRONG_FIRST) {
                        package->sub_blocks[i] = true;
                        package->memory_size++;
                    }
                    else {
                        package->sub_blocks[i] = false;
                    }
                }
            }
        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }
};


// =============================================================================
void line_usage_predictor_t::line_copy_back(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->memory_to_string().c_str())
    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP: {
            ERROR_ASSERT_PRINTF(index < this->DSBP_total_sets, "Wrong index %d > total_sets %d", index, this->DSBP_total_sets);
            ERROR_ASSERT_PRINTF(way < this->DSBP_associativity, "Wrong way %d > associativity %d", way, this->DSBP_associativity);

            // Modify the package->sub_blocks (valid_sub_blocks)
            package->memory_size = 0;
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                if (this->DSBP_sets[index].ways[way].valid_sub_blocks[i] != LINE_SUB_BLOCK_DISABLE) {
                    package->sub_blocks[i] = true;
                    package->memory_size++;
                }
                else {
                    package->sub_blocks[i] = false;
                }
            }
        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
            package->memory_size = sinuca_engine.get_global_line_size();
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
            package->memory_size = sinuca_engine.get_global_line_size();
        break;
    }
};

// =============================================================================
void line_usage_predictor_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    switch (this->get_line_usage_predictor_type()) {
        case LINE_USAGE_PREDICTOR_POLICY_DSBP:
        case LINE_USAGE_PREDICTOR_POLICY_DSBP_DISABLE:
        {
            ERROR_ASSERT_PRINTF(index < this->DSBP_total_sets, "Wrong index %d > total_sets %d", index, this->DSBP_total_sets);
            ERROR_ASSERT_PRINTF(way < this->DSBP_associativity, "Wrong way %d > associativity %d", way, this->DSBP_associativity);

            DSBP_PHT_line_t *PHT_line = this->DSBP_sets[index].ways[way].PHT_pointer;
            // PHT HIT
            if (PHT_line != NULL) {

                #ifdef LINE_USAGE_PREDICTOR_DEBUG
                    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Update PHT - usage_counter[")
                    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                        if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                        SINUCA_PRINTF("%"PRIu64" ", PHT_line->usage_counter[i])
                    }
                    SINUCA_PRINTF("]\n")
                #endif


                // Update the PHT entry
                PHT_line->last_access = sinuca_engine.get_global_cycle();
                for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                    if (this->DSBP_sets[index].ways[way].real_usage_counter[i] <= this->DSBP_usage_counter_max) {
                        PHT_line->usage_counter[i] = this->DSBP_sets[index].ways[way].real_usage_counter[i];
                        PHT_line->overflow[i] = false;
                    }
                    else {
                        PHT_line->usage_counter[i] = this->DSBP_usage_counter_max;
                        PHT_line->overflow[i] = true;
                    }
                }
                PHT_line->pointer = false;
                this->DSBP_sets[index].ways[way].PHT_pointer = NULL;

                #ifdef LINE_USAGE_PREDICTOR_DEBUG
                    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Update PHT - usage_counter[")
                    for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                        if (i % this->DSBP_sub_block_size == 0) { SINUCA_PRINTF("|"); }
                        SINUCA_PRINTF("%"PRIu64" ", PHT_line->usage_counter[i])
                    }
                    SINUCA_PRINTF("]\n")
                #endif

            }


            // Statistics
            uint32_t sub_blocks_touched = 0;
            for (uint32_t i = 0; i < sinuca_engine.get_global_line_size(); i++) {
                // Prediction Accuracy
                switch (this->DSBP_sets[index].ways[way].valid_sub_blocks[i]) {
                    case LINE_SUB_BLOCK_DISABLE:
                        if (this->DSBP_sets[index].ways[way].usage_counter[i] == 0) {
                            this->stat_DSBP_line_sub_block_disable_always++;
                        }
                        else {
                            this->stat_DSBP_line_sub_block_disable_turnoff++;
                        }
                    break;

                    case LINE_SUB_BLOCK_NORMAL:
                        if (this->DSBP_sets[index].ways[way].overflow[i] == 1) {
                            this->stat_DSBP_line_sub_block_normal_correct++;
                        }
                        else {
                            this->stat_DSBP_line_sub_block_normal_over++;
                        }
                    break;

                    case LINE_SUB_BLOCK_LEARN:
                        this->stat_DSBP_line_sub_block_learn++;
                    break;

                    case LINE_SUB_BLOCK_WRONG_FIRST:
                        this->stat_DSBP_line_sub_block_wrong_first++;
                    break;
                }

                // Touches before eviction
                if (this->DSBP_sets[index].ways[way].real_usage_counter[i] == 0) {
                    this->add_stat_sub_block_touch_0();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] == 1) {
                    this->add_stat_sub_block_touch_1();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] >= 2 && this->DSBP_sets[index].ways[way].real_usage_counter[i] <= 3) {
                    this->add_stat_sub_block_touch_2_3();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] >= 4 && this->DSBP_sets[index].ways[way].real_usage_counter[i] <= 7) {
                    this->add_stat_sub_block_touch_4_7();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] >= 8 && this->DSBP_sets[index].ways[way].real_usage_counter[i] <= 15) {
                    this->add_stat_sub_block_touch_8_15();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] >= 16 && this->DSBP_sets[index].ways[way].real_usage_counter[i] <= 127) {
                    this->add_stat_sub_block_touch_16_127();
                }
                else if (this->DSBP_sets[index].ways[way].real_usage_counter[i] >=128) {
                    this->add_stat_sub_block_touch_128_bigger();
                }

                // Sub-Blocks accessed before eviction
                if (this->DSBP_sets[index].ways[way].real_usage_counter[i] != 0) {
                    sub_blocks_touched++;
                }
            }
            this->stat_accessed_sub_block[sub_blocks_touched]++;

            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Stats %u %"PRIu64"\n", sub_blocks_touched, this->stat_accessed_sub_block[sub_blocks_touched] )
        }
        break;

        case LINE_USAGE_PREDICTOR_POLICY_SPP:
            ERROR_PRINTF("Invalid line usage predictor strategy %s.\n", get_enum_line_usage_predictor_policy_char(this->get_line_usage_predictor_type()));
        break;

        case LINE_USAGE_PREDICTOR_POLICY_DISABLE:
        break;
    }
};

// =============================================================================
// DSBP - PHT
// =============================================================================
DSBP_PHT_line_t* line_usage_predictor_t::DSBP_PHT_find_line(uint64_t pc, uint64_t memory_address) {
    uint32_t PHT_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t PHT_index = pc & this->DSBP_PHT_index_bits_mask;

    ERROR_ASSERT_PRINTF(PHT_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", PHT_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(PHT_index < this->DSBP_PHT_total_sets, "Wrong index %d > total_sets %d", PHT_index, this->DSBP_PHT_total_sets);

    for (uint32_t PHT_way = 0; PHT_way < this->get_DSBP_PHT_associativity(); PHT_way++) {
        if (this->DSBP_PHT_sets[PHT_index].ways[PHT_way].pc == pc && this->DSBP_PHT_sets[PHT_index].ways[PHT_way].offset == PHT_offset) {
            return &this->DSBP_PHT_sets[PHT_index].ways[PHT_way];
        }
    }
    return NULL;
}

// =============================================================================
DSBP_PHT_line_t* line_usage_predictor_t::DSBP_PHT_evict_address(uint64_t pc, uint64_t memory_address) {
    uint32_t PHT_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t PHT_index = pc & this->DSBP_PHT_index_bits_mask;

    ERROR_ASSERT_PRINTF(PHT_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", PHT_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(PHT_index < this->DSBP_PHT_total_sets, "Wrong index %d > total_sets %d", PHT_index, this->DSBP_PHT_total_sets);

    DSBP_PHT_line_t *choosen_line = NULL;

    switch (this->DSBP_PHT_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t PHT_way = 0; PHT_way < this->get_DSBP_PHT_associativity(); PHT_way++) {
                /// If the line is LRU
                if (this->DSBP_PHT_sets[PHT_index].ways[PHT_way].last_access <= last_access) {
                    choosen_line = &this->DSBP_PHT_sets[PHT_index].ways[PHT_way];
                    last_access = this->DSBP_PHT_sets[PHT_index].ways[PHT_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t PHT_way = (rand_r(&seed) % this->get_DSBP_PHT_associativity());
            choosen_line = &this->DSBP_PHT_sets[PHT_index].ways[PHT_way];
        }
        break;

        case REPLACEMENT_FIFO:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_FIFO not implemented.\n");
        break;

        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_LRF not implemented.\n");
        break;

        case REPLACEMENT_LRU_DSBP:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_LRU_DSBP should not use for line_usage_predictor.\n");
        break;
    }

    return choosen_line;
};
