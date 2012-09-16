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
#include "../sinuca.hpp"

#ifdef LINE_USAGE_PREDICTOR_DEBUG
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define LINE_USAGE_PREDICTOR_DEBUG_PRINTF(...)
#endif

/// ============================================================================
line_usage_predictor_dlec_t::line_usage_predictor_dlec_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DLEC;

    this->access_counter_bits = 0;
    this->access_counter_max = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;

    /// ahtm
    this->ahtm_sets = NULL;
    this->ahtm_line_number = 0;
    this->ahtm_associativity = 0;
    this->ahtm_total_sets = 0;
    this->ahtm_replacement_policy = REPLACEMENT_LRU;

    /// ahtc
    this->ahtc_sets = NULL;
    this->ahtc_line_number = 0;
    this->ahtc_associativity = 0;
    this->ahtc_total_sets = 0;
    this->ahtc_replacement_policy = REPLACEMENT_LRU;

};

/// ============================================================================
line_usage_predictor_dlec_t::~line_usage_predictor_dlec_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dlec_metadata_set_t>(metadata_sets);
    utils_t::template_delete_array<aht_set_t>(ahtm_sets);
    utils_t::template_delete_array<aht_set_t>(ahtc_sets);
};

/// ============================================================================
void line_usage_predictor_dlec_t::allocate() {
    line_usage_predictor_t::allocate();

    this->access_counter_max = pow(2, this->access_counter_bits) - 1;

    ///=========================================================================
    /// metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s metadata %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());
    this->metadata_sets = utils_t::template_allocate_array<dlec_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dlec_metadata_line_t>(this->get_metadata_associativity());
    }

    ///=========================================================================
    /// aht misses
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_ahtm_line_number() / this->get_ahtm_associativity()),
                        "Wrong line number(%u) or associativity(%u).\n", this->get_ahtm_line_number(), this->get_ahtm_associativity());
    this->set_ahtm_total_sets(this->get_ahtm_line_number() / this->get_ahtm_associativity());
    this->ahtm_sets = utils_t::template_allocate_array<aht_set_t>(this->get_ahtm_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s ahtm %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_ahtm_line_number(), this->get_ahtm_associativity(), this->get_ahtm_total_sets());

    /// INDEX MASK
    this->ahtm_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_ahtm_total_sets()); i++) {
        this->ahtm_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_ahtm_total_sets(); i++) {
        this->ahtm_sets[i].ways = utils_t::template_allocate_array<aht_line_t>(this->get_ahtm_associativity());
    }

    ///=========================================================================
    /// aht copyback
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_ahtc_line_number() / this->get_ahtc_associativity()),
            "Wrong line number(%u) or associativity(%u).\n", this->get_ahtc_line_number(), this->get_ahtc_associativity());
    this->set_ahtc_total_sets(this->get_ahtc_line_number() / this->get_ahtc_associativity());
    this->ahtc_sets = utils_t::template_allocate_array<aht_set_t>(this->get_ahtc_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s ahtc %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_ahtc_line_number(), this->get_ahtc_associativity(), this->get_ahtc_total_sets());

    /// INDEX MASK
    this->ahtc_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_ahtc_total_sets()); i++) {
        this->ahtc_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_ahtc_total_sets(); i++) {
        this->ahtc_sets[i].ways = utils_t::template_allocate_array<aht_line_t>(this->get_ahtc_associativity());
    }
};

/// ============================================================================
void line_usage_predictor_dlec_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


/// ============================================================================
void line_usage_predictor_dlec_t::fill_package_sub_blocks(memory_package_t *package) {
    (void)package;
};

/// ============================================================================
bool line_usage_predictor_dlec_t::check_sub_block_is_hit(memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    (void)package;

    if (this->metadata_sets[index].ways[way].valid_sub_blocks == LINE_SUB_BLOCK_DISABLE) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
        return false;
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
        return true;
    }
};

/// ============================================================================
bool line_usage_predictor_dlec_t::check_line_is_dead(uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_dead()\n")

    return (this->metadata_sets[index].ways[way].is_dead);
};

/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_dlec_t::line_hit(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(package->memory_operation != MEMORY_OPERATION_COPYBACK, "Line hit received a COPYBACK, this component is a LLC only.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].valid_sub_blocks != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].ahtm_pointer == NULL ||
                        this->metadata_sets[index].ways[way].ahtc_pointer == NULL, "Metadata has pointer for both ahtm and ahtc.\n");
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_hit();         /// Access Statistics

    /// Statistics
    this->metadata_sets[index].ways[way].stat_access_counter++;
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();

    if (package->memory_operation == MEMORY_OPERATION_WRITE) {
        this->metadata_sets[index].ways[way].stat_write_counter++;        
        /// First write
        if (this->metadata_sets[index].ways[way].stat_clock_first_write == 0) {
            this->metadata_sets[index].ways[way].stat_clock_first_write = sinuca_engine.get_global_cycle();
            this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();
        }
        /// NOT First Write
        else {
            this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();
        }
    }


    // Update the METADATA real_access_counter
    this->metadata_sets[index].ways[way].real_access_counter++;
    if (package->memory_operation == MEMORY_OPERATION_WRITE) {
        WARNING_PRINTF("Line hit received a WRITE, this component is a LLC only.\n");
        this->metadata_sets[index].ways[way].is_dirty = true;
    }

    /// ================================================================
    /// METADATA Learn Mode
    /// ================================================================
    if (this->metadata_sets[index].ways[way].learn_mode == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE ON\n");
        this->metadata_sets[index].ways[way].is_dead = false;
        // AHTM UPDATE
        if (this->metadata_sets[index].ways[way].ahtm_pointer != NULL &&
        this->metadata_sets[index].ways[way].ahtm_pointer->pointer == true) {

            // Check the aht Overflow
            if (this->metadata_sets[index].ways[way].ahtm_pointer->access_counter >= this->access_counter_max) {
                this->metadata_sets[index].ways[way].ahtm_pointer->overflow = true;
            }
            else {
                this->metadata_sets[index].ways[way].ahtm_pointer->access_counter++;
            }

            // AHT Statistics
            this->add_stat_ahtm_access();
            this->metadata_sets[index].ways[way].ahtm_pointer->last_access = sinuca_engine.get_global_cycle();
        }

        // AHTC UPDATE
        else if (this->metadata_sets[index].ways[way].ahtc_pointer != NULL &&
        this->metadata_sets[index].ways[way].ahtc_pointer->pointer == true) {

            // Check the aht Overflow
            if (this->metadata_sets[index].ways[way].ahtc_pointer->access_counter >= this->access_counter_max) {
                this->metadata_sets[index].ways[way].ahtc_pointer->overflow = true;
            }
            else {
                this->metadata_sets[index].ways[way].ahtc_pointer->access_counter++;
            }

            // AHT Statistics
            this->add_stat_ahtc_access();
            this->metadata_sets[index].ways[way].ahtc_pointer->last_access = sinuca_engine.get_global_cycle();
        }

    }
    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE OFF\n");
        this->metadata_sets[index].ways[way].is_dead = false;
        
        // METADATA Not Overflow + METADATA Used Predicted Number of Times
        if (this->metadata_sets[index].ways[way].overflow == false &&
        this->metadata_sets[index].ways[way].real_access_counter == this->metadata_sets[index].ways[way].access_counter) {
            //Predicted as DEAD
            this->metadata_sets[index].ways[way].is_dead = true;            
            if (this->metadata_sets[index].ways[way].is_dirty == false) {
                // METADATA Turn off the sub_blocks
                this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_DISABLE;
                this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
            }
        }
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
};



/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::line_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_line_miss();         /// Access Statistics

    /// Statistics
    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();

    // ~ // Clean the metadata entry
    // ~ this->metadata_sets[index].ways[way].clean();

    aht_line_t *ahtm_line = this->ahtm_find_line(package->opcode_address, package->memory_address);
    ///=================================================================
    /// ahtm HIT
    ///=================================================================
    if (ahtm_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t %s", ahtm_line->content_to_string().c_str())

        // Update the ahtm entry
        this->add_stat_ahtm_access();
        this->add_stat_ahtm_hit();
        ahtm_line->last_access = sinuca_engine.get_global_cycle();

        // If no ahtm_pointer
        if (ahtm_line->pointer == false) {
            // Copy AHTM prediction
            ahtm_line->pointer = true;
            this->metadata_sets[index].ways[way].ahtm_pointer = ahtm_line;
        }
        else {
            this->metadata_sets[index].ways[way].ahtm_pointer = NULL;
        }

        // Copy AHTM prediction
        this->metadata_sets[index].ways[way].learn_mode = false;
        this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_NORMAL;
        this->metadata_sets[index].ways[way].access_counter = ahtm_line->access_counter;
        this->metadata_sets[index].ways[way].overflow = ahtm_line->overflow;
        this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    }

    ///=================================================================
    /// ahtm MISS
    ///=================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm MISS\n")
        this->add_stat_ahtm_miss();
        // New ahtm entry
        ahtm_line = ahtm_evict_address(package->opcode_address, package->memory_address);
        // Clean the ahtm entry
        this->add_stat_ahtm_access();
        ahtm_line->opcode_address = package->opcode_address;
        ahtm_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
        ahtm_line->last_access = sinuca_engine.get_global_cycle();
        ahtm_line->pointer = true;
        ahtm_line->access_counter = 0;
        ahtm_line->overflow = false;

        // Copy AHTM pointer
        this->metadata_sets[index].ways[way].ahtm_pointer = ahtm_line;

        // Enable Learn Mode
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_LEARN;
        this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    }

    // Modify the package->sub_blocks (next level request)
    package->memory_size = sinuca_engine.get_global_line_size();
};


/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::sub_block_miss(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_sub_block_miss();         /// Access Statistics

    ///=================================================================
    /// aht_M HIT
    ///=================================================================
    if (this->metadata_sets[index].ways[way].ahtm_pointer != NULL &&
    this->metadata_sets[index].ways[way].ahtm_pointer->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtm_pointer->content_to_string().c_str())

        // Update the aht entry
        this->add_stat_ahtm_access();
        this->metadata_sets[index].ways[way].ahtm_pointer->last_access = sinuca_engine.get_global_cycle();

        if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
            this->metadata_sets[index].ways[way].ahtm_pointer->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
            this->metadata_sets[index].ways[way].ahtm_pointer->overflow = false;
        }
        else {
            this->metadata_sets[index].ways[way].ahtm_pointer->access_counter = this->access_counter_max;
            this->metadata_sets[index].ways[way].ahtm_pointer->overflow = true;
        }
    }
    ///=================================================================
    /// aht_C HIT
    ///=================================================================
    else if (this->metadata_sets[index].ways[way].ahtc_pointer != NULL &&
    this->metadata_sets[index].ways[way].ahtc_pointer->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtc_pointer->content_to_string().c_str())

        // Update the aht entry
        this->add_stat_ahtc_access();
        this->metadata_sets[index].ways[way].ahtc_pointer->last_access = sinuca_engine.get_global_cycle();

        if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
            this->metadata_sets[index].ways[way].ahtc_pointer->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
            this->metadata_sets[index].ways[way].ahtc_pointer->overflow = false;
        }
        else {
            this->metadata_sets[index].ways[way].ahtc_pointer->access_counter = this->access_counter_max;
            this->metadata_sets[index].ways[way].ahtc_pointer->overflow = true;
        }
    }

    // Modify the package->sub_blocks (next level request)
    package->memory_size = sinuca_engine.get_global_line_size();

    // Enable Learn_mode
    this->metadata_sets[index].ways[way].learn_mode = true;
    this->metadata_sets[index].ways[way].is_dirty = false;
    this->metadata_sets[index].ways[way].is_dead = false;
    // ~ this->metadata_sets[index].ways[way].real_access_counter = 0;
    this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_WRONG_FIRST;
    this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    this->metadata_sets[index].ways[way].access_counter = 0;
    this->metadata_sets[index].ways[way].overflow = true;
};

/// ============================================================================
// Collateral Effect: Change the package->sub_blocks[]
void line_usage_predictor_dlec_t::line_recv_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_recv_copyback();         /// Access Statistics

    /// Statistics
    this->metadata_sets[index].ways[way].stat_write_counter++;
    this->metadata_sets[index].ways[way].stat_clock_last_write = sinuca_engine.get_global_cycle();

    this->metadata_sets[index].ways[way].stat_clock_first_read = sinuca_engine.get_global_cycle();    
    this->metadata_sets[index].ways[way].stat_clock_last_read = sinuca_engine.get_global_cycle();    
    
    /// First write
    if (this->metadata_sets[index].ways[way].stat_clock_first_write == 0) {
        this->metadata_sets[index].ways[way].stat_clock_first_write = sinuca_engine.get_global_cycle();

    }


    aht_line_t *ahtc_line = this->ahtc_find_line(package->opcode_address, package->memory_address);
    ///=================================================================
    /// ahtc HIT
    ///=================================================================
    if (ahtc_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t %s", ahtc_line->content_to_string().c_str())

        // Update the ahtc entry
        this->add_stat_ahtc_access();
        this->add_stat_ahtc_hit();
        ahtc_line->last_access = sinuca_engine.get_global_cycle();

        // If no ahtc_pointer
        if (ahtc_line->pointer == false) {
            // Copy AHTM prediction
            ahtc_line->pointer = true;
            this->metadata_sets[index].ways[way].ahtc_pointer = ahtc_line;
        }
        else {
            this->metadata_sets[index].ways[way].ahtc_pointer = NULL;
        }

        // Copy AHTM prediction
        this->metadata_sets[index].ways[way].learn_mode = false;
        this->metadata_sets[index].ways[way].is_dead = false;
        this->metadata_sets[index].ways[way].is_dirty = true;
        this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_COPYBACK;

        this->metadata_sets[index].ways[way].is_last_write = ahtc_line->is_last_write;        
        this->metadata_sets[index].ways[way].access_counter = ahtc_line->access_counter;
        this->metadata_sets[index].ways[way].overflow = ahtc_line->overflow;

        this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    }

    ///=================================================================
    /// ahtc MISS
    ///=================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc MISS\n")
        // New ahtc entry
        ahtc_line = ahtc_evict_address(package->opcode_address, package->memory_address);
        // Clean the ahtc entry
        this->add_stat_ahtc_access();
        this->add_stat_ahtc_miss();
        ahtc_line->opcode_address = package->opcode_address;
        ahtc_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
        ahtc_line->last_access = sinuca_engine.get_global_cycle();
        ahtc_line->pointer = true;
        ahtc_line->access_counter = 0;
        ahtc_line->overflow = false;

        // Copy AHTM pointer
        this->metadata_sets[index].ways[way].ahtc_pointer = ahtc_line;

        // Enable Learn Mode
        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].is_dead = false;
        this->metadata_sets[index].ways[way].is_dirty = true;
        this->metadata_sets[index].ways[way].valid_sub_blocks = LINE_SUB_BLOCK_LEARN;
        this->metadata_sets[index].ways[way].clock_become_alive = sinuca_engine.get_global_cycle();
        this->metadata_sets[index].ways[way].clock_become_dead = sinuca_engine.get_global_cycle();
    }

    // Modify the package->sub_blocks (next level request)
    package->memory_size = 1;
};


/// ============================================================================
void line_usage_predictor_dlec_t::line_send_copyback(memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_copy_back() package:%s\n", package->content_to_string().c_str())
    this->add_stat_send_copyback();         /// Access Statistics
    
    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_dlec_t::line_eviction(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_eviction();         /// Access Statistics
    
    switch (this->metadata_sets[index].ways[way].valid_sub_blocks) {
        case LINE_SUB_BLOCK_LEARN:
            this->add_stat_line_sub_block_learn();
        break;

        case LINE_SUB_BLOCK_NORMAL:
            if (this->metadata_sets[index].ways[way].overflow) {
                this->add_stat_line_sub_block_normal_correct();
            }
            else {
                this->add_stat_line_sub_block_normal_over();
            }
        break;

        case LINE_SUB_BLOCK_DISABLE:
            this->add_stat_line_sub_block_disable_correct();
        break;

        case LINE_SUB_BLOCK_WRONG_FIRST:
            this->add_stat_line_sub_block_disable_under();
        break;
    
        case LINE_SUB_BLOCK_COPYBACK:
            if (this->metadata_sets[index].ways[way].is_last_write) {
                this->add_stat_line_sub_block_copyback_correct();
            }
            else {
                this->add_stat_line_sub_block_copyback_over();
            }
        break;
    }

    //==================================================================
    // Statistics

    // Accesses before eviction
    if (this->metadata_sets[index].ways[way].stat_access_counter == 0) {
        this->add_stat_line_access_0();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter == 1) {
        this->add_stat_line_access_1();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 2 && this->metadata_sets[index].ways[way].stat_access_counter <= 3) {
        this->add_stat_line_access_2_3();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 4 && this->metadata_sets[index].ways[way].stat_access_counter <= 7) {
        this->add_stat_line_access_4_7();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 8 && this->metadata_sets[index].ways[way].stat_access_counter <= 15) {
        this->add_stat_line_access_8_15();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >= 16 && this->metadata_sets[index].ways[way].stat_access_counter <= 127) {
        this->add_stat_line_access_16_127();
    }
    else if (this->metadata_sets[index].ways[way].stat_access_counter >=128) {
        this->add_stat_line_access_128_bigger();
    }

    // Writes before eviction
    if (this->metadata_sets[index].ways[way].stat_write_counter == 0) {
        this->add_stat_line_write_0();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter == 1) {
        this->add_stat_line_write_1();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 2 && this->metadata_sets[index].ways[way].stat_write_counter <= 3) {
        this->add_stat_line_write_2_3();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 4 && this->metadata_sets[index].ways[way].stat_write_counter <= 7) {
        this->add_stat_line_write_4_7();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 8 && this->metadata_sets[index].ways[way].stat_write_counter <= 15) {
        this->add_stat_line_write_8_15();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >= 16 && this->metadata_sets[index].ways[way].stat_write_counter <= 127) {
        this->add_stat_line_write_16_127();
    }
    else if (this->metadata_sets[index].ways[way].stat_write_counter >=128) {
        this->add_stat_line_write_128_bigger();
    }

    // ~ uint64_t stat_clock_first_read;
    // ~ uint64_t stat_clock_last_read;
    // ~ uint64_t stat_clock_first_write;
    // ~ uint64_t stat_clock_last_write;

    this->cycles_last_write_to_last_access += this->metadata_sets[index].ways[way].stat_clock_last_read - this->metadata_sets[index].ways[way].stat_clock_last_write;
    this->cycles_last_write_to_eviction += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_write;
    this->cycles_last_access_to_eviction += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].stat_clock_last_read;
    
    this->metadata_sets[index].ways[way].reset_statistics();

    /// AHT_M
    if (this->metadata_sets[index].ways[way].is_dirty == false) {
        aht_line_t *ahtm_line = this->metadata_sets[index].ways[way].ahtm_pointer;
        // ahtm HIT
        if (ahtm_line != NULL) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm HIT\n")
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtm_pointer->content_to_string().c_str())
            // Update the ahtm entry
            this->add_stat_ahtm_access();
            ahtm_line->last_access = sinuca_engine.get_global_cycle();

            if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
                ahtm_line->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
                ahtm_line->overflow = false;
            }
            else {
                ahtm_line->access_counter = this->access_counter_max;
                ahtm_line->overflow = true;
            }
            ahtm_line->pointer = false;
            this->metadata_sets[index].ways[way].ahtm_pointer = NULL;
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", ahtm_line->content_to_string().c_str())
        }
        else {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm MISS\n")
        }
    }
    /// AHT_C
    else {
        aht_line_t *ahtc_line = this->metadata_sets[index].ways[way].ahtc_pointer;
        // ahtc HIT
        if (ahtc_line != NULL) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc HIT\n")
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtc_pointer->content_to_string().c_str())
            // Update the ahtc entry
            this->add_stat_ahtc_access();
            ahtc_line->last_access = sinuca_engine.get_global_cycle();
            ahtc_line->is_last_write = true;
            if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
                ahtc_line->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
                ahtc_line->overflow = false;
            }
            else {
                ahtc_line->access_counter = this->access_counter_max;
                ahtc_line->overflow = true;
            }
            ahtc_line->pointer = false;
            this->metadata_sets[index].ways[way].ahtc_pointer = NULL;
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", ahtc_line->content_to_string().c_str())
        }
        else {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc MISS\n")
        }
    }
    this->metadata_sets[index].ways[way].clean();

};

/// ============================================================================
void line_usage_predictor_dlec_t::line_invalidation(uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    this->add_stat_invalidation();         /// Access Statistics

    switch (this->metadata_sets[index].ways[way].valid_sub_blocks) {
        case LINE_SUB_BLOCK_LEARN:
            this->add_stat_line_sub_block_learn();
        break;

        case LINE_SUB_BLOCK_NORMAL:
            if (this->metadata_sets[index].ways[way].overflow) {
                this->add_stat_line_sub_block_normal_correct();
            }
            else {
                this->add_stat_line_sub_block_normal_over();
            }
        break;

        case LINE_SUB_BLOCK_DISABLE:
            this->add_stat_line_sub_block_disable_correct();
        break;

        case LINE_SUB_BLOCK_WRONG_FIRST:
            this->add_stat_line_sub_block_disable_under();
        break;
    
        case LINE_SUB_BLOCK_COPYBACK:
            if (this->metadata_sets[index].ways[way].is_last_write) {
                this->add_stat_line_sub_block_copyback_under();
            }
            else {
                this->add_stat_line_sub_block_copyback_correct();
            }
        break;
    }

    /// AHT_M
    if (this->metadata_sets[index].ways[way].is_dirty == false) {
        aht_line_t *ahtm_line = this->metadata_sets[index].ways[way].ahtm_pointer;
        // ahtm HIT
        if (ahtm_line != NULL) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm HIT\n")
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtm_pointer->content_to_string().c_str())
            // Update the ahtm entry
            this->add_stat_ahtm_access();
            ahtm_line->last_access = sinuca_engine.get_global_cycle();

            if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
                ahtm_line->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
                ahtm_line->overflow = false;
            }
            else {
                ahtm_line->access_counter = this->access_counter_max;
                ahtm_line->overflow = true;
            }
            ahtm_line->pointer = false;
            this->metadata_sets[index].ways[way].ahtm_pointer = NULL;
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", ahtm_line->content_to_string().c_str())
        }
        else {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm MISS\n")
        }
    }
    /// AHT_C
    else {
        aht_line_t *ahtc_line = this->metadata_sets[index].ways[way].ahtc_pointer;
        // ahtc HIT
        if (ahtc_line != NULL) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc HIT\n")
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].ahtc_pointer->content_to_string().c_str())
            // Update the ahtc entry
            this->add_stat_ahtc_access();
            ahtc_line->last_access = sinuca_engine.get_global_cycle();
            ahtc_line->is_last_write = false;
            if (this->metadata_sets[index].ways[way].real_access_counter <= this->access_counter_max) {
                ahtc_line->access_counter = this->metadata_sets[index].ways[way].real_access_counter;
                ahtc_line->overflow = false;
            }
            else {
                ahtc_line->access_counter = this->access_counter_max;
                ahtc_line->overflow = true;
            }
            ahtc_line->pointer = false;
            this->metadata_sets[index].ways[way].ahtc_pointer = NULL;
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", ahtc_line->content_to_string().c_str())
        }
        else {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc MISS\n")
        }
    }
    this->metadata_sets[index].ways[way].clean();
};


/// ============================================================================
/// aht Miss
/// ============================================================================
aht_line_t* line_usage_predictor_dlec_t::ahtm_find_line(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("ahtm_find_line()\n")
    uint32_t ahtm_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t ahtm_index = opcode_address & this->ahtm_index_bits_mask;

    ERROR_ASSERT_PRINTF(ahtm_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", ahtm_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(ahtm_index < this->ahtm_total_sets, "Wrong index %d > total_sets %d", ahtm_index, this->ahtm_total_sets);

    for (uint32_t ahtm_way = 0; ahtm_way < this->get_ahtm_associativity(); ahtm_way++) {
        if (this->ahtm_sets[ahtm_index].ways[ahtm_way].opcode_address == opcode_address && this->ahtm_sets[ahtm_index].ways[ahtm_way].offset == ahtm_offset) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Found ahtm Index %u - Way %u\n", ahtm_index, ahtm_way )
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm %s \n", this->ahtm_sets[ahtm_index].ways[ahtm_way].content_to_string().c_str())
            return &this->ahtm_sets[ahtm_index].ways[ahtm_way];
        }
    }
    return NULL;
}

/// ============================================================================
aht_line_t* line_usage_predictor_dlec_t::ahtm_evict_address(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("ahtm_evict_address()\n")
    uint32_t ahtm_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t ahtm_index = opcode_address & this->ahtm_index_bits_mask;

    ERROR_ASSERT_PRINTF(ahtm_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", ahtm_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(ahtm_index < this->ahtm_total_sets, "Wrong index %d > total_sets %d", ahtm_index, this->ahtm_total_sets);

    aht_line_t *choosen_line = NULL;

    switch (this->ahtm_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t ahtm_way = 0; ahtm_way < this->get_ahtm_associativity(); ahtm_way++) {
                /// If the line is LRU
                if (this->ahtm_sets[ahtm_index].ways[ahtm_way].last_access <= last_access) {
                    choosen_line = &this->ahtm_sets[ahtm_index].ways[ahtm_way];
                    last_access = this->ahtm_sets[ahtm_index].ways[ahtm_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t ahtm_way = (rand_r(&seed) % this->get_ahtm_associativity());
            choosen_line = &this->ahtm_sets[ahtm_index].ways[ahtm_way];
        }
        break;

        case REPLACEMENT_INVALID_OR_LRU:
        case REPLACEMENT_DEAD_OR_LRU:
        case REPLACEMENT_FIFO:
        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: %s not implemented.\n",  get_enum_replacement_char(this->ahtm_replacement_policy));
        break;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtm %s \n", choosen_line->content_to_string().c_str())
    return choosen_line;
};

/// ============================================================================
/// aht Copyback
/// ============================================================================
aht_line_t* line_usage_predictor_dlec_t::ahtc_find_line(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("ahtc_find_line()\n")
    uint32_t ahtc_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t ahtc_index = opcode_address & this->ahtc_index_bits_mask;

    ERROR_ASSERT_PRINTF(ahtc_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", ahtc_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(ahtc_index < this->ahtc_total_sets, "Wrong index %d > total_sets %d", ahtc_index, this->ahtc_total_sets);

    for (uint32_t ahtc_way = 0; ahtc_way < this->get_ahtc_associativity(); ahtc_way++) {
        if (this->ahtc_sets[ahtc_index].ways[ahtc_way].opcode_address == opcode_address && this->ahtc_sets[ahtc_index].ways[ahtc_way].offset == ahtc_offset) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Found ahtc Index %u - Way %u\n", ahtc_index, ahtc_way )
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc %s \n", this->ahtc_sets[ahtc_index].ways[ahtc_way].content_to_string().c_str())
            return &this->ahtc_sets[ahtc_index].ways[ahtc_way];
        }
    }
    return NULL;
}

/// ============================================================================
aht_line_t* line_usage_predictor_dlec_t::ahtc_evict_address(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("ahtc_evict_address()\n")
    uint32_t ahtc_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t ahtc_index = opcode_address & this->ahtc_index_bits_mask;

    ERROR_ASSERT_PRINTF(ahtc_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", ahtc_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(ahtc_index < this->ahtc_total_sets, "Wrong index %d > total_sets %d", ahtc_index, this->ahtc_total_sets);

    aht_line_t *choosen_line = NULL;

    switch (this->ahtc_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t ahtc_way = 0; ahtc_way < this->get_ahtc_associativity(); ahtc_way++) {
                /// If the line is LRU
                if (this->ahtc_sets[ahtc_index].ways[ahtc_way].last_access <= last_access) {
                    choosen_line = &this->ahtc_sets[ahtc_index].ways[ahtc_way];
                    last_access = this->ahtc_sets[ahtc_index].ways[ahtc_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t ahtc_way = (rand_r(&seed) % this->get_ahtc_associativity());
            choosen_line = &this->ahtc_sets[ahtc_index].ways[ahtc_way];
        }
        break;

        case REPLACEMENT_INVALID_OR_LRU:
        case REPLACEMENT_DEAD_OR_LRU:
        case REPLACEMENT_FIFO:
        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: %s not implemented.\n",  get_enum_replacement_char(this->ahtc_replacement_policy));
        break;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t ahtc %s \n", choosen_line == NULL ? "NULL" : choosen_line->content_to_string().c_str())
    return choosen_line;
};


/// ============================================================================
void line_usage_predictor_dlec_t::print_structures() {
    line_usage_predictor_t::print_structures();
};

/// ============================================================================
void line_usage_predictor_dlec_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_dlec_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_dlec_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_copyback = 0;
    this->stat_recv_copyback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    this->stat_ahtm_access = 0;
    this->stat_ahtm_hit = 0;
    this->stat_ahtm_miss = 0;

    this->stat_ahtc_access = 0;
    this->stat_ahtc_hit = 0;
    this->stat_ahtc_miss = 0;

    this->stat_line_sub_block_learn = 0;
    this->stat_line_sub_block_normal_over = 0;
    this->stat_line_sub_block_normal_correct = 0;
    this->stat_line_sub_block_disable_correct = 0;
    this->stat_line_sub_block_disable_under = 0;

    this->stat_line_sub_block_copyback_over = 0;
    this->stat_line_sub_block_copyback_correct = 0;
    this->stat_line_sub_block_copyback_under = 0;

    this->stat_line_access_0 = 0;
    this->stat_line_access_1 = 0;
    this->stat_line_access_2_3 = 0;
    this->stat_line_access_4_7 = 0;
    this->stat_line_access_8_15 = 0;
    this->stat_line_access_16_127 = 0;
    this->stat_line_access_128_bigger = 0;

    this->stat_line_write_0 = 0;
    this->stat_line_write_1 = 0;
    this->stat_line_write_2_3 = 0;
    this->stat_line_write_4_7 = 0;
    this->stat_line_write_8_15 = 0;
    this->stat_line_write_16_127 = 0;
    this->stat_line_write_128_bigger = 0;

    this->cycles_last_write_to_last_access = 0;
    this->cycles_last_write_to_eviction = 0;
    this->cycles_last_access_to_eviction = 0;
     
};

/// ============================================================================
void line_usage_predictor_dlec_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    for (uint32_t index = 0; index < this->get_metadata_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_metadata_associativity(); way++) {
            this->line_eviction(index, way);
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_hit", stat_line_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_send_copyback", stat_send_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_recv_copyback", stat_recv_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_access", stat_ahtm_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_hit", stat_ahtm_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtm_miss", stat_ahtm_miss);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_access", stat_ahtc_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_hit", stat_ahtc_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_ahtc_miss", stat_ahtc_miss);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_learn", stat_line_sub_block_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_normal_over", stat_line_sub_block_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_normal_correct", stat_line_sub_block_normal_correct);
            
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_disable_correct", stat_line_sub_block_disable_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_disable_under", stat_line_sub_block_disable_under);
    
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_copyback_over", stat_line_sub_block_copyback_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_copyback_correct", stat_line_sub_block_copyback_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_sub_block_copyback_under", stat_line_sub_block_copyback_under);        

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_0", stat_line_access_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_1", stat_line_access_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_2_3", stat_line_access_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_4_7", stat_line_access_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_8_15", stat_line_access_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_16_127", stat_line_access_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_access_128_bigger", stat_line_access_128_bigger);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_0", stat_line_write_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_1", stat_line_write_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_2_3", stat_line_write_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_4_7", stat_line_write_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_8_15", stat_line_write_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_16_127", stat_line_write_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_write_128_bigger", stat_line_write_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_write_to_last_access", cycles_last_write_to_last_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_write_to_eviction", cycles_last_write_to_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_last_access_to_eviction", cycles_last_access_to_eviction);

};

/// ============================================================================
void line_usage_predictor_dlec_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_bits", access_counter_bits);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_max", access_counter_max);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// ahtm
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_line_number", ahtm_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_associativity", ahtm_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_total_sets", ahtm_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtm_replacement_policy", get_enum_replacement_char(ahtm_replacement_policy));

    /// ahtc
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_line_number", ahtc_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_associativity", ahtc_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_total_sets", ahtc_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "ahtc_replacement_policy", get_enum_replacement_char(ahtc_replacement_policy));
};
