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
#include "../sinuca.hpp"

#ifdef MEMORY_CONTROLLER_DEBUG
    #define MEMORY_CONTROLLER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define MEMORY_CONTROLLER_DEBUG_PRINTF(...)
#endif

/// ============================================================================
memory_controller_t::memory_controller_t() {
    this->set_type_component(COMPONENT_MEMORY_CONTROLLER);

    this->line_size = 0;

    this->bus_width = 0;
    this->bus_frequency = 0;
    this->core_to_bus_clock_ratio = 0;

    this->controller_number = 0;
    this->total_controllers = 0;
    this->channels_per_controller = 0;
    this->banks_per_channel = 0;
    this->read_buffer_size = 0;
    this->write_buffer_size = 0;
    this->row_buffer_size = 0;

    this->RP_latency = 0;
    this->RCD_latency = 0;
    this->CAS_latency = 0;
    this->RAS_latency = 0;
};

/// ============================================================================
memory_controller_t::~memory_controller_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<memory_channel_t>(channels);

    utils_t::template_delete_array<uint64_t>(bus_ready_cycle);
    utils_t::template_delete_array<uint64_t>(send_ready_cycle);
    utils_t::template_delete_array<uint64_t>(recv_read_ready_cycle);
    utils_t::template_delete_array<uint64_t>(recv_write_ready_cycle);

    utils_t::template_delete_array<uint32_t>(last_bank_selected);

    utils_t::template_delete_array<memory_package_t>(fill_buffer);
};

/// ============================================================================
void memory_controller_t::allocate() {

    this->bus_latency = (this->line_size / this->get_bus_width()) * (this->get_core_to_bus_clock_ratio() / 2); /// Divide by to to consider a DDR (Double Data Rate)
    this->bus_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_channels_per_controller(), 0);

    this->last_bank_selected = utils_t::template_allocate_initialize_array<uint32_t>(this->get_channels_per_controller(), 0);
    this->last_channel_selected = 0;

    this->send_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_channels_per_controller(), 0);
    this->recv_read_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_channels_per_controller(), 0);
    this->recv_write_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_channels_per_controller(), 0);

    this->channels = utils_t::template_allocate_array<memory_channel_t>(this->get_channels_per_controller());
    this->fill_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_channels_per_controller());

    for (uint32_t i = 0; i < this->get_channels_per_controller(); i++) {
        this->channels[i].row_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_banks_per_channel());
        this->channels[i].cas_ready_cycle = utils_t::template_allocate_initialize_array<uint64_t>(this->get_banks_per_channel(), 0);

        /// Initialize with the row 0 opened. (Anyway a new row need to be opened, but save one IF on the clock)
        for (uint32_t j = 0; j < this->get_banks_per_channel(); j++) {
            this->channels[i].row_buffer[j].package_ready(0);
        }
        this->channels[i].read_buffer = utils_t::template_allocate_matrix<memory_package_t>(this->get_banks_per_channel(), this->get_read_buffer_size());
        this->channels[i].write_buffer = utils_t::template_allocate_matrix<memory_package_t>(this->get_banks_per_channel(), this->get_write_buffer_size());

        this->channels[i].read_buffer_position_used = utils_t::template_allocate_initialize_array<uint32_t>(this->get_banks_per_channel(), 0);
        this->channels[i].write_buffer_position_used = utils_t::template_allocate_initialize_array<uint32_t>(this->get_banks_per_channel(), 0);

        this->channels[i].drain_write = utils_t::template_allocate_initialize_array<bool>(this->get_banks_per_channel(), false);
    }

    this->set_masks();

    #ifdef MEMORY_CONTROLLER_DEBUG
        this->print_configuration();
    #endif
};

/// ============================================================================
void memory_controller_t::set_masks() {
    uint64_t i;

    ERROR_ASSERT_PRINTF(this->get_total_controllers() > this->get_controller_number(), "Wrong number of memory_controllers (%u/%u).\n", this->get_controller_number(), this->get_total_controllers());
    ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 0, "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());
    ERROR_ASSERT_PRINTF(this->get_banks_per_channel() > 0, "Wrong number of memory_banks (%u).\n", this->get_banks_per_channel());
    this->column_bits_mask = 0;
    this->controller_bits_mask = 0;
    this->channel_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->row_bits_mask = 0;

    switch (this->get_address_mask_type()) {
        case MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_CTRL_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_controllers() > 1 && utils_t::check_if_power_of_two(this->get_total_controllers()), "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 1 && utils_t::check_if_power_of_two(this->get_channels_per_controller()), "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->column_bits_shift = 0;
            this->controller_bits_shift = utils_t::get_power_of_two(this->get_row_buffer_size());
            this->channel_bits_shift = this->controller_bits_shift + utils_t::get_power_of_two(this->get_total_controllers());
            this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->get_channels_per_controller());
            this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->get_banks_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// CONTROLLER MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_total_controllers()); i++) {
                this->controller_bits_mask |= 1 << (i + controller_bits_shift);
            }

            /// CHANNEL MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_channels_per_controller()); i++) {
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

        case MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_controllers() == 1, "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 1 && utils_t::check_if_power_of_two(this->get_channels_per_controller()), "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->column_bits_shift = 0;
            this->controller_bits_shift = 0;
            this->channel_bits_shift = utils_t::get_power_of_two(this->get_row_buffer_size());
            this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->get_channels_per_controller());
            this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->get_banks_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// CHANNEL MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_channels_per_controller()); i++) {
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


        case MEMORY_CONTROLLER_MASK_ROW_BANK_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_controllers() == 1, "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() == 1, "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->column_bits_shift = 0;
            this->controller_bits_shift = 0;
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


/// ============================================================================
// CAS = Use the open row
// RAS = Open a new row
void memory_controller_t::find_cas_and_ras(memory_package_t *input_buffer, uint32_t input_buffer_size, memory_package_t *row_buffer, int32_t& cas_position, int32_t& ras_position) {
    cas_position = POSITION_FAIL;
    ras_position = POSITION_FAIL;

    switch (this->request_priority_policy) {
        /// ====================================================================
        /// ROW_BUFFER_HITS_FIRST
        /// Return the older CAS and older RAS
        case REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST: {
            uint64_t cas_older_cycle = sinuca_engine.get_global_cycle() + 1;
            uint64_t ras_older_cycle = sinuca_engine.get_global_cycle() + 1;

            for (uint32_t i = 0; i < input_buffer_size; i++) {
                if (input_buffer[i].state == PACKAGE_STATE_UNTREATED &&
                input_buffer[i].ready_cycle <= sinuca_engine.get_global_cycle()) {
                    /// Found a CAS
                    if (this->cmp_row_bank_channel(input_buffer[i].memory_address, row_buffer->memory_address)) {
                        /// Get Older
                        if (input_buffer[i].born_cycle <= cas_older_cycle) {
                            cas_position = i;
                            cas_older_cycle = input_buffer[i].born_cycle;
                        }
                    }
                    /// Found a RAS
                    else {
                        /// Get Older
                        if (input_buffer[i].born_cycle <= ras_older_cycle) {
                            ras_position = i;
                            ras_older_cycle = input_buffer[i].born_cycle;
                        }
                    }
                }
            }
        }
        break;

        /// ====================================================================
        /// ROW_BUFFER_FIFO
        /// Return the oldest request
        case REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE: {
            uint64_t older_cycle = sinuca_engine.get_global_cycle() + 1;
            int32_t older_position = POSITION_FAIL;

            for (uint32_t i = 0; i < input_buffer_size; i++) {
                if (input_buffer[i].state == PACKAGE_STATE_UNTREATED &&
                input_buffer[i].born_cycle <= older_cycle &&
                input_buffer[i].ready_cycle <= sinuca_engine.get_global_cycle()) {
                    ERROR_ASSERT_PRINTF(input_buffer[i].is_answer == false, "This buffer should only have requests on READ buffer\n")
                    older_position = i;
                    older_cycle = input_buffer[i].born_cycle;
                }
            }

            /// No READY request found
            if (older_position == FAIL) {
                return;
            }
            /// Found a CAS
            else if (this->cmp_row_bank_channel(input_buffer[older_position].memory_address, row_buffer->memory_address)) {
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
        ERROR_ASSERT_PRINTF(input_buffer[cas_position].is_answer == false, "This buffer should only have requests on READ/WRITE buffer\n")
    }
    if (ras_position != POSITION_FAIL) {
        ERROR_ASSERT_PRINTF(input_buffer[ras_position].is_answer == false, "This buffer should only have requests on READ/WRITE buffer\n")
    }
};

/// ============================================================================
void memory_controller_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    MEMORY_CONTROLLER_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    MEMORY_CONTROLLER_DEBUG_PRINTF("====================\n");
    MEMORY_CONTROLLER_DEBUG_PRINTF("cycle() \n");


    for (uint32_t channel = 0; channel < this->get_channels_per_controller(); channel++) {
        uint32_t bank = 0;
        /// Select a port to be activated.
        switch (this->get_bank_selection_policy()) {
            case SELECTION_RANDOM:
                bank = this->selection_bank_random(this->get_banks_per_channel());
            break;

            case SELECTION_ROUND_ROBIN:
                bank = this->selection_bank_round_robin(channel, this->get_banks_per_channel());
            break;

            case SELECTION_BUFFER_LEVEL:
                ERROR_PRINTF("Selection Policy: SELECTION_BUFFER_LEVEL not implemented.\n");
            break;
        }

        bool signal_sent = false;
        /// From the selected channel find a non empty one.
        for (uint32_t i = 0; i < this->get_banks_per_channel(); i++) {
            /// ========================================================================
            /// Row Buffer States:
            ///     READY       ==> Row opened and ready
            ///     TRANSMIT    ==> Row opened not ready (waiting to transmit)
            /// ========================================================================

            /// If the last operations is over
            if (this->channels[channel].row_buffer[bank].state == PACKAGE_STATE_READY &&
            this->channels[channel].row_buffer[bank].ready_cycle <= sinuca_engine.get_global_cycle()) {
                int32_t read_cas = POSITION_FAIL;
                int32_t read_ras = POSITION_FAIL;
                int32_t write_cas = POSITION_FAIL;
                int32_t write_ras = POSITION_FAIL;

                /// ================================================================
                /// WRITE PRIORITY
                switch (this->write_priority_policy) {
                    case WRITE_PRIORITY_DRAIN_WHEN_FULL:
                        /// If WRITE_BUFFER FULL or READ_BUFFER EMPTY => DRAIN WRITE
                        if (this->channels[channel].write_buffer_position_used[bank] == this->write_buffer_size - 1 ||
                        (this->channels[channel].read_buffer_position_used[bank] == 0 && this->channels[channel].write_buffer_position_used[bank] >= this->write_buffer_size / 2)) {
                            this->channels[channel].drain_write[bank] = true;
                        }
                        /// Keep Drain until drain all
                        else if (this->channels[channel].write_buffer_position_used[bank] == 0) {
                            this->channels[channel].drain_write[bank] = false;
                        }
                    break;

                    case WRITE_PRIORITY_SERVICE_AT_NO_READ:
                        /// If WRITE_BUFFER FULL or READ_BUFFER EMPTY => DRAIN WRITE
                        if (this->channels[channel].write_buffer_position_used[bank] == this->write_buffer_size - 1 ||
                        this->channels[channel].read_buffer_position_used[bank] == 0 ) {
                            this->channels[channel].drain_write[bank] = true;
                        }
                        else {
                            this->channels[channel].drain_write[bank] = false;
                        }
                    break;
                }

                if (this->channels[channel].drain_write[bank] == true) {
                    this->find_cas_and_ras(this->channels[channel].write_buffer[bank], this->write_buffer_size, &this->channels[channel].row_buffer[bank], write_cas, write_ras);
                }
                else {
                    this->find_cas_and_ras(this->channels[channel].read_buffer[bank], this->read_buffer_size, &this->channels[channel].row_buffer[bank], read_cas, read_ras);
                }

                /// ================================================================
                /// THREAT THE CAS or RAS
                /// Have some CAS for the opened row buffer (row buffer hit)
                if (read_cas != POSITION_FAIL || write_cas != POSITION_FAIL) {
                    /// BUS && ROW ready for next CAS
                    if (this->bus_ready_cycle[channel] <= sinuca_engine.get_global_cycle()) {
                        /// Have some READ CAS
                        if (read_cas != POSITION_FAIL) {
                            MEMORY_CONTROLLER_DEBUG_PRINTF("CAS: READ Bank:%u, Position:%d\n", bank, read_cas);
                            this->channels[channel].row_buffer[bank] = this->channels[channel].read_buffer[bank][read_cas];
                            this->channels[channel].row_buffer[bank].born_cycle = this->channels[channel].read_buffer[bank][read_cas].born_cycle;

                            this->channels[channel].row_buffer[bank].is_answer = true;
                            this->channels[channel].row_buffer[bank].memory_size = this->get_line_size();
                            this->channels[channel].row_buffer[bank].package_set_src_dst(this->get_id(), this->channels[channel].row_buffer[bank].id_src);
                            this->channels[channel].row_buffer[bank].package_transmit(this->get_CAS_latency());

                            this->channels[channel].read_buffer[bank][read_cas].package_clean();
                            this->channels[channel].read_buffer_position_used[bank]--;
                            this->bus_ready_cycle[channel] = this->bus_latency + sinuca_engine.get_global_cycle();
                            signal_sent = true;

                            /// Statistics
                            this->add_stat_accesses();
                            switch (this->fill_buffer[channel].memory_operation) {
                                case MEMORY_OPERATION_READ:
                                    this->add_stat_read_completed(this->fill_buffer[channel].born_cycle);
                                break;

                                case MEMORY_OPERATION_INST:
                                    this->add_stat_instruction_completed(this->fill_buffer[channel].born_cycle);
                                break;

                                case MEMORY_OPERATION_PREFETCH:
                                    this->add_stat_prefetch_completed(this->fill_buffer[channel].born_cycle);
                                break;

                                case MEMORY_OPERATION_WRITE:
                                case MEMORY_OPERATION_COPYBACK:
                                break;
                            }
                        }
                        /// Have some WRITE CAS
                        else if (write_cas != POSITION_FAIL) {
                            MEMORY_CONTROLLER_DEBUG_PRINTF("CAS: WRITE Bank:%u, Position:%d\n", bank, write_cas);
                            this->channels[channel].row_buffer[bank] = this->channels[channel].write_buffer[bank][write_cas];
                            this->channels[channel].row_buffer[bank].born_cycle = this->channels[channel].write_buffer[bank][write_cas].born_cycle;

                            this->channels[channel].row_buffer[bank].is_answer = true;
                            this->channels[channel].row_buffer[bank].memory_size = 1;
                            this->channels[channel].row_buffer[bank].package_set_src_dst(this->get_id(), this->channels[channel].row_buffer[bank].id_src);
                            /// Never send WRITE ANSWER
                            this->channels[channel].row_buffer[bank].package_ready(this->get_CAS_latency());

                            this->channels[channel].write_buffer[bank][write_cas].package_clean();
                            this->channels[channel].write_buffer_position_used[bank]--;
                            this->bus_ready_cycle[channel] = this->bus_latency + sinuca_engine.get_global_cycle();
                            signal_sent = true;

                            /// Statistics
                            this->add_stat_accesses();
                            switch (this->fill_buffer[channel].memory_operation) {
                                case MEMORY_OPERATION_WRITE:
                                    this->add_stat_write_completed(this->fill_buffer[channel].born_cycle);
                                break;

                                case MEMORY_OPERATION_COPYBACK:
                                    this->add_stat_copyback_completed(this->fill_buffer[channel].born_cycle);
                                break;

                                case MEMORY_OPERATION_READ:
                                case MEMORY_OPERATION_INST:
                                case MEMORY_OPERATION_PREFETCH:
                                break;
                            }
                        }
                    }
                }
                /// No CAS available (row buffer miss), Open new Row (RAS)
                else {
                    /// New Row can be opened
                    if (this->channels[channel].cas_ready_cycle[bank] <= sinuca_engine.get_global_cycle()) {
                        /// Have some READ RAS
                        if (read_ras != POSITION_FAIL) {
                            MEMORY_CONTROLLER_DEBUG_PRINTF("RAS: READ Bank %u, Position:%d\n", bank, read_ras);
                            this->channels[channel].row_buffer[bank] = this->channels[channel].read_buffer[bank][read_ras];
                            this->channels[channel].row_buffer[bank].package_ready(this->get_RP_latency() + this->get_RCD_latency());
                            this->channels[channel].cas_ready_cycle[bank] = this->get_RAS_latency() + sinuca_engine.get_global_cycle();
                            /// Statistics
                            this->add_stat_open_new_row();
                            signal_sent = true;
                        }
                        /// Have some WRITE RAS
                        else if (write_ras != POSITION_FAIL) {
                            MEMORY_CONTROLLER_DEBUG_PRINTF("RAS: WRITE Bank %u, Position:%d\n", bank, write_ras);
                            this->channels[channel].row_buffer[bank] = this->channels[channel].write_buffer[bank][write_ras];
                            this->channels[channel].row_buffer[bank].package_ready(this->get_RP_latency() + this->get_RCD_latency());
                            this->channels[channel].cas_ready_cycle[bank] = this->get_RAS_latency() + sinuca_engine.get_global_cycle();
                            /// Statistics
                            this->add_stat_open_new_row();
                            signal_sent = true;
                        }
                    }
                }
            }

            if (signal_sent == true) {
                break;
            }
            /// If no signal could be sent, choose another bank
            else {
                bank++;
                if (bank >= this->get_banks_per_channel()) {
                    bank = 0;
                }
                this->last_bank_selected[channel] = bank;
            }
        }
    }

    /// ========================================================================
    /// RECEIVE DATA FROM CHANNEL => FILL_BUFFER
    for (uint32_t channel = 0; channel < this->get_channels_per_controller(); channel++) {
        /// Empty FILL_BUFFER position
        if ((this->fill_buffer[channel].state == PACKAGE_STATE_READY || this->fill_buffer[channel].state == PACKAGE_STATE_FREE) &&
        this->fill_buffer[channel].ready_cycle <= sinuca_engine.get_global_cycle()) {
            int32_t position = memory_package_t::find_old_answer_state_ready(this->channels[channel].row_buffer, this->get_banks_per_channel(), PACKAGE_STATE_TRANSMIT);
            if (position != POSITION_FAIL) {
                this->fill_buffer[channel] = this->channels[channel].row_buffer[position];
                this->fill_buffer[channel].package_untreated(this->bus_latency);
                this->channels[channel].row_buffer[position].package_ready(this->bus_latency);
            }
        }
    }

    /// ========================================================================
    /// SEND DATA
    uint32_t channel = 0;
    /// Select a channel to be activated.
    switch (this->get_channel_selection_policy()) {
        case SELECTION_RANDOM:
            channel = this->selection_channel_random(this->get_channels_per_controller());
        break;

        case SELECTION_ROUND_ROBIN:
            channel = this->selection_channel_round_robin(this->get_channels_per_controller());
        break;

        case SELECTION_BUFFER_LEVEL:
            ERROR_PRINTF("Selection Policy: SELECTION_BUFFER_LEVEL not implemented.\n");
        break;
    }

    /// From the selected channel find a non empty one.
    for (uint32_t i = 0; i < this->get_channels_per_controller(); i++) {
        /// If NOT UNTREATED or NOT READY package
        if (this->send_ready_cycle[channel] > sinuca_engine.get_global_cycle() ||
        this->fill_buffer[channel].state != PACKAGE_STATE_UNTREATED ||
        this->fill_buffer[channel].ready_cycle > sinuca_engine.get_global_cycle()) {
            channel++;
            if (channel >= this->get_channels_per_controller()) {
                channel = 0;
            }
            this->last_channel_selected = channel;
        }

        /// Send the oldest UNTREATED package.
        else {
            MEMORY_CONTROLLER_DEBUG_PRINTF("%s: State TO_HIGHER FILL_BUFFER[%d].\n", this->get_label(), channel);
            int32_t transmission_latency = this->send_package(&this->fill_buffer[channel]);
            if (transmission_latency != POSITION_FAIL) {
                this->fill_buffer[channel].package_ready(transmission_latency);
            }
            break;
        }
    }
};

/// ============================================================================
// Selection Strategies
/// ============================================================================
/// Selection strategy: Random
uint32_t memory_controller_t::selection_bank_random(uint32_t total_buffers) {
    unsigned int seed = sinuca_engine.get_global_cycle() % 1000;
    uint32_t selected = (rand_r(&seed) % total_buffers);
    return selected;
};

/// ============================================================================
/// Selection strategy: Round Robin
uint32_t memory_controller_t::selection_bank_round_robin(uint32_t channel, uint32_t total_buffers) {
    this->last_bank_selected[channel]++;
    if (this->last_bank_selected[channel] >= total_buffers) {
        this->last_bank_selected[channel] = 0;
    }
    return this->last_bank_selected[channel];
};

/// ============================================================================
/// Selection strategy: Random
uint32_t memory_controller_t::selection_channel_random(uint32_t total_channels) {
    unsigned int seed = sinuca_engine.get_global_cycle() % 1000;
    uint32_t selected = (rand_r(&seed) % total_channels);
    return selected;
};

/// ============================================================================
/// Selection strategy: Round Robin
uint32_t memory_controller_t::selection_channel_round_robin(uint32_t total_channels) {
    this->last_channel_selected++;
    if (this->last_channel_selected >= total_channels) {
        this->last_channel_selected = 0;
    }
    return this->last_channel_selected;
};

/// ============================================================================
int32_t memory_controller_t::send_package(memory_package_t *package) {
    MEMORY_CONTROLLER_DEBUG_PRINTF("send_package() package:%s\n", package->memory_to_string().c_str());
    ERROR_ASSERT_PRINTF(package->memory_operation != MEMORY_OPERATION_COPYBACK && package->memory_operation != MEMORY_OPERATION_WRITE, "Main memory must never send answer for WRITE.\n");

    uint32_t channel = get_channel(package->memory_address);

    if (this->send_ready_cycle[channel] <= sinuca_engine.get_global_cycle()) {
        sinuca_engine.interconnection_controller->find_package_route(package);
        ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
        uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
        ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
        package->hop_count--;  /// Consume its own port

        uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package, this, this->get_interface_output_component(output_port));
        bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port), transmission_latency);
        if (sent) {
            MEMORY_CONTROLLER_DEBUG_PRINTF("\tSEND OK\n");
            this->send_ready_cycle[channel] = sinuca_engine.get_global_cycle() + transmission_latency;
            return transmission_latency;
        }
        else {
            MEMORY_CONTROLLER_DEBUG_PRINTF("\tSEND FAIL\n");
            package->hop_count++;  /// Do not Consume its own port
            return POSITION_FAIL;
        }
    }
    MEMORY_CONTROLLER_DEBUG_PRINTF("\tSEND FAIL (BUSY)\n");
    return POSITION_FAIL;
};

/// ============================================================================
bool memory_controller_t::receive_package(memory_package_t *package, uint32_t input_port, uint32_t transmission_latency) {
    ERROR_ASSERT_PRINTF(package->id_dst == this->get_id() && package->hop_count == POSITION_FAIL, "Received some package for a different id_dst.\n");
    ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");
    ERROR_ASSERT_PRINTF(get_controller(package->memory_address) == this->get_controller_number(), "Wrong channel.\n")
    ERROR_ASSERT_PRINTF(package->is_answer == false, "Only requests are expected.\n");

    int32_t slot = POSITION_FAIL;
    uint32_t channel = get_channel(package->memory_address);
    uint32_t bank = get_bank(package->memory_address);

    switch (package->memory_operation) {
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_PREFETCH:
            if (this->recv_read_ready_cycle[channel] <= sinuca_engine.get_global_cycle()) {
                slot = memory_package_t::find_free(this->channels[channel].read_buffer[bank], this->read_buffer_size);
                if (slot != POSITION_FAIL) {
                    this->channels[channel].read_buffer_position_used[bank]++;
                    this->channels[channel].read_buffer[bank][slot] = *package;
                    this->channels[channel].read_buffer[bank][slot].package_untreated(transmission_latency);
                    this->recv_read_ready_cycle[channel] = transmission_latency + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                    MEMORY_CONTROLLER_DEBUG_PRINTF("RECEIVED READ/INST from Port[%d]\n", input_port);
                    MEMORY_CONTROLLER_DEBUG_PRINTF("RECEIVED OK: %s\n", package->memory_to_string().c_str());
                    return OK;
                }
                else {
                    add_stat_full_read_buffer();
                    return FAIL;
                }
            }
            MEMORY_CONTROLLER_DEBUG_PRINTF("\tRECV READ/INST FAIL (BUSY)\n");
            return FAIL;
        break;

        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_COPYBACK:
            if (this->recv_write_ready_cycle[channel] <= sinuca_engine.get_global_cycle()) {
                slot = memory_package_t::find_free(this->channels[channel].write_buffer[bank], this->write_buffer_size);
                if (slot != POSITION_FAIL) {
                    this->channels[channel].write_buffer_position_used[bank]++;
                    this->channels[channel].write_buffer[bank][slot] = *package;
                    this->channels[channel].write_buffer[bank][slot].package_untreated(transmission_latency);
                    this->recv_write_ready_cycle[channel] = transmission_latency + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                    MEMORY_CONTROLLER_DEBUG_PRINTF("RECEIVED WRITE/COPYBACK from Port[%d]\n", input_port);
                    MEMORY_CONTROLLER_DEBUG_PRINTF("RECEIVED OK: %s\n", package->memory_to_string().c_str());
                    return OK;
                }
                else {
                    add_stat_full_write_buffer();
                    return FAIL;
                }
            }
            MEMORY_CONTROLLER_DEBUG_PRINTF("\tRECV WRITE/COPYBACK FAIL (BUSY)\n");
            return FAIL;
        break;
    }
    ERROR_PRINTF("Memory receiving %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};

/// ============================================================================
void memory_controller_t::print_structures() {

    for (uint32_t i = 0; i < this->get_channels_per_controller(); i++) {
        SINUCA_PRINTF("%s MEMORY_CONTROLLER CHANNEL[%d]\n", this->get_label(), i)

        SINUCA_PRINTF("\t%s READ_BUFFER_POSITION_USED:", this->get_label())
        for (uint32_t bank = 0; bank < this->banks_per_channel ; bank++){
            SINUCA_PRINTF("%d ", this->channels[i].read_buffer_position_used[bank])
        }
        SINUCA_PRINTF("\n")

        SINUCA_PRINTF("\t%s WRITE_BUFFER_POSITION_USED:", this->get_label())
        for (uint32_t bank = 0; bank < this->banks_per_channel ; bank++){
            SINUCA_PRINTF("%d ", this->channels[i].write_buffer_position_used[bank])
        }
        SINUCA_PRINTF("\n")

        SINUCA_PRINTF("\t%s READ_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->channels[i].read_buffer, this->get_banks_per_channel(), this->read_buffer_size).c_str())
        SINUCA_PRINTF("\t%s WRITE_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->channels[i].write_buffer, this->get_banks_per_channel(), this->write_buffer_size).c_str())
        SINUCA_PRINTF("\t%s ROW_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->channels[i].row_buffer, this->get_banks_per_channel()).c_str())
    }
};

// =============================================================================
void memory_controller_t::panic() {
    this->print_structures();
};

/// ============================================================================
void memory_controller_t::periodic_check(){
    #ifdef MEMORY_CONTROLLER_DEBUG
        this->print_structures();
    #endif

    for (uint32_t i = 0; i < this->get_channels_per_controller(); i++) {
        ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->channels[i].read_buffer, this->get_banks_per_channel(), this->read_buffer_size) == OK, "Check_age failed.\n");
        // ~ ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->channels[i].write_buffer, this->get_banks_per_channel(), this->write_buffer_size) == OK, "Check_age failed.\n");
    }
};

/// ============================================================================
// STATISTICS
/// ============================================================================
void memory_controller_t::reset_statistics() {

    this->stat_accesses = 0;
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

/// ============================================================================
void memory_controller_t::print_statistics() {
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

/// ============================================================================
void memory_controller_t::print_configuration() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_latency", this->get_interconnection_latency());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "interconnection_width", this->get_interconnection_width());

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bus_width", bus_width);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bus_frequency", bus_frequency);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "core_to_bus_clock_ratio", core_to_bus_clock_ratio);


    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_size", line_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "address_mask_type", get_enum_memory_controller_mask_char(address_mask_type));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "controller_number", controller_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "total_controllers", total_controllers);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channels_per_controller", channels_per_controller);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "banks_per_channel", banks_per_channel);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "read_buffer_size", read_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_buffer_size", write_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_buffer_size", row_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_selection_policy", get_enum_selection_char(bank_selection_policy));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channel_selection_policy", get_enum_selection_char(channel_selection_policy));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "request_priority_policy", get_enum_request_priority_char(request_priority_policy));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_priority_policy", get_enum_write_priority_char(write_priority_policy));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "RP_latency", RP_latency);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "RCD_latency", RCD_latency);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "CAS_latency", CAS_latency);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "RAS_latency", RAS_latency);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_bits_mask", utils_t::address_to_binary(this->row_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_bits_mask", utils_t::address_to_binary(this->bank_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channel_bits_mask", utils_t::address_to_binary(this->channel_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "controller_bits_mask", utils_t::address_to_binary(this->controller_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "column_bits_mask", utils_t::address_to_binary(this->column_bits_mask).c_str());
};

