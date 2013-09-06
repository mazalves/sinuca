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
line_usage_predictor_dewp_t::line_usage_predictor_dewp_t() {
    this->line_usage_predictor_type = LINE_USAGE_PREDICTOR_POLICY_DEWP;

    this->early_eviction = true;
    this->early_writeback = true;
    this->turnoff_dead_lines = true;

    this->access_counter_bits_read = 0;
    this->access_counter_bits_writeback = 0;

    this->access_counter_max_read = 0;
    this->access_counter_max_writeback = 0;

    /// metadata
    this->metadata_sets = NULL;
    this->metadata_line_number = 0;
    this->metadata_associativity = 0;
    this->metadata_total_sets = 0;

    /// aht
    this->aht_sets = NULL;
    this->aht_line_number = 0;
    this->aht_associativity = 0;
    this->aht_total_sets = 0;
    this->aht_replacement_policy = REPLACEMENT_LRU;

};

/// ============================================================================
line_usage_predictor_dewp_t::~line_usage_predictor_dewp_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<dewp_metadata_set_t>(metadata_sets);
    utils_t::template_delete_array<aht_set_t>(aht_sets);
};

/// ============================================================================
void line_usage_predictor_dewp_t::allocate() {
    line_usage_predictor_t::allocate();

    this->access_counter_max_read = pow(2, this->access_counter_bits_read) - 1;
    this->access_counter_max_writeback = pow(2, this->access_counter_bits_writeback) - 1;

    ///=========================================================================
    /// metadata
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_metadata_line_number() / this->get_metadata_associativity()), "Wrong line_number(%u) or associativity(%u).\n", this->get_metadata_line_number(), this->get_metadata_associativity());
    this->set_metadata_total_sets(this->get_metadata_line_number() / this->get_metadata_associativity());

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s metadata %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_metadata_line_number(), this->get_metadata_associativity(), this->get_metadata_total_sets());
    this->metadata_sets = utils_t::template_allocate_array<dewp_metadata_set_t>(this->get_metadata_total_sets());
    for (uint32_t i = 0; i < this->get_metadata_total_sets(); i++) {
        this->metadata_sets[i].ways = utils_t::template_allocate_array<dewp_metadata_line_t>(this->get_metadata_associativity());
    }

    ///=========================================================================
    /// aht misses
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_aht_line_number() / this->get_aht_associativity()),
                        "Wrong line number(%u) or associativity(%u).\n", this->get_aht_line_number(), this->get_aht_associativity());
    this->set_aht_total_sets(this->get_aht_line_number() / this->get_aht_associativity());
    this->aht_sets = utils_t::template_allocate_array<aht_set_t>(this->get_aht_total_sets());
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("Allocate %s aht %d(lines) / %d(assoc) = %d (sets))\n",
            this->get_label(), this->get_aht_line_number(), this->get_aht_associativity(), this->get_aht_total_sets());

    /// INDEX MASK
    this->aht_index_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(this->get_aht_total_sets()); i++) {
        this->aht_index_bits_mask |= 1 << (i);
    }

    for (uint32_t i = 0; i < this->get_aht_total_sets(); i++) {
        this->aht_sets[i].ways = utils_t::template_allocate_array<aht_line_t>(this->get_aht_associativity());
    }
};

/// ============================================================================
void line_usage_predictor_dewp_t::clock(uint32_t subcycle) {
    line_usage_predictor_t::clock(subcycle);

    if (subcycle != 0) return;
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("==================== ");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("====================\n");
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("cycle() \n");
};


/// ============================================================================
void line_usage_predictor_dewp_t::fill_package_sub_blocks(memory_package_t *package) {
    (void)package;
};

/// ============================================================================
void line_usage_predictor_dewp_t::line_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
void line_usage_predictor_dewp_t::predict_sub_blocks_to_package(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("predict_sub_blocks_to_package() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;
    (void)index;
    (void)way;

    package->memory_size = sinuca_engine.get_global_line_size();
};

/// ============================================================================
bool line_usage_predictor_dewp_t::check_sub_block_is_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint64_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("fill_package_sub_blocks() package:%s\n", package->content_to_string().c_str())

    (void)cache;
    (void)cache_line;
    (void)package;

    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks MISS\n")
        return false;
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t sub_blocks HIT\n")
        return true;
    }
};

/// ============================================================================
bool line_usage_predictor_dewp_t::check_line_is_last_access(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    return (this->metadata_sets[index].ways[way].learn_mode == false &&
            this->metadata_sets[index].ways[way].is_dead_read &&
            this->metadata_sets[index].ways[way].is_dead_writeback &&
            this->early_eviction);
};

/// ============================================================================
bool line_usage_predictor_dewp_t::check_line_is_last_write(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way){
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("check_line_is_last_access()\n")

    (void)cache;
    (void)cache_line;
    return (this->metadata_sets[index].ways[way].learn_mode == false &&
            this->metadata_sets[index].ways[way].is_dead_writeback &&
            this->metadata_sets[index].ways[way].is_dirty &&
            this->early_writeback);
};

/// ============================================================================
/// Cache Memory Operations
/// ============================================================================
void line_usage_predictor_dewp_t::line_hit(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_hit() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(package->memory_operation != MEMORY_OPERATION_WRITEBACK && package->memory_operation != MEMORY_OPERATION_WRITE, "Line hit received a WRITEBACK or WRITE.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_DISABLE, "Line hit into a DISABLED sub_block.\n");
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    /// Mechanism statistics
    this->add_stat_line_hit();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }

    this->metadata_sets[index].ways[way].real_access_counter_read++;

    /// ================================================================
    /// METADATA Learn Mode
    /// ================================================================
    if (this->metadata_sets[index].ways[way].learn_mode == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE ON\n");

        // AHT UPDATE
        if (this->metadata_sets[index].ways[way].aht_pointer != NULL &&
        this->metadata_sets[index].ways[way].aht_pointer->pointer == true) {
            // AHT Statistics
            this->add_stat_aht_access();
            this->add_stat_aht_hit();

            // Update the aht entry
            this->metadata_sets[index].ways[way].aht_pointer->last_access = sinuca_engine.get_global_cycle();

            // Check the aht Overflow
            if (this->metadata_sets[index].ways[way].aht_pointer->access_counter_read >= this->access_counter_max_read) {
                this->metadata_sets[index].ways[way].aht_pointer->overflow_read = true;
            }
            else {
                this->metadata_sets[index].ways[way].aht_pointer->access_counter_read++;
            }
        }
    }
    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE OFF\n");
        //Predicted as DEAD-READ
        if (this->metadata_sets[index].ways[way].overflow_read == false &&
        this->metadata_sets[index].ways[way].real_access_counter_read == this->metadata_sets[index].ways[way].access_counter_read) {
            this->metadata_sets[index].ways[way].is_dead_read = true;
            // Can Turn off the sub_blocks
            if (this->metadata_sets[index].ways[way].is_dirty == false && this->turnoff_dead_lines == true) {
                this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
            }
        }
    }
    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
};



/// ============================================================================
void line_usage_predictor_dewp_t::line_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE, "Metadata Line should be clean\n");
    (void)cache;
    (void)cache_line;
    /// Mechanism statistics
    this->add_stat_line_miss();

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }

    aht_line_t *aht_line = this->aht_find_line(package->opcode_address, package->memory_address);
    ///=================================================================
    /// AHT HIT
    ///=================================================================
    if (aht_line != NULL) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t %s", aht_line->content_to_string().c_str())
        // Statistics
        this->add_stat_aht_access();
        this->add_stat_aht_hit();

        // Clean the entries
        this->metadata_sets[index].ways[way].clean();

        aht_line->last_access = sinuca_engine.get_global_cycle();

        // If no aht_pointer
        if (aht_line->pointer == false) {
            aht_line->pointer = true;
            this->metadata_sets[index].ways[way].aht_pointer = aht_line;
        }
        else {
            this->metadata_sets[index].ways[way].aht_pointer = NULL;
        }

        // Copy AHT prediction
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;

        this->metadata_sets[index].ways[way].access_counter_read = aht_line->access_counter_read;
        this->metadata_sets[index].ways[way].access_counter_writeback = aht_line->access_counter_writeback;

        this->metadata_sets[index].ways[way].overflow_read = aht_line->overflow_read;
        this->metadata_sets[index].ways[way].overflow_writeback = aht_line->overflow_writeback;

        if (this->metadata_sets[index].ways[way].overflow_writeback == false &&
        this->metadata_sets[index].ways[way].access_counter_writeback == 0) {
            //Predicted as DEAD-WRITEBACK
            this->metadata_sets[index].ways[way].is_dead_writeback = true;
        }
        else {
            this->metadata_sets[index].ways[way].is_dead_writeback = false;
        }

        //Predicted as DEAD-READ
        if (this->metadata_sets[index].ways[way].overflow_read == false &&
        this->metadata_sets[index].ways[way].access_counter_read == 0) {
            this->metadata_sets[index].ways[way].is_dead_read = true;
        }
        else {
            this->metadata_sets[index].ways[way].is_dead_read = false;
        }
    }

    ///=================================================================
    /// AHT MISS
    ///=================================================================
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht MISS\n")
        // Statistics
        this->add_stat_aht_access();
        this->add_stat_aht_miss();

        // New aht entry
        aht_line = aht_evict_address(package->opcode_address, package->memory_address);

        // Clean the entries
        aht_line->clean();
        this->metadata_sets[index].ways[way].clean();

        aht_line->opcode_address = package->opcode_address;
        aht_line->offset = package->memory_address & sinuca_engine.get_global_offset_bits_mask();
        aht_line->last_access = sinuca_engine.get_global_cycle();

        aht_line->pointer = true;
        this->metadata_sets[index].ways[way].aht_pointer = aht_line;

        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_LEARN;
        this->metadata_sets[index].ways[way].learn_mode = true;
    }
    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
void line_usage_predictor_dewp_t::sub_block_miss(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("sub_block_miss() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    /// Mechanism statistics
    this->add_stat_sub_block_miss();

    (void)package;

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }


    // Enable Learn_mode
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRONG_FIRST;

    this->metadata_sets[index].ways[way].learn_mode = true;
    this->metadata_sets[index].ways[way].is_dead_read = false;
    this->metadata_sets[index].ways[way].is_dead_writeback = false;

    this->metadata_sets[index].ways[way].access_counter_read = 0;
    this->metadata_sets[index].ways[way].access_counter_writeback = 0;

    this->metadata_sets[index].ways[way].overflow_read = true;
    this->metadata_sets[index].ways[way].overflow_writeback = true;

    ///=================================================================
    /// AHT HIT
    ///=================================================================
    if (this->metadata_sets[index].ways[way].aht_pointer != NULL &&
    this->metadata_sets[index].ways[way].aht_pointer->pointer == true) {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].aht_pointer->content_to_string().c_str())
        // Statistics
        this->add_stat_aht_access();
        this->add_stat_aht_hit();

        // Update the aht entry
        this->metadata_sets[index].ways[way].aht_pointer->last_access = sinuca_engine.get_global_cycle();

        if (this->metadata_sets[index].ways[way].real_access_counter_read <= this->access_counter_max_read) {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_read = this->metadata_sets[index].ways[way].real_access_counter_read;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_read = false;
        }
        else {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_read = this->access_counter_max_read;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_read = true;
        }
    }


    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dewp_t::line_send_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_send_writeback() package:%s\n", package->content_to_string().c_str())
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].learn_mode == false, "Learn mode should be enabled.\n")
    (void)cache;
    (void)cache_line;
    this->add_stat_send_writeback();         /// Access Statistics

    (void)package;

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }

    /// ================================================================
    /// METADATA Not Learn Mode
    /// ================================================================
    this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRITEBACK;
    // If predicted as dead
    if (this->metadata_sets[index].ways[way].is_dead_read == true && this->turnoff_dead_lines == true) {
        // METADATA Turn off the sub_blocks
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    }
    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dewp_t::line_recv_writeback(cache_memory_t *cache, cache_line_t *cache_line, memory_package_t *package, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_recv_writeback() package:%s\n", package->content_to_string().c_str())
    // ~ ERROR_ASSERT_PRINTF(package->memory_operation == MEMORY_OPERATION_WRITEBACK, "Line hit received a WRITEBACK, this component is a LLC only.\n");
    ERROR_ASSERT_PRINTF(this->metadata_sets[index].ways[way].line_status != LINE_SUB_BLOCK_WRITEBACK, "Line writeback into a already written back sub_block (should be in wrong_first).\n");
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    /// Mechanism statistics
    this->add_stat_recv_writeback();

    (void)package;

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }

    this->metadata_sets[index].ways[way].real_access_counter_writeback++;
    this->metadata_sets[index].ways[way].is_dirty = true;

    ///=========================================================================
    /// If it was correct predicted
    ///=========================================================================
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_NORMAL;

        /// ================================================================
        /// METADATA Learn Mode
        /// ================================================================
        if (this->metadata_sets[index].ways[way].learn_mode == true) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE ON\n");
            // AHT UPDATE
            if (this->metadata_sets[index].ways[way].aht_pointer != NULL &&
            this->metadata_sets[index].ways[way].aht_pointer->pointer == true) {
                // Statistics
                this->add_stat_aht_access();
                this->add_stat_aht_hit();

                // Update the aht entry
                this->metadata_sets[index].ways[way].aht_pointer->last_access = sinuca_engine.get_global_cycle();

                // Check the aht Overflow
                if (this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback >= this->access_counter_max_writeback) {
                    this->metadata_sets[index].ways[way].aht_pointer->overflow_writeback = true;
                }
                else {
                    this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback++;
                }
            }
        }
        /// ================================================================
        /// METADATA Not Learn Mode
        /// ================================================================
        else {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t LEARN MODE OFF\n");
            // METADATA Not Overflow + METADATA Used Predicted Number of Times
            if (this->metadata_sets[index].ways[way].overflow_writeback == false &&
            this->metadata_sets[index].ways[way].real_access_counter_writeback == this->metadata_sets[index].ways[way].access_counter_writeback) {
                //Predicted as DEAD
                this->metadata_sets[index].ways[way].is_dead_writeback = true;
            }
        }

    }
    ///=========================================================================
    /// Misspredicted (already did the write_back)
    ///=========================================================================
    else if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_WRITEBACK) {
        // Enable Learn_mode
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_WRONG_FIRST;

        this->metadata_sets[index].ways[way].learn_mode = true;
        this->metadata_sets[index].ways[way].is_dead_read = false;
        this->metadata_sets[index].ways[way].is_dead_writeback = false;

        this->metadata_sets[index].ways[way].access_counter_read = 0;
        this->metadata_sets[index].ways[way].access_counter_writeback = 0;

        this->metadata_sets[index].ways[way].overflow_read = true;
        this->metadata_sets[index].ways[way].overflow_writeback = true;


        ///=================================================================
        /// AHT HIT
        ///=================================================================
        if (this->metadata_sets[index].ways[way].aht_pointer != NULL &&
        this->metadata_sets[index].ways[way].aht_pointer->pointer == true) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht HIT\n")
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].aht_pointer->content_to_string().c_str())
            // Statistics
            this->add_stat_aht_access();
            this->add_stat_aht_hit();

            // Update the aht entry
            this->metadata_sets[index].ways[way].aht_pointer->last_access = sinuca_engine.get_global_cycle();

            if (this->metadata_sets[index].ways[way].real_access_counter_writeback <= this->access_counter_max_writeback) {
                this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback = this->metadata_sets[index].ways[way].real_access_counter_writeback;
                this->metadata_sets[index].ways[way].aht_pointer->overflow_writeback = false;
            }
            else {
                this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback = this->access_counter_max_writeback;
                this->metadata_sets[index].ways[way].aht_pointer->overflow_writeback = true;
            }
        }
    }


    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Hit %s", this->metadata_sets[index].ways[way].content_to_string().c_str());
};

/// ============================================================================
void line_usage_predictor_dewp_t::line_eviction(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_eviction()\n")
    ERROR_ASSERT_PRINTF(index < this->metadata_total_sets, "Wrong index %d > total_sets %d", index, this->metadata_total_sets);
    ERROR_ASSERT_PRINTF(way < this->metadata_associativity, "Wrong way %d > associativity %d", way, this->metadata_associativity);
    (void)cache;
    (void)cache_line;
    /// Mechanism statistics
    this->add_stat_eviction();

    // Reads before eviction
    if (this->metadata_sets[index].ways[way].real_access_counter_read == 0) {
        this->add_stat_line_read_0();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read == 1) {
        this->add_stat_line_read_1();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 2 && this->metadata_sets[index].ways[way].real_access_counter_read <= 3) {
        this->add_stat_line_read_2_3();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 4 && this->metadata_sets[index].ways[way].real_access_counter_read <= 7) {
        this->add_stat_line_read_4_7();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 8 && this->metadata_sets[index].ways[way].real_access_counter_read <= 15) {
        this->add_stat_line_read_8_15();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read >= 16 && this->metadata_sets[index].ways[way].real_access_counter_read <= 127) {
        this->add_stat_line_read_16_127();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_read >=128) {
        this->add_stat_line_read_128_bigger();
    }

    // Reads before eviction
    if (this->metadata_sets[index].ways[way].real_access_counter_writeback == 0) {
        this->add_stat_line_writeback_0();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback == 1) {
        this->add_stat_line_writeback_1();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 2 && this->metadata_sets[index].ways[way].real_access_counter_writeback <= 3) {
        this->add_stat_line_writeback_2_3();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 4 && this->metadata_sets[index].ways[way].real_access_counter_writeback <= 7) {
        this->add_stat_line_writeback_4_7();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 8 && this->metadata_sets[index].ways[way].real_access_counter_writeback <= 15) {
        this->add_stat_line_writeback_8_15();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >= 16 && this->metadata_sets[index].ways[way].real_access_counter_writeback <= 127) {
        this->add_stat_line_writeback_16_127();
    }
    else if (this->metadata_sets[index].ways[way].real_access_counter_writeback >=128) {
        this->add_stat_line_writeback_128_bigger();
    }



    //==================================================================
    // Prediction Accuracy Statistics
    if (this->metadata_sets[index].ways[way].is_dirty == true) {
        switch (this->metadata_sets[index].ways[way].line_status) {
            case LINE_SUB_BLOCK_LEARN:
                this->add_stat_dead_writeback_learn();
            break;

            case LINE_SUB_BLOCK_NORMAL:
                if (this->metadata_sets[index].ways[way].overflow_writeback) {
                    this->add_stat_dead_writeback_notsent_correct();
                }
                else {
                    this->add_stat_dead_writeback_notsent_over();
                }
            break;

            case LINE_SUB_BLOCK_DISABLE:
            case LINE_SUB_BLOCK_WRITEBACK:
                this->add_stat_dead_writeback_sent_correct();
            break;

            case LINE_SUB_BLOCK_WRONG_FIRST:
                this->add_stat_dead_writeback_sent_under();
            break;

        }
    }
    else {
        switch (this->metadata_sets[index].ways[way].line_status) {
            case LINE_SUB_BLOCK_LEARN:
                this->add_stat_dead_read_learn();
            break;

            case LINE_SUB_BLOCK_NORMAL:
                if (this->metadata_sets[index].ways[way].overflow_read) {
                    this->add_stat_dead_read_normal_correct();
                }
                else {
                    this->add_stat_dead_read_normal_over();
                }
            break;

            case LINE_SUB_BLOCK_DISABLE:
                this->add_stat_dead_read_disable_correct();
            break;

            case LINE_SUB_BLOCK_WRONG_FIRST:
                this->add_stat_dead_read_disable_under();
            break;

            case LINE_SUB_BLOCK_WRITEBACK:
                ERROR_PRINTF("Wrong state.\n")
            break;
        }
    }

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }

    // AHT UPDATE
    if (this->metadata_sets[index].ways[way].aht_pointer != NULL &&
    this->metadata_sets[index].ways[way].aht_pointer->pointer == true) {
        // Statistics
        this->add_stat_aht_access();
        this->add_stat_aht_hit();

        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht HIT\n")
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].content_to_string().c_str())
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Before Update %s", this->metadata_sets[index].ways[way].aht_pointer->content_to_string().c_str())
        // Update the aht entry
        this->metadata_sets[index].ways[way].aht_pointer->last_access = sinuca_engine.get_global_cycle();

        // Update the Read
        if (this->metadata_sets[index].ways[way].real_access_counter_read <= this->access_counter_max_read) {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_read = this->metadata_sets[index].ways[way].real_access_counter_read;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_read = false;
        }
        else {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_read = this->access_counter_max_read;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_read = true;
        }

        // Update the Writeback
        if (this->metadata_sets[index].ways[way].real_access_counter_writeback <= this->access_counter_max_writeback) {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback = this->metadata_sets[index].ways[way].real_access_counter_writeback;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_writeback = false;
        }
        else {
            this->metadata_sets[index].ways[way].aht_pointer->access_counter_writeback = this->access_counter_max_writeback;
            this->metadata_sets[index].ways[way].aht_pointer->overflow_writeback = true;
        }

        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t After Update %s", this->metadata_sets[index].ways[way].aht_pointer->content_to_string().c_str())
        this->metadata_sets[index].ways[way].aht_pointer->pointer = false;
        this->metadata_sets[index].ways[way].aht_pointer = NULL;
    }
    else {
        LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht MISS\n")
    }

    this->metadata_sets[index].ways[way].clean();
    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
};

/// ============================================================================
void line_usage_predictor_dewp_t::line_invalidation(cache_memory_t *cache, cache_line_t *cache_line, uint32_t index, uint32_t way) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("line_invalidation()\n")
    (void)cache;
    (void)cache_line;

    /// Statistics
    if (this->metadata_sets[index].ways[way].line_status == LINE_SUB_BLOCK_DISABLE) {
        this->cycles_turned_off += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        if (this->metadata_sets[index].ways[way].clock_first_access == 0){
            cycles_turned_off_since_begin += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
        }
    }
    else {
        this->cycles_turned_on += sinuca_engine.get_global_cycle() - this->metadata_sets[index].ways[way].clock_first_access;
    }


    if (this->metadata_sets[index].ways[way].learn_mode == false) {
        this->metadata_sets[index].ways[way].line_status = LINE_SUB_BLOCK_DISABLE;
    }


    this->metadata_sets[index].ways[way].clock_first_access = sinuca_engine.get_global_cycle();
};


/// ============================================================================
/// aht Miss
/// ============================================================================
aht_line_t* line_usage_predictor_dewp_t::aht_find_line(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("aht_find_line()\n")
    uint32_t aht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t aht_index = opcode_address & this->aht_index_bits_mask;

    ERROR_ASSERT_PRINTF(aht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", aht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(aht_index < this->aht_total_sets, "Wrong index %d > total_sets %d", aht_index, this->aht_total_sets);

    for (uint32_t aht_way = 0; aht_way < this->get_aht_associativity(); aht_way++) {
        if (this->aht_sets[aht_index].ways[aht_way].opcode_address == opcode_address && this->aht_sets[aht_index].ways[aht_way].offset == aht_offset) {
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t Found aht Index %u - Way %u\n", aht_index, aht_way )
            LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht %s \n", this->aht_sets[aht_index].ways[aht_way].content_to_string().c_str())
            return &this->aht_sets[aht_index].ways[aht_way];
        }
    }
    return NULL;
}

/// ============================================================================
aht_line_t* line_usage_predictor_dewp_t::aht_evict_address(uint64_t opcode_address, uint64_t memory_address) {
    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("aht_evict_address()\n")
    uint32_t aht_offset = memory_address & sinuca_engine.get_global_offset_bits_mask();
    uint32_t aht_index = opcode_address & this->aht_index_bits_mask;

    ERROR_ASSERT_PRINTF(aht_offset < sinuca_engine.get_global_line_size(), "Wrong offset %d > line_size %d", aht_offset, sinuca_engine.get_global_line_size());
    ERROR_ASSERT_PRINTF(aht_index < this->aht_total_sets, "Wrong index %d > total_sets %d", aht_index, this->aht_total_sets);

    aht_line_t *choosen_line = NULL;

    switch (this->aht_replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (uint32_t aht_way = 0; aht_way < this->get_aht_associativity(); aht_way++) {
                /// If the line is LRU
                if (this->aht_sets[aht_index].ways[aht_way].last_access <= last_access) {
                    choosen_line = &this->aht_sets[aht_index].ways[aht_way];
                    last_access = this->aht_sets[aht_index].ways[aht_way].last_access;
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            uint32_t aht_way = (rand_r(&seed) % this->get_aht_associativity());
            choosen_line = &this->aht_sets[aht_index].ways[aht_way];
        }
        break;

        case REPLACEMENT_INVALID_OR_LRU:
        case REPLACEMENT_DEAD_OR_LRU:
        case REPLACEMENT_FIFO:
        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: %s not implemented.\n",  get_enum_replacement_char(this->aht_replacement_policy));
        break;
    }

    LINE_USAGE_PREDICTOR_DEBUG_PRINTF("\t aht %s \n", choosen_line->content_to_string().c_str())
    return choosen_line;
};


/// ============================================================================
void line_usage_predictor_dewp_t::print_structures() {
    line_usage_predictor_t::print_structures();
};

/// ============================================================================
void line_usage_predictor_dewp_t::panic() {
    line_usage_predictor_t::panic();

    this->print_structures();
};

/// ============================================================================
void line_usage_predictor_dewp_t::periodic_check(){
    line_usage_predictor_t::periodic_check();

    #ifdef PREFETCHER_DEBUG
        this->print_structures();
    #endif
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void line_usage_predictor_dewp_t::reset_statistics() {
    line_usage_predictor_t::reset_statistics();

    this->stat_line_hit = 0;
    this->stat_line_miss = 0;
    this->stat_sub_block_miss = 0;
    this->stat_send_writeback = 0;
    this->stat_recv_writeback = 0;
    this->stat_eviction = 0;
    this->stat_invalidation = 0;

    this->stat_aht_access = 0;
    this->stat_aht_hit = 0;
    this->stat_aht_miss = 0;

    /// Prediction accuracy
    this->stat_dead_read_learn = 0;
    this->stat_dead_read_normal_over = 0;
    this->stat_dead_read_normal_correct = 0;
    this->stat_dead_read_disable_correct = 0;
    this->stat_dead_read_disable_under = 0;

    this->stat_dead_writeback_learn = 0;
    this->stat_dead_writeback_notsent_over = 0;
    this->stat_dead_writeback_notsent_correct = 0;
    this->stat_dead_writeback_sent_correct = 0;
    this->stat_dead_writeback_sent_under = 0;

    this->stat_line_read_0 = 0;
    this->stat_line_read_1 = 0;
    this->stat_line_read_2_3 = 0;
    this->stat_line_read_4_7 = 0;
    this->stat_line_read_8_15 = 0;
    this->stat_line_read_16_127 = 0;
    this->stat_line_read_128_bigger = 0;

    this->stat_line_writeback_0 = 0;
    this->stat_line_writeback_1 = 0;
    this->stat_line_writeback_2_3 = 0;
    this->stat_line_writeback_4_7 = 0;
    this->stat_line_writeback_8_15 = 0;
    this->stat_line_writeback_16_127 = 0;
    this->stat_line_writeback_128_bigger = 0;

    this->cycles_turned_on = 0;
    this->cycles_turned_off = 0;
    this->cycles_turned_off_since_begin = 0;
};

/// ============================================================================
void line_usage_predictor_dewp_t::print_statistics() {
    line_usage_predictor_t::print_statistics();

    for (uint32_t index = 0; index < this->get_metadata_total_sets(); index++) {
        for (uint32_t way = 0; way < this->get_metadata_associativity(); way++) {
            this->line_eviction(NULL, NULL, index, way);
        }
    }

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_hit", stat_line_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_miss", stat_line_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sub_block_miss", stat_sub_block_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_send_writeback", stat_send_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_recv_writeback", stat_recv_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_aht_access", stat_aht_access);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_aht_hit", stat_aht_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_aht_miss", stat_aht_miss);

    /// prediction accuracy
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_learn", stat_dead_read_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_over", stat_dead_read_normal_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_normal_correct", stat_dead_read_normal_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_correct", stat_dead_read_disable_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_read_disable_under", stat_dead_read_disable_under);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_learn", stat_dead_writeback_learn);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_notsent_over", stat_dead_writeback_notsent_over);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_notsent_correct", stat_dead_writeback_notsent_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_sent_correct", stat_dead_writeback_sent_correct);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_dead_writeback_sent_under", stat_dead_writeback_sent_under);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_0", stat_line_read_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_1", stat_line_read_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_2_3", stat_line_read_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_4_7", stat_line_read_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_8_15", stat_line_read_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_16_127", stat_line_read_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_read_128_bigger", stat_line_read_128_bigger);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_0", stat_line_writeback_0);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_1", stat_line_writeback_1);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_2_3", stat_line_writeback_2_3);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_4_7", stat_line_writeback_4_7);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_8_15", stat_line_writeback_8_15);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_16_127", stat_line_writeback_16_127);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_line_writeback_128_bigger", stat_line_writeback_128_bigger);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_on", cycles_turned_on);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off", cycles_turned_off);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_on_per_lines", cycles_turned_on/ metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_per_lines", cycles_turned_off/ metadata_line_number);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_since_begin", cycles_turned_off_since_begin);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cycles_turned_off_since_begin_per_lines", cycles_turned_off_since_begin/ metadata_line_number);

    if ((this->cycles_turned_on + this->cycles_turned_off) / metadata_line_number != sinuca_engine.get_global_cycle()) {
        WARNING_PRINTF("Total of cycles (alive + dead) does not match with global cycle.\n")
    }

};

/// ============================================================================
void line_usage_predictor_dewp_t::print_configuration() {
    line_usage_predictor_t::print_configuration();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_bits_read", access_counter_bits_read);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_bits_writeback", access_counter_bits_writeback);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_max_read", access_counter_max_read);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "access_counter_max_writeback", access_counter_max_writeback);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_eviction", early_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "early_writeback", early_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "turnoff_dead_lines", turnoff_dead_lines);

    /// metadata
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_line_number", metadata_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_associativity", metadata_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "metadata_total_sets", metadata_total_sets);

    /// aht
    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "aht_line_number", aht_line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "aht_associativity", aht_associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "aht_total_sets", aht_total_sets);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "aht_replacement_policy", get_enum_replacement_char(aht_replacement_policy));
};
