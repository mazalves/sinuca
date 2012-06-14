// =============================================================================
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
// =============================================================================
#include "../sinuca.hpp"

#ifdef CACHE_DEBUG
    #define CACHE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define CACHE_DEBUG_PRINTF(...)
#endif


// =============================================================================
// DSBP
// =============================================================================
void cache_line_t::DSBP_tag_allocate(uint32_t sub_block_number) {
    this->DSBP_tag = new DSBP_metadata;

    this->DSBP_tag->valid_sub_blocks = utils_t::template_allocate_initialize_array<bool>(sub_block_number, false);
    this->DSBP_tag->usage_counter = utils_t::template_allocate_initialize_array<uint32_t>(sub_block_number, 0);
    this->DSBP_tag->overflow = utils_t::template_allocate_initialize_array<bool>(sub_block_number, false);
};

// =============================================================================
void cache_memory_t::DSBP_PHT_allocate(uint32_t sub_block_number) {
    this->DSBP_PHT->pc = 0;
    this->DSBP_PHT->offset = 0;
    this->DSBP_PHT->pointer = false;

    this->DSBP_PHT->usage_counter = utils_t::template_allocate_initialize_array<uint32_t>(sub_block_number, 0);
    this->DSBP_PHT->overflow = utils_t::template_allocate_initialize_array<bool>(sub_block_number, false);
};
// =============================================================================
// =============================================================================


// =============================================================================
cache_memory_t::cache_memory_t() {
    this->set_type_component(COMPONENT_CACHE_MEMORY);
    this->lower_level_cache = new container_ptr_cache_memory_t;
    this->higher_level_cache = new container_ptr_cache_memory_t;
};

// =============================================================================
cache_memory_t::~cache_memory_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<cache_set_t>(sets);
    utils_t::template_delete_array<memory_package_t>(mshr_buffer);
    utils_t::template_delete_variable<container_ptr_cache_memory_t>(lower_level_cache);
    utils_t::template_delete_variable<container_ptr_cache_memory_t>(higher_level_cache);
};

// =============================================================================
void cache_memory_t::allocate() {
    uint32_t i;

    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_line_number() / this->get_associativity()), "Wrong line number(%u) or associativity(%u).\n", this->get_line_number(), this->get_associativity());
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(this->get_line_size()), "Wrong line size.\n");

    this->set_total_sets(this->get_line_number() / this->get_associativity());
    this->sets = utils_t::template_allocate_array<cache_set_t>(this->get_total_sets());
    for (i = 0; i < this->get_total_sets(); i++) {
        this->sets[i].ways = utils_t::template_allocate_array<cache_line_t>(this->get_associativity());
    }

    this->read_ready = sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
    this->write_ready = sinuca_engine.get_global_cycle();  /// Ready to receive from LOWER_PORT

    /// MSHR = [    REQUEST    | COPYBACK | PREFETCH ]
    ERROR_ASSERT_PRINTF(mshr_buffer_request_reserved_size > 0, "mshr_buffer_request_reserved_size should be bigger than one.\n");
    ERROR_ASSERT_PRINTF(mshr_buffer_copyback_reserved_size > mshr_buffer_prefetch_reserved_size, "mshr_buffer_copyback_reserved_size should be bigger than size_prefetch.\n");
    ERROR_ASSERT_PRINTF(mshr_buffer_prefetch_reserved_size > 0, "mshr_buffer_prefetch_reserved_size should be bigger than one.\n");

    this->mshr_buffer_size = this->mshr_buffer_request_reserved_size +
                                this->mshr_buffer_copyback_reserved_size +
                                this->mshr_buffer_prefetch_reserved_size;
    this->mshr_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_mshr_buffer_size());

    /// Check the MSHR sizes (Sum_Higher_MSHR <= This_MSHR)
    /*
    uint32_t MSHR_size_total = 0;
    for (uint32_t i = 0; i < this->higher_level_cache->size(); i++) {
        MSHR_size_total += this->higher_level_cache[0][i]->get_mshr_buffer_request_reserved_size();
        MSHR_size_total += this->higher_level_cache[0][i]->get_mshr_buffer_copyback_reserved_size();
        MSHR_size_total += this->higher_level_cache[0][i]->get_mshr_buffer_prefetch_reserved_size();
    }
    ERROR_ASSERT_PRINTF(MSHR_size_total <= this->get_mshr_buffer_request_reserved_size(),
                        "%s, should have %u MSHR entries to avoid dead_locks.\n", this->get_label(), MSHR_size_total);
    */

    this->set_masks();

    ERROR_ASSERT_PRINTF(this->get_total_banks() == 1 || this->prefetcher.get_prefetcher_type() == PREFETCHER_DISABLE, "Cannot use a multibanked cache with prefetch. (Some requests may be generated in the wrong bank)\n");

    char label[50] = "";
    sprintf(label, "Prefetch_%s", this->get_label());
    this->prefetcher.set_label(label);
    this->prefetcher.allocate();

    #ifdef CACHE_DEBUG
        this->print_configuration();
    #endif
};

// =============================================================================
void cache_memory_t::set_masks() {
    uint64_t i;

    ERROR_ASSERT_PRINTF(this->get_total_banks() > this->get_bank_number(), "Wrong number of banks (%u/%u).\n", this->get_bank_number(), this->get_total_banks());
    this->offset_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->index_bits_mask = 0;
    this->tag_bits_mask = 0;

    switch (this->get_address_mask_type()) {
        case CACHE_MASK_TAG_INDEX_BANK_OFFSET:
            ERROR_ASSERT_PRINTF(this->get_total_banks() > 1 && utils_t::check_if_power_of_two(this->get_total_banks()), "Wrong number of banks (%u).\n", this->get_total_banks());

            this->offset_bits_shift = 0;
            this->bank_bits_shift = utils_t::get_power_of_two(this->get_line_size());
            this->index_bits_shift = bank_bits_shift + utils_t::get_power_of_two(this->get_total_banks());
            this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(this->get_total_sets());

            /// OFFSET MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_line_size()); i++) {
                this->offset_bits_mask |= 1 << i;
            }
            this->not_offset_bits_mask = ~offset_bits_mask;

            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_total_banks()); i++) {
                this->bank_bits_mask |= 1 << (i + bank_bits_shift);
            }

            /// INDEX MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_total_sets()); i++) {
                this->index_bits_mask |= 1 << (i + index_bits_shift);
            }

            /// TAG MASK
            for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->tag_bits_mask |= 1 << i;
            }
        break;

        case CACHE_MASK_TAG_INDEX_OFFSET:
            ERROR_ASSERT_PRINTF(this->get_total_banks() == 1 && this->get_bank_number() == 0, "Wrong number of banks (%u).\n", this->get_total_banks());

            this->offset_bits_shift = 0;
            this->bank_bits_shift = 0;
            this->index_bits_shift = utils_t::get_power_of_two(this->get_line_size());
            this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(this->get_total_sets());

            /// OFFSET MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_line_size()); i++) {
                this->offset_bits_mask |= 1 << i;
            }
            this->not_offset_bits_mask = ~offset_bits_mask;

            /// INDEX MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_total_sets()); i++) {
                this->index_bits_mask |= 1 << (i + index_bits_shift);
            }

            /// TAG MASK
            for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->tag_bits_mask |= 1 << i;
            }
        break;

    }
};

// =============================================================================
void cache_memory_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    CACHE_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    CACHE_DEBUG_PRINTF("====================\n");
    CACHE_DEBUG_PRINTF("cycle() \n");

    this->prefetcher.clock(subcycle);
    int32_t position_ans = POSITION_FAIL;
    int32_t position_rqst = POSITION_FAIL;

    // =================================================================
    // MSHR_BUFFER - TRANSMISSION
    // =================================================================
    memory_package_t::find_old_rqst_ans_state_ready(this->mshr_buffer, this->mshr_buffer_size, PACKAGE_STATE_TRANSMIT, position_rqst, position_ans);
    /// PACKAGE_STATE_TRANSMIT (answer) => send()
    if (position_ans != POSITION_FAIL) {
        CACHE_DEBUG_PRINTF("\t Send ANSWER MSHR[%d] %s\n", position_ans, this->mshr_buffer[position_ans].memory_to_string().c_str());
        int32_t transmission_latency = send_package(&this->mshr_buffer[position_ans]);
        if (transmission_latency != POSITION_FAIL) {
            this->mshr_buffer[position_ans].package_clean();
        }
    }

    /// PACKAGE_STATE_TRANSMIT (request) => send()
    if (position_rqst != POSITION_FAIL) {
        CACHE_DEBUG_PRINTF("\t Send REQUEST MSHR[%d] %s\n", position_rqst, this->mshr_buffer[position_rqst].memory_to_string().c_str());
        int32_t transmission_latency = send_package(&this->mshr_buffer[position_rqst]);
        if (transmission_latency != POSITION_FAIL) {
            this->mshr_buffer[position_rqst].package_wait(transmission_latency);
        }
    }

    // =================================================================
    // MSHR_BUFFER - UNTREATED
    // =================================================================
    memory_package_t::find_old_rqst_ans_state_ready(this->mshr_buffer, this->mshr_buffer_size, PACKAGE_STATE_UNTREATED, position_rqst, position_ans);
    /// PACKAGE_STATE_UNTREATED + ANSWER => treat()
    if (position_ans != POSITION_FAIL) {
        CACHE_DEBUG_PRINTF("\t Treat ANSWER MSHR[%d] %s\n", position_ans, this->mshr_buffer[position_ans].memory_to_string().c_str());
        this->mshr_buffer[position_ans].state = sinuca_engine.directory_controller->treat_cache_answer(this->get_cache_id(), &this->mshr_buffer[position_ans]);
        /// Could not treat, then restart born_cycle (change priority)
        if (this->mshr_buffer[position_ans].state == PACKAGE_STATE_UNTREATED) {
                this->mshr_buffer[position_ans].born_cycle = sinuca_engine.get_global_cycle();
        }
    }

    /// PACKAGE_STATE_UNTREATED + REQUEST => treat()
    if (position_rqst != POSITION_FAIL) {
        CACHE_DEBUG_PRINTF("\t Treat REQUEST MSHR[%d] %s\n", position_rqst, this->mshr_buffer[position_rqst].memory_to_string().c_str());
        this->prefetcher.treat_prefetch(&this->mshr_buffer[position_rqst]);
        this->mshr_buffer[position_rqst].state = sinuca_engine.directory_controller->treat_cache_request(this->get_cache_id(), &this->mshr_buffer[position_rqst]);
        /// Could not treat, then restart born_cycle (change priority)
        if (this->mshr_buffer[position_rqst].state == PACKAGE_STATE_UNTREATED) {
                this->mshr_buffer[position_rqst].born_cycle = sinuca_engine.get_global_cycle();
        }
    }

    // =================================================================
    // PREFETCHER
    // =================================================================
    memory_package_t* memory_package = this->prefetcher.request_buffer_get_older();
    if (memory_package != NULL) {
        bool already_requested = false;
        CACHE_DEBUG_PRINTF("\t Has New Prefetch.\n");

        /// Check if the same address has been already requested
        for (uint32_t i = 0 ; i < this->mshr_buffer_size; i++) {
            if (this->cmp_tag_index_bank(this->mshr_buffer[i].memory_address, memory_package->memory_address)) {
                already_requested = true;
                CACHE_DEBUG_PRINTF("\t\t Dropping PREFETCH\n");
                this->prefetcher.request_buffer_remove();
                break;
            }
        }

        if (already_requested == false) {
            /// Check for free space into MSHR[PREFETCH]
            int32_t position_pfetch = memory_package_t::find_free(this->mshr_buffer + this->mshr_buffer_request_reserved_size + this->mshr_buffer_copyback_reserved_size, this->mshr_buffer_prefetch_reserved_size);
            if (position_pfetch != POSITION_FAIL) {
                position_pfetch += this->mshr_buffer_request_reserved_size + this->mshr_buffer_copyback_reserved_size;
                    CACHE_DEBUG_PRINTF("\t RECEIVED PREFETCH\n");
                    this->mshr_buffer[position_pfetch] = *memory_package;
                    this->mshr_buffer[position_pfetch].id_owner = get_id();
                    this->mshr_buffer[position_pfetch].id_src = get_id();
                    this->mshr_buffer[position_pfetch].id_dst = get_id();
                    this->mshr_buffer[position_pfetch].package_untreated(0);
                }
            else {
                add_stat_full_mshr_buffer_prefetch();
            }
        }
    }
};

// =============================================================================
int32_t cache_memory_t::send_package(memory_package_t *package) {
    CACHE_DEBUG_PRINTF("send_package() package:%s\n", package->memory_to_string().c_str());
    sinuca_engine.interconnection_controller->find_package_route(package);

    ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
    uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
    ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
    package->hop_count--;  /// Consume its own port

    bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port));
    if (sent) {
        CACHE_DEBUG_PRINTF("\tSEND OK\n");
        uint32_t latency = sinuca_engine.interconnection_controller->find_package_route_latency(package);
        return latency;
    }
    else {
        CACHE_DEBUG_PRINTF("\tSEND FAIL\n");
        package->hop_count++;  /// Do not Consume its own port
        return POSITION_FAIL;
    }
};

// =============================================================================
bool cache_memory_t::receive_package(memory_package_t *package, uint32_t input_port) {
    CACHE_DEBUG_PRINTF("receive_package() port:%u, package:%s\n", input_port, package->memory_to_string().c_str());
    ERROR_ASSERT_PRINTF(get_bank(package->memory_address) == this->get_bank_number(), "Wrong bank.\n")
    ERROR_ASSERT_PRINTF(package->id_dst == this->get_id(), "Received some package for a different id_dst.\n");
    ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");

    uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package);
    int32_t slot = POSITION_FAIL;

    /// NEW ANSWER
    if (package->is_answer) {
        for (uint32_t i = 0 ; i < this->mshr_buffer_size; i++) {
            /// Find the correct REQUEST which matches with the ANSWER
            if (this->mshr_buffer[i].state == PACKAGE_STATE_WAIT &&
            this->mshr_buffer[i].id_owner == package->id_owner &&
            this->mshr_buffer[i].opcode_number == package->opcode_number &&
            this->mshr_buffer[i].uop_number == package->uop_number &&
            this->cmp_tag_index_bank(this->mshr_buffer[i].memory_address, package->memory_address)) {
                CACHE_DEBUG_PRINTF("\t RECEIVED ANSWER package WANTED\n");
                this->mshr_buffer[i].is_answer = package->is_answer;
                this->mshr_buffer[i].memory_size = package->memory_size;
                this->mshr_buffer[i].id_src = package->id_src;
                this->mshr_buffer[i].id_dst = package->id_dst;
                this->mshr_buffer[i].package_untreated(transmission_latency);
                return OK;
            }
        }
        ERROR_PRINTF("Receive a NOT WANTED package %s.\n", package->memory_to_string().c_str())
    }
    /// NEW REQUEST
    else {
        switch (package->memory_operation) {
            case MEMORY_OPERATION_READ:
            case MEMORY_OPERATION_INST:
            case MEMORY_OPERATION_PREFETCH:
            {
                if (this->read_ready <= sinuca_engine.get_global_cycle()) {
                    slot = memory_package_t::find_free(this->mshr_buffer, this->mshr_buffer_request_reserved_size);
                    if (slot != POSITION_FAIL) {
                        CACHE_DEBUG_PRINTF("\t RECEIVED READ REQUEST\n");
                        this->mshr_buffer[slot] = *package;
                        this->mshr_buffer[slot].package_untreated(transmission_latency);
                        this->read_ready = 1 + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                        return OK;
                    }
                    else {
                        add_stat_full_mshr_buffer_request();
                        return FAIL;
                    }
                }
            }
            break;

            case MEMORY_OPERATION_WRITE:
            case MEMORY_OPERATION_COPYBACK:
            {
                if (this->write_ready <= sinuca_engine.get_global_cycle()) {
                    slot = memory_package_t::find_free(this->mshr_buffer, this->mshr_buffer_request_reserved_size);
                    if (slot != POSITION_FAIL) {
                        CACHE_DEBUG_PRINTF("\t RECEIVED WRITE REQUEST\n");
                        this->mshr_buffer[slot] = *package;
                        this->mshr_buffer[slot].package_untreated(transmission_latency);
                        this->write_ready = 1 + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                        return OK;
                    }
                    else {
                        add_stat_full_mshr_buffer_request();
                        return FAIL;
                    }
                }
            }
            break;
        }
    }

    return FAIL;
};

// =============================================================================
/// Methods called by the directory to add statistics and others
// =============================================================================
void cache_memory_t::cache_hit(memory_package_t *package) {
    switch (package->memory_operation) {
        case MEMORY_OPERATION_READ:
            this->add_stat_read_hit();
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_INST:
            this->add_stat_instruction_hit();
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_WRITE:
            this->add_stat_write_hit();
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_PREFETCH:
            this->add_stat_prefetch_hit();
        break;

        case MEMORY_OPERATION_COPYBACK:
            this->add_stat_copyback_hit();
        break;
    }
};
// =============================================================================
void cache_memory_t::cache_miss(memory_package_t *package) {
    switch (package->memory_operation) {
        case MEMORY_OPERATION_READ:
            this->add_stat_read_miss(package->born_cycle);
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_INST:
            this->add_stat_instruction_miss(package->born_cycle);
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_WRITE:
            this->add_stat_write_miss(package->born_cycle);
            this->add_stat_accesses();
        break;

        case MEMORY_OPERATION_PREFETCH:
            this->add_stat_prefetch_miss(package->born_cycle);
        break;

        case MEMORY_OPERATION_COPYBACK:
            this->add_stat_copyback_miss(package->born_cycle);
        break;
    }
};
// =============================================================================
void cache_memory_t::cache_invalidate(uint64_t memory_address, bool is_copyback) {
    ERROR_ASSERT_PRINTF(memory_address != 0, "Invalid memory_address.\n")
    if (is_copyback) {
        this->add_stat_invalidation_copyback();
    }
    else {
        this->add_stat_invalidation();
    }
};
// =============================================================================
void cache_memory_t::cache_evict(uint64_t memory_address, bool is_copyback) {
    ERROR_ASSERT_PRINTF(memory_address != 0, "Invalid memory_address.\n")
    if (is_copyback) {
        this->add_stat_eviction_copyback();
    }
    else {
        this->add_stat_eviction();
    }
};

//==============================================================================
void cache_memory_t::print_structures() {
    SINUCA_PRINTF("%s MSHR_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->mshr_buffer, this->mshr_buffer_size).c_str())
};

// =============================================================================
void cache_memory_t::panic() {
    this->print_structures();
    this->prefetcher.panic();
};

//==============================================================================
void cache_memory_t::periodic_check(){
    #ifdef CACHE_DEBUG
        CACHE_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->mshr_buffer, this->mshr_buffer_size) == OK, "Check_age failed.\n");
    this->prefetcher.periodic_check();
};


// =============================================================================
// STATISTICS
// =============================================================================
void cache_memory_t::reset_statistics() {
    this->set_stat_accesses(0);
    this->set_stat_invalidation(0);
    this->set_stat_invalidation_copyback(0);
    this->set_stat_eviction(0);
    this->set_stat_eviction_copyback(0);

    this->set_stat_instruction_hit(0);
    this->set_stat_read_hit(0);
    this->set_stat_prefetch_hit(0);
    this->set_stat_write_hit(0);
    this->set_stat_copyback_hit(0);

    this->set_stat_instruction_miss(0);
    this->set_stat_read_miss(0);
    this->set_stat_prefetch_miss(0);
    this->set_stat_write_miss(0);
    this->set_stat_copyback_miss(0);

    this->stat_min_instruction_wait_time = MAX_ALIVE_TIME;
    this->stat_max_instruction_wait_time = 0;
    this->stat_acumulated_instruction_wait_time = 0;

    this->stat_min_read_wait_time = MAX_ALIVE_TIME;
    this->stat_max_read_wait_time = 0;
    this->stat_acumulated_read_wait_time = 0;

    this->stat_min_prefetch_wait_time = MAX_ALIVE_TIME;
    this->stat_max_prefetch_wait_time = 0;
    this->stat_acumulated_prefetch_wait_time = 0;

    this->stat_min_write_wait_time = MAX_ALIVE_TIME;
    this->stat_max_write_wait_time = 0;
    this->stat_acumulated_write_wait_time = 0;

    this->stat_min_copyback_wait_time = MAX_ALIVE_TIME;
    this->stat_max_copyback_wait_time = 0;
    this->stat_acumulated_copyback_wait_time = 0;

    this->prefetcher.reset_statistics();
};

// =============================================================================
void cache_memory_t::print_statistics() {
    char title[50] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_accesses", stat_accesses);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation", stat_invalidation);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_invalidation_copyback", stat_invalidation_copyback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction", stat_eviction);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_eviction_copyback", stat_eviction_copyback);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_hit", stat_instruction_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_hit", stat_read_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_hit", stat_prefetch_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_hit", stat_write_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback_hit", stat_copyback_hit);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_miss", stat_instruction_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_miss", stat_read_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_miss", stat_prefetch_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_miss", stat_write_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback_miss", stat_copyback_miss);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_instruction_miss_percentage",stat_instruction_miss, stat_instruction_miss + stat_instruction_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_read_miss_percentage",stat_read_miss, stat_read_miss + stat_read_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_prefetch_miss_percentage",stat_prefetch_miss, stat_prefetch_miss + stat_prefetch_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_write_miss_percentage",stat_write_miss, stat_write_miss + stat_write_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_copyback_miss_percentage",stat_copyback_miss, stat_copyback_miss + stat_copyback_hit);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_instruction_wait_time", stat_min_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_read_wait_time", stat_min_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_prefetch_wait_time", stat_min_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_write_wait_time", stat_min_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_copyback_wait_time", stat_min_copyback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_instruction_wait_time", stat_max_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_read_wait_time", stat_max_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_prefetch_wait_time", stat_max_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_write_wait_time", stat_max_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_copyback_wait_time", stat_max_copyback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_instruction_wait_time_ratio",stat_acumulated_instruction_wait_time, stat_instruction_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_read_wait_time_ratio",stat_acumulated_read_wait_time, stat_read_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_prefetch_wait_time_ratio",stat_acumulated_prefetch_wait_time, stat_prefetch_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_write_wait_time_ratio",stat_acumulated_write_wait_time, stat_write_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_copyback_wait_time_ratio",stat_acumulated_copyback_wait_time, stat_copyback_miss);

    this->prefetcher.print_statistics();
};

// =============================================================================
// =============================================================================
void cache_memory_t::print_configuration() {
    char title[50] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "cache_id", cache_id);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_number", bank_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "total_banks", total_banks);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "hierarchy_level", hierarchy_level);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_size", line_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_number", line_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "associativity", associativity);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "replacement_policy", get_enum_replacement_char(replacement_policy));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "penalty_read", penalty_read);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "penalty_write", penalty_write);

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_request_reserved_size", mshr_buffer_request_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_copyback_reserved_size", mshr_buffer_copyback_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_prefetch_reserved_size", mshr_buffer_prefetch_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_size", mshr_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "address_mask_type", get_enum_cache_mask_char(address_mask_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "tag_bits_mask", utils_t::address_to_binary(this->tag_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "index_bits_mask", utils_t::address_to_binary(this->index_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_bits_mask", utils_t::address_to_binary(this->bank_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "offset_bits_mask", utils_t::address_to_binary(this->offset_bits_mask).c_str());

    this->prefetcher.print_configuration();
};

// =============================================================================
cache_line_t* cache_memory_t::evict_address(uint64_t memory_address) {
    uint32_t selected;
    uint64_t index;
    cache_line_t *choosen_line = NULL;

    index = get_index(memory_address);

    switch (this->replacement_policy) {
        case REPLACEMENT_LRU: {
            uint64_t last_access = sinuca_engine.get_global_cycle() + 1;
            for (selected = 0; selected < this->get_associativity(); selected++) {
                if (sinuca_engine.directory_controller->is_locked(this->sets[index].ways[selected].tag) == FAIL) {
                    /// If there is free space && the line is not locked by directory
                    if (this->sets[index].ways[selected].status == PROTOCOL_STATUS_I) {
                        choosen_line = &this->sets[index].ways[selected];
                        break;
                    }
                    /// If the line is LRU && the line is not locked by directory
                    else if (this->sets[index].ways[selected].last_access <= last_access) {
                        choosen_line = &this->sets[index].ways[selected];
                        last_access = this->sets[index].ways[selected].last_access;
                    }
                }
                else {
                    ERROR_ASSERT_PRINTF(cmp_tag_index_bank(memory_address, this->sets[index].ways[selected].tag) == FAIL, "Trying to find one line to evict, but tag == address\n")
                }
            }
        }
        break;

        case REPLACEMENT_RANDOM: {
            /// Initialize random seed
            unsigned int seed = time(NULL);
            /// Generate random number
            selected = (rand_r(&seed) % this->get_associativity());
            /// Check if the line is not locked by directory
            if (sinuca_engine.directory_controller->is_locked(this->sets[index].ways[selected].tag) == FAIL) {
                choosen_line = &this->sets[index].ways[selected];
            }
        }
        break;

        case REPLACEMENT_FIFO:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_FIFO not implemented.\n");
        break;

        case REPLACEMENT_LRF:
            ERROR_PRINTF("Replacement Policy: REPLACEMENT_POLICY_LRF not implemented.\n");
        break;
    }

    return choosen_line;
};

// =============================================================================//
void cache_memory_t::change_address(cache_line_t *line, uint64_t new_memory_address) {
    ERROR_ASSERT_PRINTF(line != NULL, "Can not change the tag address of a NULL line.\n")
    line->tag = new_memory_address;
    return;
};


// =============================================================================//
void cache_memory_t::change_status(cache_line_t *line, protocol_status_t status) {
    ERROR_ASSERT_PRINTF(line != NULL, "Can not change the status of a NULL line.\n")
    line->status = status;
    return;
};

// =============================================================================//
void cache_memory_t::update_last_access(cache_line_t *line) {
    ERROR_ASSERT_PRINTF(line != NULL, "Can not change the last_access of a NULL line.\n")
    line->last_access = sinuca_engine.get_global_cycle();
    line->usage_counter++;
    return;
};

// =============================================================================//
cache_line_t* cache_memory_t::find_line(uint64_t memory_address) {
    uint32_t i;
    uint64_t index;
    cache_line_t *line;

    index = get_index(memory_address);
    line = this->sets[index].ways;

    for (i = 0; i < this->get_associativity(); i++) {
        if (cmp_tag_index_bank(line->tag, memory_address)) {
            return line;
        }
        line++;
    }

    return NULL;
}

