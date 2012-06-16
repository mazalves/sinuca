//==============================================================================
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
//==============================================================================
#include "../sinuca.hpp"

#ifdef MAIN_MEMORY_DEBUG
    #define MAIN_MEMORY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define MAIN_MEMORY_DEBUG_PRINTF(...)
#endif

//==============================================================================
main_memory_t::main_memory_t() {
    this->set_type_component(COMPONENT_MAIN_MEMORY);

    this->channel_number = 0;
    this->total_channels = 0;
    this->banks_per_channel = 0;

    this->line_size = 0;
    this->data_bus_latency = 0;

    this->read_buffer_size = 0;
    this->write_buffer_size = 0;

    this->bank_penalty_ras = 0;
    this->bank_penalty_cas = 0;

    this->row_buffer_size = 0;
    this->drain_write = false;

    this->data_bus_ready = 0;
    this->read_ready = 0;
    this->write_ready = 0;
};

//==============================================================================
main_memory_t::~main_memory_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<memory_package_t>(row_buffer);
    utils_t::template_delete_matrix<memory_package_t>(read_buffer, this->get_banks_per_channel());
    utils_t::template_delete_matrix<memory_package_t>(write_buffer, this->get_banks_per_channel());
    utils_t::template_delete_array<uint32_t>(read_buffer_position_used);
    utils_t::template_delete_array<uint32_t>(write_buffer_position_used);
};

//==============================================================================
void main_memory_t::allocate() {

    this->row_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_banks_per_channel());
    /// Initialize with the row 0 opened. (Anyway a new row need to be opened, but save one IF on the clock)
    for (uint32_t i = 0; i < this->get_banks_per_channel(); i++) {
        this->row_buffer[i].package_ready(0);
    }
    this->read_buffer = utils_t::template_allocate_matrix<memory_package_t>(this->get_banks_per_channel(), this->get_read_buffer_size());
    this->write_buffer = utils_t::template_allocate_matrix<memory_package_t>(this->get_banks_per_channel(), this->get_write_buffer_size());

    this->read_buffer_position_used = utils_t::template_allocate_initialize_array<uint32_t>(this->get_banks_per_channel(), 0);
    this->write_buffer_position_used = utils_t::template_allocate_initialize_array<uint32_t>(this->get_banks_per_channel(), 0);


    this->set_masks();

    #ifdef MAIN_MEMORY_DEBUG
        this->print_configuration();
    #endif
};

//==============================================================================
void main_memory_t::set_masks() {
    uint64_t i;

    ERROR_ASSERT_PRINTF(this->get_total_channels() > this->get_channel_number(), "Wrong number of channels (%u/%u).\n", this->get_channel_number(), this->get_total_channels());
    ERROR_ASSERT_PRINTF(this->get_banks_per_channel() > 0, "Wrong number of banks (%u).\n", this->get_banks_per_channel());
    this->column_bits_mask = 0;
    this->channel_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->row_bits_mask = 0;

    switch (this->get_address_mask_type()) {
        case MAIN_MEMORY_MASK_ROW_BANK_CHANNEL_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_channels() > 1 && utils_t::check_if_power_of_two(this->get_total_channels()), "Wrong number of channels (%u).\n", this->get_total_channels());

            this->column_bits_shift = 0;
            this->channel_bits_shift = utils_t::get_power_of_two(this->get_row_buffer_size());
            this->bank_bits_shift = channel_bits_shift + utils_t::get_power_of_two(this->get_total_channels());
            this->row_bits_shift = bank_bits_shift + utils_t::get_power_of_two(this->get_banks_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// CHANNEL MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_total_channels()); i++) {
                this->channel_bits_mask |= 1 << (i + channel_bits_shift);
            }

            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_banks_per_channel()); i++) {
                this->bank_bits_mask |= 1 << (i + bank_bits_shift);
            }

            /// ROW MASK
            for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->row_bits_mask |= 1 << i;
            }
        break;

        case MAIN_MEMORY_MASK_ROW_BANK_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_channels() > 1 || this->get_channel_number() == 0, "Wrong number of channels (%u).\n", this->get_total_channels());

            this->column_bits_shift = 0;
            this->channel_bits_shift = 0;
            this->bank_bits_shift = utils_t::get_power_of_two(this->get_row_buffer_size());
            this->row_bits_shift = bank_bits_shift + utils_t::get_power_of_two(this->get_banks_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_banks_per_channel()); i++) {
                this->bank_bits_mask |= 1 << (i + bank_bits_shift);
            }

            /// ROW MASK
            for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->row_bits_mask |= 1 << i;
            }
        break;
    }
};


//==============================================================================
// CAS = Use the open row
// RAS = Open a new row
void main_memory_t::find_cas_and_ras(memory_package_t **input_buffer, uint32_t input_buffer_size, uint32_t bank, int32_t& cas_position, int32_t& ras_position) {
    cas_position = POSITION_FAIL;
    ras_position = POSITION_FAIL;

    switch (this->row_buffer_policy) {
        /// ====================================================================
        /// ROW_BUFFER_HITS_FIRST
        /// Return the older CAS and older RAS
        case ROW_BUFFER_HITS_FIRST: {
            uint64_t cas_older_cycle = sinuca_engine.get_global_cycle() + 1;
            uint64_t ras_older_cycle = sinuca_engine.get_global_cycle() + 1;

            for (uint32_t i = 0; i < input_buffer_size; i++) {
                if (input_buffer[bank][i].state == PACKAGE_STATE_UNTREATED &&
                input_buffer[bank][i].ready_cycle <= sinuca_engine.get_global_cycle()) {
                    /// Found a CAS
                    if (this->cmp_row_bank_channel(input_buffer[bank][i].memory_address, this->row_buffer[bank].memory_address)) {
                        /// Get Older
                        if (input_buffer[bank][i].born_cycle <= cas_older_cycle) {
                            cas_position = i;
                            cas_older_cycle = input_buffer[bank][i].born_cycle;
                        }
                    }
                    /// Found a RAS
                    else {
                        /// Get Older
                        if (input_buffer[bank][i].born_cycle <= ras_older_cycle) {
                            ras_position = i;
                            ras_older_cycle = input_buffer[bank][i].born_cycle;
                        }
                    }
                }
            }
        }
        break;

        /// ====================================================================
        /// ROW_BUFFER_FIFO
        /// Return the oldest request
        case ROW_BUFFER_FIFO: {
            uint64_t older_cycle = sinuca_engine.get_global_cycle() + 1;
            int32_t older_position = POSITION_FAIL;

            for (uint32_t i = 0; i < input_buffer_size; i++) {
                if (input_buffer[bank][i].state == PACKAGE_STATE_UNTREATED &&
                input_buffer[bank][i].born_cycle <= older_cycle &&
                input_buffer[bank][i].ready_cycle <= sinuca_engine.get_global_cycle()) {
                    ERROR_ASSERT_PRINTF(input_buffer[bank][i].is_answer == false, "This buffer should only have requests on READ buffer\n")
                    older_position = i;
                    older_cycle = input_buffer[bank][i].born_cycle;
                }
            }

            /// No READY request found
            if (older_position == FAIL) {
                return;
            }
            /// Found a CAS
            else if (this->cmp_row_bank_channel(input_buffer[bank][older_position].memory_address, this->row_buffer[bank].memory_address)) {
                cas_position = older_position;
            }
            /// Found a RAS
            else {
                ras_position = older_position;
            }
        }
        break;
    }

    if (cas_position != POSITION_FAIL) {
        ERROR_ASSERT_PRINTF(input_buffer[bank][cas_position].is_answer == false, "This buffer should only have requests on READ/WRITE buffer\n")
    }
    if (ras_position != POSITION_FAIL) {
        ERROR_ASSERT_PRINTF(input_buffer[bank][ras_position].is_answer == false, "This buffer should only have requests on READ/WRITE buffer\n")
    }
};

//==============================================================================
void main_memory_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    MAIN_MEMORY_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    MAIN_MEMORY_DEBUG_PRINTF("====================\n");
    MAIN_MEMORY_DEBUG_PRINTF("cycle() \n");

    /// ========================================================================
    /// Row Buffer States:
    ///     READY       ==> Row opened and ready
    ///     TRANSMIT    ==> Row opened not ready (waiting to transmit)
    /// ========================================================================
    for (uint32_t bank = 0; bank < this->get_banks_per_channel(); bank++) {

        /// If the last operations is over
        if (this->row_buffer[bank].state == PACKAGE_STATE_READY &&
        this->row_buffer[bank].ready_cycle <= sinuca_engine.get_global_cycle()) {
            int32_t read_cas = POSITION_FAIL;
            int32_t read_ras = POSITION_FAIL;
            int32_t write_cas = POSITION_FAIL;
            int32_t write_ras = POSITION_FAIL;

            /// ================================================================
            /// WRITE PRIORITY
            switch (this->write_priority_policy) {
                case WRITE_PRIORITY_DRAIN_WHEN_FULL:
                    /// If WRITE_BUFFER FULL or READ_BUFFER EMPTY => DRAIN WRITE
                    if (this->write_buffer_position_used[bank] == this->write_buffer_size -1 ||
                    this->read_buffer_position_used[bank] == 0 ) {
                        this->drain_write = true;
                    }
                    /// Keep Drain until drain all
                    else if (this->write_buffer_position_used[bank] == 0) {
                        this->drain_write = false;
                    }
                break;

                case WRITE_PRIORITY_SERVICE_AT_NO_READ:
                    /// If WRITE_BUFFER FULL or READ_BUFFER EMPTY => DRAIN WRITE
                    if (this->write_buffer_position_used[bank] == this->write_buffer_size -1 ||
                    this->read_buffer_position_used[bank] == 0 ) {
                        this->drain_write = true;
                    }
                    else {
                        this->drain_write = false;
                    }

                break;
            }

            if (drain_write == true) {
                this->find_cas_and_ras(write_buffer, write_buffer_size, bank, write_cas, write_ras);
            }
            else {
                this->find_cas_and_ras(read_buffer, read_buffer_size, bank, read_cas, read_ras);
            }

            /// ================================================================
            /// THREAT THE CAS or RAS
            /// Have some CAS for the opened row buffer (row buffer hit)
            if (read_cas != POSITION_FAIL || write_cas != POSITION_FAIL) {
                /// BUS && ROW ready for next CAS
                if (this->data_bus_ready <= sinuca_engine.get_global_cycle()) {
                    /// Have some READ CAS
                    if (read_cas != POSITION_FAIL) {
                        MAIN_MEMORY_DEBUG_PRINTF("CAS: READ Bank %u, Position:%d\n", bank, read_cas);
                        this->row_buffer[bank] = this->read_buffer[bank][read_cas];
                        this->row_buffer[bank].born_cycle = this->read_buffer[bank][read_cas].born_cycle;

                        this->row_buffer[bank].is_answer = true;
                        this->row_buffer[bank].memory_size = this->get_line_size();
                        this->row_buffer[bank].id_dst = this->row_buffer[bank].id_src;
                        this->row_buffer[bank].id_src = this->get_id();
                        this->row_buffer[bank].package_transmit(this->get_bank_penalty_cas());

                        this->read_buffer[bank][read_cas].package_clean();
                        this->read_buffer_position_used[bank]--;
                        this->data_bus_ready = this->get_data_bus_latency() + sinuca_engine.get_global_cycle();
                    }
                    /// Have some WRITE CAS
                    else if (write_cas != POSITION_FAIL) {
                        MAIN_MEMORY_DEBUG_PRINTF("CAS: WRITE Bank %u, Position:%d\n", bank, write_cas);
                        this->row_buffer[bank] = this->write_buffer[bank][write_cas];
                        this->row_buffer[bank].born_cycle = this->write_buffer[bank][write_cas].born_cycle;

                        this->row_buffer[bank].is_answer = true;
                        this->row_buffer[bank].memory_size = 1;
                        this->row_buffer[bank].id_dst = this->row_buffer[bank].id_src;
                        this->row_buffer[bank].id_src = this->get_id();
                        this->row_buffer[bank].package_transmit(this->get_bank_penalty_cas());

                        this->write_buffer[bank][write_cas].package_clean();
                        this->write_buffer_position_used[bank]--;
                        this->data_bus_ready = 1 + sinuca_engine.get_global_cycle();
                    }
                }
            }
            /// No CAS available (row buffer miss), Open new Row (RAS)
            else {
                /// New Row can be opened
                /// Have some READ RAS
                if (read_ras != POSITION_FAIL) {
                    MAIN_MEMORY_DEBUG_PRINTF("RAS: READ Bank %u, Position:%d\n", bank, read_ras);
                    this->row_buffer[bank] = this->read_buffer[bank][read_ras];
                    this->row_buffer[bank].package_ready(this->get_bank_penalty_ras());
                    /// Statistics
                    this->add_stat_open_new_row();
                }
                /// Have some WRITE RAS
                else if (write_ras != POSITION_FAIL) {
                    MAIN_MEMORY_DEBUG_PRINTF("RAS: WRITE Bank %u, Position:%d\n", bank, write_ras);
                    this->row_buffer[bank] = this->write_buffer[bank][write_ras];
                    this->row_buffer[bank].package_ready(this->get_bank_penalty_ras());
                    /// Statistics
                    this->add_stat_open_new_row();
                }
            }
        }
    }


    /// ========================================================================
    /// SEND DATA
    int32_t position = memory_package_t::find_old_answer_state_ready(this->row_buffer, this->get_banks_per_channel(), PACKAGE_STATE_TRANSMIT);
    if (position != POSITION_FAIL) {
        MAIN_MEMORY_DEBUG_PRINTF("%s: State TO_HIGHER ROW_BUFFER[%d].\n", this->get_label(), position);
        int32_t transmission_latency = send_package(this->row_buffer + position);
        if  (transmission_latency != POSITION_FAIL) {
            this->row_buffer[position].package_ready(transmission_latency);
            this->add_stat_accesses();

            /// Statistics
            switch (this->row_buffer[position].memory_operation) {
                case MEMORY_OPERATION_READ:
                    this->add_stat_read_completed(this->row_buffer[position].born_cycle);
                break;

                case MEMORY_OPERATION_INST:
                    this->add_stat_instruction_completed(this->row_buffer[position].born_cycle);
                break;

                case MEMORY_OPERATION_WRITE:
                    this->add_stat_write_completed(this->row_buffer[position].born_cycle);
                break;

                case MEMORY_OPERATION_PREFETCH:
                    this->add_stat_prefetch_completed(this->row_buffer[position].born_cycle);
                break;

                case MEMORY_OPERATION_COPYBACK:
                    this->add_stat_copyback_completed(this->row_buffer[position].born_cycle);
                break;
            }
        }
    }
};


//==============================================================================
int32_t main_memory_t::send_package(memory_package_t *package) {
    MAIN_MEMORY_DEBUG_PRINTF("send_package() package:%s\n", package->memory_to_string().c_str());
    sinuca_engine.interconnection_controller->find_package_route(package);

    ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
    uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
    ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
    package->hop_count--;  /// Consume its own port

    bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port));
    if (sent) {
        MAIN_MEMORY_DEBUG_PRINTF("\tSEND OK\n");
        uint32_t latency = sinuca_engine.interconnection_controller->find_package_route_latency(package);
        return latency;
    }
    else {
        MAIN_MEMORY_DEBUG_PRINTF("\tSEND FAIL\n");
        package->hop_count++;  /// Do not Consume its own port
        return POSITION_FAIL;
    }
};

//==============================================================================
bool main_memory_t::receive_package(memory_package_t *package, uint32_t input_port) {
    ERROR_ASSERT_PRINTF(package->id_dst == this->get_id(), "Received some package for a different id_dst.\n");
    ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");
    ERROR_ASSERT_PRINTF(get_channel(package->memory_address) == this->get_channel_number(), "Wrong channel.\n")
    ERROR_ASSERT_PRINTF(package->is_answer == false, "Only requests are expected.\n");

    int32_t slot = POSITION_FAIL;
    uint32_t bank = 0;

    bank = get_bank(package->memory_address);

    switch (package->memory_operation) {
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_PREFETCH:
            slot = memory_package_t::find_free(this->read_buffer[bank], this->read_buffer_size);
            if (slot == -1 || this->read_ready > sinuca_engine.get_global_cycle()) {
                add_stat_full_read_buffer();
                return FAIL;
            }
            this->read_buffer_position_used[bank]++;
            this->read_buffer[bank][slot] = *package;
            this->read_buffer[bank][slot].state = PACKAGE_STATE_UNTREATED;
            this->read_buffer[bank][slot].born_cycle = sinuca_engine.get_global_cycle();
            MAIN_MEMORY_DEBUG_PRINTF("RECEIVED READ/INST from Port[%d]\n", input_port);
            MAIN_MEMORY_DEBUG_PRINTF("RECEIVED OK: %s\n", package->memory_to_string().c_str());

            this->read_ready = 1 + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
            return OK;
        break;

        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_COPYBACK:
            slot = memory_package_t::find_free(this->write_buffer[bank], this->write_buffer_size);
            if (slot == -1 || this->write_ready > sinuca_engine.get_global_cycle()) {
                add_stat_full_write_buffer();
                return FAIL;
            }
            this->write_buffer_position_used[bank]++;
            this->write_buffer[bank][slot] = *package;
            this->write_buffer[bank][slot].state = PACKAGE_STATE_UNTREATED;
            this->write_buffer[bank][slot].born_cycle = sinuca_engine.get_global_cycle();
            MAIN_MEMORY_DEBUG_PRINTF("RECEIVED WRITE from Port[%d]\n", input_port);
            MAIN_MEMORY_DEBUG_PRINTF("RECEIVED OK: %s\n", package->memory_to_string().c_str());

            this->write_ready = 1 + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
            return OK;
        break;
    }
    ERROR_PRINTF("Memory receiving %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};

//==============================================================================
void main_memory_t::print_structures() {
    SINUCA_PRINTF("%s READ_BUFFER_POSITION_USED:", this->get_label())
    for (uint32_t bank = 0; bank < this->banks_per_channel ; bank++){
        SINUCA_PRINTF("%d ", this->read_buffer_position_used[bank])
    }
    SINUCA_PRINTF("\n")

    SINUCA_PRINTF("%s WRITE_BUFFER_POSITION_USED:", this->get_label())
    for (uint32_t bank = 0; bank < this->banks_per_channel ; bank++){
        SINUCA_PRINTF("%d ", this->write_buffer_position_used[bank])
    }
    SINUCA_PRINTF("\n")

    SINUCA_PRINTF("%s READ_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->read_buffer, this->get_banks_per_channel(), this->read_buffer_size).c_str())
    SINUCA_PRINTF("%s WRITE_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->write_buffer, this->get_banks_per_channel(), this->write_buffer_size).c_str())
    SINUCA_PRINTF("%s ROW_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->row_buffer, this->get_banks_per_channel()).c_str())
};

// =============================================================================
void main_memory_t::panic() {
    this->print_structures();
};

//==============================================================================
void main_memory_t::periodic_check(){
    #ifdef MAIN_MEMORY_DEBUG
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->read_buffer, this->get_banks_per_channel(), this->read_buffer_size) == OK, "Check_age failed.\n");
    ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->write_buffer, this->get_banks_per_channel(), this->write_buffer_size) == OK, "Check_age failed.\n");
};

//==============================================================================
// STATISTICS
//==============================================================================
void main_memory_t::reset_statistics() {
    this->stat_open_new_row = 0;

    this->stat_full_read_buffer = 0;
    this->stat_full_write_buffer = 0;

    this->set_stat_instruction_completed(0);
    this->set_stat_read_completed(0);
    this->set_stat_prefetch_completed(0);
    this->set_stat_write_completed(0);
    this->set_stat_copyback_completed(0);

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
};

//==============================================================================
void main_memory_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_accesses", stat_accesses);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_open_new_row", stat_open_new_row);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_row_buffer_hit", stat_accesses - stat_open_new_row);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_read_buffer", stat_full_read_buffer);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_write_buffer", stat_full_write_buffer);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_completed", stat_instruction_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_completed", stat_read_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_completed", stat_prefetch_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_completed", stat_write_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback_completed", stat_copyback_completed);

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
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_instruction_wait_time",stat_acumulated_instruction_wait_time, stat_instruction_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_read_wait_time",stat_acumulated_read_wait_time, stat_read_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_prefetch_wait_time",stat_acumulated_prefetch_wait_time, stat_prefetch_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_write_wait_time",stat_acumulated_write_wait_time, stat_write_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_copyback_wait_time",stat_acumulated_copyback_wait_time, stat_copyback_completed);
};

//==============================================================================
void main_memory_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channel_number", channel_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "total_channels", total_channels);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "banks_per_channel", banks_per_channel);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_size", line_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "data_bus_latency", data_bus_latency);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "read_buffer_size", read_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_buffer_size", write_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_buffer_size", row_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_buffer_policy", get_enum_row_buffer_char(row_buffer_policy));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_priority_policy", get_enum_write_priority_char(write_priority_policy));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_penalty_ras", bank_penalty_ras);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_penalty_cas", bank_penalty_cas);


    sinuca_engine.write_statistics_small_separator();
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "address_mask_type", get_enum_main_memory_mask_char(address_mask_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_bits_mask", utils_t::address_to_binary(this->row_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_bits_mask", utils_t::address_to_binary(this->bank_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channel_bits_mask", utils_t::address_to_binary(this->channel_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "column_bits_mask", utils_t::address_to_binary(this->column_bits_mask).c_str());
};

