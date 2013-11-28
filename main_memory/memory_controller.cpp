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

    this->address_mask_type = MEMORY_CONTROLLER_MASK_ROW_BANK_COLUMN;
    this->line_size = 0;

    this->controller_number = 0;
    this->total_controllers = 0;
    this->mshr_buffer_size = 0;
    this->channels_per_controller = 0;
    this->bank_per_channel = 0;
    this->bank_buffer_size = 0;
    this->bank_row_buffer_size = 0;
    this->bank_selection_policy = SELECTION_ROUND_ROBIN;

    this->request_priority_policy = REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST;
    this->write_priority_policy = WRITE_PRIORITY_DRAIN_WHEN_FULL;

    this->send_ready_cycle = 0;
    this->recv_ready_cycle = 0;

    this->channels = NULL;
    this->mshr_buffer = NULL;

    this->bus_frequency = 0;
    this->burst_length = 0;
    this->core_to_bus_clock_ratio = 0;

    this->timing_burst = 0;
    this->timing_al = 0;
    this->timing_cas = 0;
    this->timing_ccd = 0;
    this->timing_cwd = 0;
    this->timing_faw = 0;
    this->timing_ras = 0;
    this->timing_rc = 0;
    this->timing_rcd = 0;
    this->timing_rp = 0;
    this->timing_rrd = 0;
    this->timing_rtp = 0;
    this->timing_wr = 0;
    this->timing_wtr = 0;
};

/// ============================================================================
memory_controller_t::~memory_controller_t() {
    // De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<memory_channel_t>(channels);
    utils_t::template_delete_array<memory_package_t>(mshr_buffer);
};

/// ============================================================================
void memory_controller_t::allocate() {

    ERROR_ASSERT_PRINTF(mshr_buffer_request_reserved_size > 0, "mshr_buffer_request_reserved_size should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(mshr_buffer_writeback_reserved_size > 0, "mshr_buffer_writeback_reserved_size should be bigger than zero.\n");
    ERROR_ASSERT_PRINTF(mshr_buffer_prefetch_reserved_size > 0, "mshr_buffer_prefetch_reserved_size should be bigger than zero.\n");


    /// MSHR = [    REQUEST    | WRITEBACK | PREFETCH ]
    this->mshr_buffer_size = this->mshr_buffer_request_reserved_size +
                                this->mshr_buffer_writeback_reserved_size +
                                this->mshr_buffer_prefetch_reserved_size;
    this->mshr_buffer = utils_t::template_allocate_array<memory_package_t>(this->get_mshr_buffer_size());
    this->mshr_born_ordered.reserve(this->mshr_buffer_size);
    this->token_list.reserve(this->mshr_buffer_size);

    this->set_masks();

    this->timing_burst = (this->line_size / this->get_burst_length());

    this->channels = utils_t::template_allocate_array<memory_channel_t>(this->get_channels_per_controller());
    for (uint32_t i = 0; i < this->get_channels_per_controller(); i++) {
        char label[50] = "";
        sprintf(label, "%s_MEMORY_CHANNEL_%d", this->get_label(), i);
        this->channels[i].set_label(label);

        this->channels[i].bank_per_channel = this->bank_per_channel;
        this->channels[i].bank_buffer_size = this->bank_buffer_size;
        this->channels[i].bank_selection_policy = this->bank_selection_policy;

        this->channels[i].request_priority_policy = this->request_priority_policy;
        this->channels[i].write_priority_policy = this->write_priority_policy;

        /// Consider the latency in terms of processor cycles
        this->channels[i].timing_burst = this->timing_burst * this->core_to_bus_clock_ratio;
        this->channels[i].timing_burst += this->timing_burst * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_burst ? 1 : 0;

        this->channels[i].timing_al = this->timing_al * this->core_to_bus_clock_ratio;
        this->channels[i].timing_al += this->timing_al * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_al ? 1 : 0;

        this->channels[i].timing_cas = this->timing_cas * this->core_to_bus_clock_ratio;
        this->channels[i].timing_cas += this->timing_cas * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_cas ? 1 : 0;

        this->channels[i].timing_ccd = this->timing_ccd * this->core_to_bus_clock_ratio;
        this->channels[i].timing_ccd += this->timing_ccd * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_ccd ? 1 : 0;

        this->channels[i].timing_cwd = this->timing_cwd * this->core_to_bus_clock_ratio;
        this->channels[i].timing_cwd += this->timing_cwd * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_cwd ? 1 : 0;

        this->channels[i].timing_faw = this->timing_faw * this->core_to_bus_clock_ratio;
        this->channels[i].timing_faw += this->timing_faw * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_faw ? 1 : 0;

        this->channels[i].timing_ras = this->timing_ras * this->core_to_bus_clock_ratio;
        this->channels[i].timing_ras += this->timing_ras * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_ras ? 1 : 0;

        this->channels[i].timing_rc = this->timing_rc * this->core_to_bus_clock_ratio;
        this->channels[i].timing_rc += this->timing_rc * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_rc ? 1 : 0;

        this->channels[i].timing_rcd = this->timing_rcd * this->core_to_bus_clock_ratio;
        this->channels[i].timing_rcd += this->timing_rcd * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_rcd ? 1 : 0;

        this->channels[i].timing_rp = this->timing_rp * this->core_to_bus_clock_ratio;
        this->channels[i].timing_rp += this->timing_rp * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_rp ? 1 : 0;

        this->channels[i].timing_rrd = this->timing_rrd * this->core_to_bus_clock_ratio;
        this->channels[i].timing_rrd += this->timing_rrd * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_rrd ? 1 : 0;

        this->channels[i].timing_rtp = this->timing_rtp * this->core_to_bus_clock_ratio;
        this->channels[i].timing_rtp += this->timing_rtp * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_rtp ? 1 : 0;

        this->channels[i].timing_wr = this->timing_wr * this->core_to_bus_clock_ratio;
        this->channels[i].timing_wr += this->timing_wr * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_wr ? 1 : 0;

        this->channels[i].timing_wtr = this->timing_wtr * this->core_to_bus_clock_ratio;
        this->channels[i].timing_wtr += this->timing_wtr * this->core_to_bus_clock_ratio > (float)this->channels[i].timing_wtr ? 1 : 0;

        /// Copy the masks
        this->channels[i].column_bits_mask = this->column_bits_mask;
        this->channels[i].not_column_bits_mask = this->not_column_bits_mask;
        this->channels[i].column_bits_shift = this->column_bits_shift;

        this->channels[i].row_bits_mask = this->row_bits_mask;
        this->channels[i].row_bits_shift = this->row_bits_shift;

        this->channels[i].bank_bits_mask = this->bank_bits_mask;
        this->channels[i].bank_bits_shift = this->bank_bits_shift;

        /// Call the channel allocate()
        this->channels[i].allocate();
    }

    #ifdef MEMORY_CONTROLLER_DEBUG
        this->print_configuration();
    #endif
};

/// ============================================================================
void memory_controller_t::set_masks() {
    uint64_t i;

    ERROR_ASSERT_PRINTF(this->get_total_controllers() > this->get_controller_number(),
                        "Wrong number of memory_controllers (%u/%u).\n", this->get_controller_number(), this->get_total_controllers());
    ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 0,
                        "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());
    ERROR_ASSERT_PRINTF(this->get_bank_per_channel() > 0,
                        "Wrong number of memory_banks (%u).\n", this->get_bank_per_channel());
    this->column_bits_mask = 0;
    this->controller_bits_mask = 0;
    this->channel_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->row_bits_mask = 0;

    switch (this->get_address_mask_type()) {
        case MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_CTRL_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_controllers() > 1 &&
                                utils_t::check_if_power_of_two(this->get_total_controllers()),
                                "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 1 &&
                                utils_t::check_if_power_of_two(this->get_channels_per_controller()),
                                "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->column_bits_shift = 0;
            this->controller_bits_shift = utils_t::get_power_of_two(this->get_bank_row_buffer_size());
            this->channel_bits_shift = this->controller_bits_shift + utils_t::get_power_of_two(this->get_total_controllers());
            this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->get_channels_per_controller());
            this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->get_bank_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_row_buffer_size()); i++) {
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
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_per_channel()); i++) {
                this->bank_bits_mask |= 1 << (i + bank_bits_shift);
            }

            /// ROW MASK
            for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->row_bits_mask |= 1 << i;
            }
        break;

        case MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_COLUMN:
            ERROR_ASSERT_PRINTF(this->get_total_controllers() == 1,
                                "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 1 &&
                                utils_t::check_if_power_of_two(this->get_channels_per_controller()),
                                "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->column_bits_shift = 0;
            this->controller_bits_shift = 0;
            this->channel_bits_shift = utils_t::get_power_of_two(this->get_bank_row_buffer_size());
            this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->get_channels_per_controller());
            this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->get_bank_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// CHANNEL MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_channels_per_controller()); i++) {
                this->channel_bits_mask |= 1 << (i + channel_bits_shift);
            }

            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_per_channel()); i++) {
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
            this->bank_bits_shift = utils_t::get_power_of_two(this->get_bank_row_buffer_size());
            this->row_bits_shift = bank_bits_shift + utils_t::get_power_of_two(this->get_bank_per_channel());

            /// COLUMN MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_row_buffer_size()); i++) {
                this->column_bits_mask |= 1 << i;
            }
            this->not_column_bits_mask = ~column_bits_mask;

            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_per_channel()); i++) {
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
void memory_controller_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    MEMORY_CONTROLLER_DEBUG_PRINTF("==================== ID(%u) ",this->get_id());
    MEMORY_CONTROLLER_DEBUG_PRINTF("====================\n");
    MEMORY_CONTROLLER_DEBUG_PRINTF("cycle() \n");

    /// =================================================================
    /// MSHR_BUFFER - REMOVE THE READY PACKAGES
    /// =================================================================
    for (uint32_t i = 0; i < this->mshr_born_ordered.size(); i++){
        if (mshr_born_ordered[i]->state == PACKAGE_STATE_READY &&
        this->mshr_born_ordered[i]->ready_cycle <= sinuca_engine.get_global_cycle()) {
            switch (this->mshr_born_ordered[i]->memory_operation) {
                case MEMORY_OPERATION_READ:
                    this->add_stat_read_completed(this->mshr_born_ordered[i]->born_cycle);
                break;

                case MEMORY_OPERATION_INST:
                    this->add_stat_instruction_completed(this->mshr_born_ordered[i]->born_cycle);
                break;

                case MEMORY_OPERATION_PREFETCH:
                    this->add_stat_prefetch_completed(this->mshr_born_ordered[i]->born_cycle);
                break;

                case MEMORY_OPERATION_WRITEBACK:
                    this->add_stat_writeback_completed(this->mshr_born_ordered[i]->born_cycle);
                break;

                case MEMORY_OPERATION_WRITE:
                    this->add_stat_write_completed(this->mshr_born_ordered[i]->born_cycle);
                break;
            }

            this->add_stat_accesses();
            this->mshr_born_ordered[i]->package_clean();
            this->mshr_born_ordered.erase(this->mshr_born_ordered.begin() + i);
        }
    }

    /// =================================================================
    /// MSHR_BUFFER - TRANSMISSION
    /// =================================================================
    for (uint32_t i = 0; i < this->mshr_born_ordered.size(); i++){
        if (mshr_born_ordered[i]->state == PACKAGE_STATE_TRANSMIT &&
        this->mshr_born_ordered[i]->ready_cycle <= sinuca_engine.get_global_cycle()) {

            ERROR_ASSERT_PRINTF(this->mshr_born_ordered[i]->is_answer == true, "Packages being transmited should be answer.")
            MEMORY_CONTROLLER_DEBUG_PRINTF("\t Send ANSWER this->mshr_born_ordered[%d] %s\n", i, this->mshr_born_ordered[i]->content_to_string().c_str());
            int32_t transmission_latency = send_package(mshr_born_ordered[i]);
            if (transmission_latency != POSITION_FAIL) {
                this->mshr_born_ordered[i]->package_ready(transmission_latency);
            }
            break;
        }
    }


    /// =================================================================
    /// MSHR_BUFFER - UNTREATED
    /// =================================================================
    for (uint32_t i = 0; i < this->mshr_born_ordered.size(); i++){
        if (mshr_born_ordered[i]->state == PACKAGE_STATE_UNTREATED &&
        this->mshr_born_ordered[i]->ready_cycle <= sinuca_engine.get_global_cycle()) {

            ERROR_ASSERT_PRINTF(this->mshr_born_ordered[i]->is_answer == false, "Packages being treated should not be answer.")
            uint32_t channel = this->get_channel(this->mshr_born_ordered[i]->memory_address);

            this->mshr_born_ordered[i]->state = this->channels[channel].treat_memory_request(this->mshr_born_ordered[i]);
            ERROR_ASSERT_PRINTF(this->mshr_born_ordered[i]->state != PACKAGE_STATE_FREE, "Must not receive back a FREE, should receive READY + Latency")
            /// Could not treat, then restart born_cycle (change priority)
            if (this->mshr_born_ordered[i]->state == PACKAGE_STATE_UNTREATED) {
                this->mshr_born_ordered[i]->born_cycle = sinuca_engine.get_global_cycle();

                memory_package_t *package = this->mshr_born_ordered[i];
                this->mshr_born_ordered.erase(this->mshr_born_ordered.begin() + i);
                this->insert_mshr_born_ordered(package);
            }
            break;
        }
    }

    for (uint32_t i = 0; i < this->channels_per_controller; i++) {
        this->channels[i].clock(subcycle);
    }

};

/// ============================================================================
void memory_controller_t::insert_mshr_born_ordered(memory_package_t* package){
    /// this->mshr_born_ordered            = [OLDER --------> NEWER]
    /// this->mshr_born_ordered.born_cycle = [SMALLER -----> BIGGER]

    /// Most of the insertions are made in the end !!!
    for (int32_t i = this->mshr_born_ordered.size() - 1; i >= 0 ; i--){
        if (this->mshr_born_ordered[i]->born_cycle <= package->born_cycle) {
            this->mshr_born_ordered.insert(this->mshr_born_ordered.begin() + i + 1, package);
            return;
        }
    }
    /// Could not find a older package to insert or it is just empty
    this->mshr_born_ordered.insert(this->mshr_born_ordered.begin(), package);

    /// Check the MSHR BORN ORDERED
    #ifdef CACHE_DEBUG
        uint64_t test_order = 0;
        for (uint32_t i = 0; i < this->mshr_born_ordered.size(); i++){
            if (test_order > this->mshr_born_ordered[i]->born_cycle) {
                for (uint32_t j = 0; j < this->mshr_born_ordered.size(); j++){
                    MEMORY_CONTROLLER_DEBUG_PRINTF("%"PRIu64" ", this->mshr_born_ordered[j]->born_cycle);
                }
                ERROR_ASSERT_PRINTF(test_order > this->mshr_born_ordered[i]->born_cycle, "Wrong order when inserting (%"PRIu64")\n", package->born_cycle);
            }
            test_order = this->mshr_born_ordered[i]->born_cycle;
        }
    #endif
};

/// ============================================================================
int32_t memory_controller_t::allocate_request(memory_package_t* package){

    int32_t slot = memory_package_t::find_free(this->mshr_buffer, this->mshr_buffer_request_reserved_size);
    ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Receiving a REQUEST package, but MSHR is full\n")
    if (slot != POSITION_FAIL) {
        MEMORY_CONTROLLER_DEBUG_PRINTF("\t NEW REQUEST\n");
        this->mshr_buffer[slot] = *package;
        this->insert_mshr_born_ordered(&this->mshr_buffer[slot]);    /// Insert into a parallel and well organized MSHR structure
    }
    return slot;
};

/// ============================================================================
int32_t memory_controller_t::allocate_writeback(memory_package_t* package){

    int32_t slot = memory_package_t::find_free(this->mshr_buffer + this->mshr_buffer_request_reserved_size, this->mshr_buffer_writeback_reserved_size);
    ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Receiving a WRITEBACK package, but MSHR is full\n")
    if (slot != POSITION_FAIL) {
        slot += this->mshr_buffer_request_reserved_size;
        MEMORY_CONTROLLER_DEBUG_PRINTF("\t NEW WRITEBACK\n");
        this->mshr_buffer[slot] = *package;
        this->insert_mshr_born_ordered(&this->mshr_buffer[slot]);    /// Insert into a parallel and well organized MSHR structure
    }
    return slot;
};

/// ============================================================================
int32_t memory_controller_t::allocate_prefetch(memory_package_t* package){
    int32_t slot = memory_package_t::find_free(this->mshr_buffer + this->mshr_buffer_request_reserved_size + this->mshr_buffer_writeback_reserved_size,
                                                this->mshr_buffer_prefetch_reserved_size);
    ERROR_ASSERT_PRINTF(slot != POSITION_FAIL, "Receiving a PREFETCH package, but MSHR is full\n")
    if (slot != POSITION_FAIL) {
        slot += this->mshr_buffer_request_reserved_size + this->mshr_buffer_writeback_reserved_size;
        MEMORY_CONTROLLER_DEBUG_PRINTF("\t NEW PREFETCH\n");
        this->mshr_buffer[slot] = *package;
        this->insert_mshr_born_ordered(&this->mshr_buffer[slot]);    /// Insert into a parallel and well organized MSHR structure
    }
    return slot;
};



/// ============================================================================
int32_t memory_controller_t::send_package(memory_package_t *package) {
    MEMORY_CONTROLLER_DEBUG_PRINTF("send_package() package:%s\n", package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(package->memory_address != 0, "Wrong memory address.\n%s\n", package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(package->memory_operation != MEMORY_OPERATION_WRITEBACK && package->memory_operation != MEMORY_OPERATION_WRITE, "Main memory must never send answer for WRITE.\n");

    if (this->send_ready_cycle <= sinuca_engine.get_global_cycle()) {
        sinuca_engine.interconnection_controller->find_package_route(package);
        ERROR_ASSERT_PRINTF(package->hop_count != POSITION_FAIL, "Achieved the end of the route\n");
        uint32_t output_port = package->hops[package->hop_count];  /// Where to send the package ?
        ERROR_ASSERT_PRINTF(output_port < this->get_max_ports(), "Output Port does not exist\n");
        package->hop_count--;  /// Consume its own port

        uint32_t transmission_latency = sinuca_engine.interconnection_controller->find_package_route_latency(package, this, this->get_interface_output_component(output_port));
        bool sent = this->get_interface_output_component(output_port)->receive_package(package, this->get_ports_output_component(output_port), transmission_latency);
        if (sent) {
            MEMORY_CONTROLLER_DEBUG_PRINTF("\tSEND OK\n");
            this->send_ready_cycle = sinuca_engine.get_global_cycle() + transmission_latency;
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
    MEMORY_CONTROLLER_DEBUG_PRINTF("receive_package() port:%u, package:%s\n", input_port, package->content_to_string().c_str());

    ERROR_ASSERT_PRINTF(package->memory_address != 0, "Wrong memory address.\n%s\n", package->content_to_string().c_str());
    ERROR_ASSERT_PRINTF(package->id_dst == this->get_id() && package->hop_count == POSITION_FAIL, "Received some package for a different id_dst.\n");
    ERROR_ASSERT_PRINTF(input_port < this->get_max_ports(), "Received a wrong input_port\n");
    ERROR_ASSERT_PRINTF(package->is_answer == false, "Only requests are expected.\n");

    int32_t slot = POSITION_FAIL;
    if (this->recv_ready_cycle <= sinuca_engine.get_global_cycle()) {
        switch (package->memory_operation) {
            case MEMORY_OPERATION_READ:
            case MEMORY_OPERATION_INST:
                slot = this->allocate_request(package);
                if (slot != POSITION_FAIL) {
                    MEMORY_CONTROLLER_DEBUG_PRINTF("\t RECEIVED READ REQUEST\n");
                    this->mshr_buffer[slot].package_untreated(1);
                    /// Prepare for answer later
                    this->mshr_buffer[slot].package_set_src_dst(this->get_id(), package->id_src);
                    this->recv_ready_cycle = transmission_latency + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                    this->remove_token_list(package);
                    return OK;
                }
                return FAIL;
            break;

            case MEMORY_OPERATION_PREFETCH:
                slot = this->allocate_prefetch(package);
                if (slot != POSITION_FAIL) {
                    MEMORY_CONTROLLER_DEBUG_PRINTF("\t RECEIVED READ REQUEST\n");
                    this->mshr_buffer[slot].package_untreated(1);
                    /// Prepare for answer later
                    this->mshr_buffer[slot].package_set_src_dst(this->get_id(), package->id_src);
                    this->recv_ready_cycle = transmission_latency + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                    this->remove_token_list(package);
                    return OK;
                }
                return FAIL;
            break;

            case MEMORY_OPERATION_WRITEBACK:
            case MEMORY_OPERATION_WRITE:
                slot = this->allocate_writeback(package);
                if (slot != POSITION_FAIL) {
                    MEMORY_CONTROLLER_DEBUG_PRINTF("\t RECEIVED READ REQUEST\n");
                    this->mshr_buffer[slot].package_untreated(1);
                    /// Prepare for answer later
                    this->mshr_buffer[slot].package_set_src_dst(this->get_id(), package->id_src);
                    this->recv_ready_cycle = transmission_latency + sinuca_engine.get_global_cycle();  /// Ready to receive from HIGHER_PORT
                    this->remove_token_list(package);
                    return OK;
                }
                return FAIL;
            break;
        }
    }
    else {
        MEMORY_CONTROLLER_DEBUG_PRINTF("\tRECV READ/INST FAIL (BUSY)\n");
        return FAIL;
    }
    ERROR_PRINTF("Memory receiving %s.\n", get_enum_memory_operation_char(package->memory_operation))
    return FAIL;
};


/// ============================================================================
/// Token Controller Methods
/// ============================================================================
bool memory_controller_t::check_token_list(memory_package_t *package) {
    ERROR_ASSERT_PRINTF(package->is_answer == false, "check_token_list received a Answer.\n")
    uint32_t token = 0;

    /// 1. Check if the name is already in the guest list.
    for (token = 0; token < this->token_list.size(); token++) {
        /// Requested Address Found
        if (this->token_list[token].id_owner == package->id_owner &&
        this->token_list[token].opcode_number == package->opcode_number &&
        this->token_list[token].uop_number == package->uop_number &&
        this->token_list[token].memory_address == package->memory_address &&
        this->token_list[token].memory_operation == package->memory_operation) {
            break;
        }
    }

    /// 2. Name is not in the guest list, lets add it.
    if (token == this->token_list.size()) {
        /// Allocate the new token
        token_t new_token;
        new_token.id_owner = package->id_owner;
        new_token.opcode_number = package->opcode_number;
        new_token.uop_number = package->uop_number;
        new_token.memory_address = package->memory_address;
        new_token.memory_operation = package->memory_operation;

        this->token_list.push_back(new_token);
    }

    /// Attention: Since we classify the incoming requests into request, prefetch and writeback
    /// But we only have one token_list, we are counting how many requests of each type exists
    /// so we know if we can or cannot receive the package.
    uint32_t slot, request_number = 0, writeback_number = 0, prefetch_number = 0;
    for (slot = 0; slot < token; slot++) {
        switch (this->token_list[slot].memory_operation) {
            case MEMORY_OPERATION_READ:
            case MEMORY_OPERATION_INST:
                request_number++;
            break;

            case MEMORY_OPERATION_PREFETCH:
                prefetch_number++;
            break;

            case MEMORY_OPERATION_WRITEBACK:
            case MEMORY_OPERATION_WRITE:
                writeback_number++;
            break;
        }
    }

    /// 3. Check if the guest can come now, Or it needs to wait for free space.
    /// Hold on !
    switch (package->memory_operation) {
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
            if (request_number < memory_package_t::count_free(this->mshr_buffer, this->mshr_buffer_request_reserved_size)) {
                /// Lets party !
                return OK;
            }
            else {
                this->add_stat_full_mshr_buffer_request();
            }
        break;

        case MEMORY_OPERATION_PREFETCH:
            if (prefetch_number < memory_package_t::count_free(this->mshr_buffer + this->mshr_buffer_request_reserved_size + this->mshr_buffer_writeback_reserved_size,
                                                    this->mshr_buffer_prefetch_reserved_size)) {
                /// Lets party !
                return OK;
            }
            else {
                this->add_stat_full_mshr_buffer_prefetch();
            }
        break;

        case MEMORY_OPERATION_WRITEBACK:
        case MEMORY_OPERATION_WRITE:
            if (writeback_number < memory_package_t::count_free(this->mshr_buffer + this->mshr_buffer_request_reserved_size,
                                                    this->mshr_buffer_writeback_reserved_size)) {
                /// Lets party !
                return OK;
            }
            else {
                this->add_stat_full_mshr_buffer_writeback();
            }
        break;
    }

    return FAIL;

};

/// ============================================================================
void memory_controller_t::remove_token_list(memory_package_t *package) {
    for (uint32_t token = 0; token < this->token_list.size(); token++) {
        /// Requested Address Found
        if (this->token_list[token].id_owner == package->id_owner &&
        this->token_list[token].opcode_number == package->opcode_number &&
        this->token_list[token].uop_number == package->uop_number &&
        this->token_list[token].memory_address == package->memory_address &&
        this->token_list[token].memory_operation == package->memory_operation) {
            this->token_list.erase(this->token_list.begin() + token);
            return;
        }
    }
    ERROR_PRINTF("Could not find the previous allocated token.\n%s\n", package->content_to_string().c_str())
};


/// ============================================================================
void memory_controller_t::print_structures() {
    SINUCA_PRINTF("%s MSHR_BUFFER:\n%s", this->get_label(), memory_package_t::print_all(this->mshr_buffer, this->mshr_buffer_size).c_str())

    for (uint32_t i = 0; i < this->channels_per_controller; i++) {
        this->channels[i].print_structures();
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

    switch (this->write_priority_policy) {
        case WRITE_PRIORITY_DRAIN_WHEN_FULL:
        break;

        case WRITE_PRIORITY_SERVICE_AT_NO_READ:
            ERROR_ASSERT_PRINTF(memory_package_t::check_age(this->mshr_buffer, this->mshr_buffer_size) == OK, "Check_age failed.\n");

            for (uint32_t i = 0; i < this->channels_per_controller; i++) {
                this->channels[i].periodic_check();
            }
        break;
    }
};

/// ============================================================================
/// STATISTICS
/// ============================================================================
void memory_controller_t::reset_statistics() {
    this->stat_accesses = 0;

    this->set_stat_instruction_completed(0);
    this->set_stat_read_completed(0);
    this->set_stat_prefetch_completed(0);
    this->set_stat_write_completed(0);
    this->set_stat_writeback_completed(0);

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

    this->stat_min_writeback_wait_time = MAX_ALIVE_TIME;
    this->stat_max_writeback_wait_time = 0;
    this->stat_acumulated_writeback_wait_time = 0;

    this->stat_full_mshr_buffer_request = 0;
    this->stat_full_mshr_buffer_writeback = 0;
    this->stat_full_mshr_buffer_prefetch = 0;

    for (uint32_t i = 0; i < this->channels_per_controller; i++) {
        this->channels[i].reset_statistics();
    }
};

/// ============================================================================
void memory_controller_t::print_statistics() {
    char title[100] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_accesses", stat_accesses);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_completed", stat_instruction_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_completed", stat_read_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_completed", stat_prefetch_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_completed", stat_write_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_writeback_completed", stat_writeback_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sum_read", stat_instruction_completed + stat_read_completed + stat_prefetch_completed);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_sum_write", stat_write_completed + stat_writeback_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_instruction_wait_time", stat_min_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_read_wait_time", stat_min_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_prefetch_wait_time", stat_min_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_write_wait_time", stat_min_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_writeback_wait_time", stat_min_writeback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_instruction_wait_time", stat_max_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_read_wait_time", stat_max_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_prefetch_wait_time", stat_max_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_write_wait_time", stat_max_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_writeback_wait_time", stat_max_writeback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_instruction_wait_time",stat_acumulated_instruction_wait_time, stat_instruction_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_read_wait_time",stat_acumulated_read_wait_time, stat_read_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_prefetch_wait_time",stat_acumulated_prefetch_wait_time, stat_prefetch_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_write_wait_time",stat_acumulated_write_wait_time, stat_write_completed);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_writeback_wait_time",stat_acumulated_writeback_wait_time, stat_writeback_completed);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_mshr_buffer_request", stat_full_mshr_buffer_request);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_mshr_buffer_writeback", stat_full_mshr_buffer_writeback);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_full_mshr_buffer_prefetch", stat_full_mshr_buffer_prefetch);

    for (uint32_t i = 0; i < this->channels_per_controller; i++) {
        this->channels[i].print_statistics();
    }
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
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "line_size", line_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "address_mask_type", get_enum_memory_controller_mask_char(address_mask_type));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "controller_number", controller_number);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "total_controllers", total_controllers);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channels_per_controller", channels_per_controller);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_per_channel", bank_per_channel);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_buffer_size", bank_buffer_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_row_buffer_size", bank_row_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_selection_policy", get_enum_selection_char(bank_selection_policy));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "request_priority_policy", get_enum_request_priority_char(request_priority_policy));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "write_priority_policy", get_enum_write_priority_char(write_priority_policy));

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_request_reserved_size", mshr_buffer_request_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_writeback_reserved_size", mshr_buffer_writeback_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_prefetch_reserved_size", mshr_buffer_prefetch_reserved_size);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mshr_buffer_size", mshr_buffer_size);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bus_frequency", bus_frequency);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "burst_length", burst_length);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "core_to_bus_clock_ratio", core_to_bus_clock_ratio);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_burst", timing_burst);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_al", timing_al);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_cas", timing_cas);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_ccd", timing_ccd);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_cwd", timing_cwd);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_faw", timing_faw);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_ras", timing_ras);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_rc", timing_rc);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_rcd", timing_rcd);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_rp", timing_rp);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_rrd", timing_rrd);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_rtp", timing_rtp);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_wr", timing_wr);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "timing_wtr", timing_wtr);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "row_bits_mask", utils_t::address_to_binary(this->row_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "bank_bits_mask", utils_t::address_to_binary(this->bank_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "channel_bits_mask", utils_t::address_to_binary(this->channel_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "controller_bits_mask", utils_t::address_to_binary(this->controller_bits_mask).c_str());
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "column_bits_mask", utils_t::address_to_binary(this->column_bits_mask).c_str());


    for (uint32_t i = 0; i < this->channels_per_controller; i++) {
        this->channels[i].print_configuration();
    }
};

