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

#include "./sinuca.hpp"

// =============================================================================
sinuca_engine_t::sinuca_engine_t() {
    this->arg_configuration_file_name = NULL;
    this->arg_configuration_path = NULL;
    this->arg_trace_file_name = NULL;
    this->arg_result_file_name = NULL;
    this->arg_warmup_instructions = 0;
    this->arg_stopat_instructions = 0;
    this->arg_graph_file_name = NULL;

    for (uint32_t i = 0; i < MAX_CORES; i++) {
        this->thread_affinity[i] = i;
    }

    this->arg_map_file_name = NULL;

    this->interconnection_interface_array = NULL;
    this->processor_array = NULL;
    this->cache_memory_array = NULL;
    this->memory_controller_array = NULL;
    this->interconnection_router_array = NULL;

    this->round_robin_processor = 0;
    this->round_robin_cache_memory = 0;
    this->round_robin_memory_controller = 0;
    this->round_robin_interconnection_router = 0;

    this->stat_vm_start = 0.0;
    this->stat_rss_start = 0.0;

    this->stat_vm_allocate = 0.0;
    this->stat_rss_allocate = 0.0;

    this->stat_vm_end = 0.0;
    this->stat_rss_end = 0.0;

    this->stat_vm_max = 0.0;
    this->stat_rss_max = 0.0;

    this->stat_old_memory_package = 0;
    this->stat_old_opcode_package = 0;
    this->stat_old_uop_package = 0;

    this->interconnection_interface_array_size = 0;
    this->processor_array_size = 0;
    this->cache_memory_array_size = 0;
    this->memory_controller_array_size = 0;
    this->interconnection_router_array_size = 0;

    this->global_cycle = 0;
    this->global_line_size = 64;

    this->is_simulation_allocated = false;
    this->is_processor_trace_eof = NULL;
    this->is_simulation_eof = false;
    this->is_runtime_debug = true;
    this->is_warmup = false;
    this->is_global_panic = false;

    this->trace_reader = new trace_reader_t;
    this->directory_controller = NULL;
    this->interconnection_controller = NULL;

    utils_t::process_mem_usage(&stat_vm_start, &stat_rss_start);
};

// =============================================================================
sinuca_engine_t::~sinuca_engine_t() {
    // Do not free these argment pointers
    // ~ utils_t::template_delete_variable<char>(this->arg_configuration_file_name);
    // ~ utils_t::template_delete_variable<char>(this->arg_trace_file_name);

    utils_t::template_delete_array<char>(this->arg_configuration_path);

    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<interconnection_interface_t*>(this->interconnection_interface_array);

    if (this->processor_array != NULL) {
        for (uint32_t i = 0; i < this->get_processor_array_size(); ++i) {
            utils_t::template_delete_variable<processor_t>(this->processor_array[i]);
        }
        utils_t::template_delete_array<processor_t*>(this->processor_array);
    }

    if (this->cache_memory_array != NULL) {
        for (uint32_t i = 0; i < this->get_cache_memory_array_size(); ++i) {
            utils_t::template_delete_variable<cache_memory_t>(this->cache_memory_array[i]);
        }
        utils_t::template_delete_array<cache_memory_t*>(this->cache_memory_array);
    }

    if (this->memory_controller_array != NULL) {
        for (uint32_t i = 0; i < this->get_memory_controller_array_size(); ++i) {
            utils_t::template_delete_variable<memory_controller_t>(this->memory_controller_array[i]);
        }
        utils_t::template_delete_array<memory_controller_t*>(this->memory_controller_array);
    }

    if (this->interconnection_router_array != NULL) {
        for (uint32_t i = 0; i < this->get_interconnection_router_array_size(); ++i) {
            utils_t::template_delete_variable<interconnection_router_t>(this->interconnection_router_array[i]);
        }
        utils_t::template_delete_array<interconnection_router_t*>(this->interconnection_router_array);
    }

    utils_t::template_delete_array<bool>(is_processor_trace_eof);

    utils_t::template_delete_variable<trace_reader_t>(trace_reader);
    utils_t::template_delete_variable<directory_controller_t>(directory_controller);
    utils_t::template_delete_variable<interconnection_controller_t>(interconnection_controller);
};




// =============================================================================
void sinuca_engine_t::set_global_line_size(uint32_t new_size) {
    if (this->global_line_size == 0) {
        this->global_line_size = new_size;
        /// OFFSET MASK
        for (uint32_t i = 0; i < utils_t::get_power_of_two(this->global_line_size); i++) {
            this->global_offset_bits_mask |= 1 << i;
        }
        this->global_not_offset_bits_mask = ~this->global_offset_bits_mask;
    }
    ERROR_ASSERT_PRINTF(this->get_global_line_size() == new_size, "All the line_size must be equal.\n")
};

// =============================================================================
uint32_t sinuca_engine_t::get_global_line_size() {
    return (this->global_line_size);
};


// =============================================================================
void sinuca_engine_t::premature_termination() {
    SINUCA_PRINTF("Termination request sent to program.\n");

    this->global_print_configuration();
    if (this->get_is_simulation_allocated()) {
        this->global_print_statistics();
    }
    SINUCA_PRINTF("Abnormal termination!\n");
};

// =============================================================================
bool sinuca_engine_t::alive() {
    /// Wait the all TRACES EOF
    if (!this->is_simulation_eof) {
        return OK;
    }

    /// Wait all processors
    for (uint32_t i = 0 ; i < this->get_processor_array_size() ; i++) {
        if (this->processor_array[i]->is_busy()) {
            return OK;
        }
    }
    /// Wait all directory transactions
    if (this->directory_controller->get_directory_lines_size() != 0) {
        return OK;
    }
    return FAIL;
};

// =============================================================================
void sinuca_engine_t::set_is_processor_trace_eof(uint32_t cpuid) {
    uint32_t i;
    this->is_processor_trace_eof[cpuid] = true;
    for (i = 0 ; i < get_processor_array_size() ; i++) {
        if (this->is_processor_trace_eof[i] == false)
            return;
    }
    this->set_is_simulation_eof(true);
};

// =============================================================================
void sinuca_engine_t::global_panic() {

    if (this->is_global_panic) {
        SINUCA_PRINTF("Abnormal termination!\n");
        SINUCA_PRINTF("Two panics in a row!\n");
        exit(EXIT_FAILURE);
    }
    this->is_global_panic = true;


    #ifndef SINUCA_DEBUG
    if (this->get_is_simulation_allocated()) {
        SINUCA_PRINTF("---------------------------------------------\nPANIC\n\n");
        for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size(); i++) {
            SINUCA_PRINTF("Component:%s ", interconnection_interface_array[i]->get_label());
            SINUCA_PRINTF("ID:%u ", interconnection_interface_array[i]->get_id());
            SINUCA_PRINTF("Type:%s\n", get_enum_component_char(interconnection_interface_array[i]->get_type_component()));
            this->interconnection_interface_array[i]->panic();
            SINUCA_PRINTF("\n\n");
        }
        this->directory_controller->panic();
        this->interconnection_controller->panic();
    }
    #endif
};

// =============================================================================
void sinuca_engine_t::global_periodic_check() {
    if (sinuca_engine.get_is_runtime_debug()) {
        for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size() ; i++) {
            this->interconnection_interface_array[i]->periodic_check();
        }
        this->directory_controller->periodic_check();
        this->interconnection_controller->periodic_check();

        /// Get the max VM (virtual memory) and RSS (resident set size)
        double vm = 0;
        double rss = 0;
        utils_t::process_mem_usage(&vm, &rss);
        if (vm > stat_vm_max) stat_vm_max = vm;
        if (rss > stat_rss_max) stat_rss_max = rss;
    }
};

// =============================================================================
void sinuca_engine_t::global_clock() {
    uint32_t sub_cycle = 0;

    if (INITIALIZE_DEBUG != 0 && INITIALIZE_DEBUG >= this->get_global_cycle()) {
        this->set_is_runtime_debug(false);
    }
    else if (FINALIZE_DEBUG != 0 && FINALIZE_DEBUG < this->get_global_cycle()) {
        this->set_is_runtime_debug(false);
    }
    else {
        this->set_is_runtime_debug(true);
    }

    DEBUG_PRINTF("\n\n\n")
    DEBUG_PRINTF("========================================================================================================\n")
    DEBUG_PRINTF("======================================== Sinuca_Cycle %" PRIu64 "/%u ========================================\n\n\n", this->get_global_cycle(), sub_cycle)

    for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size() ; i++) {
        this->interconnection_interface_array[i]->clock(sub_cycle);
    }
    this->global_cycle++;
};

// =============================================================================
void sinuca_engine_t::global_open_output_files(){

    time_t     now = time(0);
    struct tm  tstruct;
    char       buffer[100];
    tstruct = *localtime(&now);

    strftime(buffer, sizeof(buffer), "# SiNUCA # Y:%Y M:%m D:%d - Time:%X\n", &tstruct);

    /// Open the result file
    if (this->result_file.is_open() == false && this->arg_result_file_name != NULL) {
        /// Open a new file
        this->result_file.open(this->arg_result_file_name, std::ofstream::app);
        /// Check if the file was created
        ERROR_ASSERT_PRINTF(this->result_file.is_open() == true, "Could not open the result file.\n")
    }
    this->result_file.write(buffer, strlen(buffer));
    this->result_file.close();


    /// Open the graph file
    if (this->graph_file.is_open() == false && this->arg_graph_file_name != NULL) {
        /// Open a new file
        this->graph_file.open(this->arg_graph_file_name, std::ofstream::app);
        /// Check if the file was created
        ERROR_ASSERT_PRINTF(this->graph_file.is_open() == true, "Could not open the graph file.\n")
    }
    this->graph_file.write(buffer, strlen(buffer));
    this->graph_file.close();

};

// =============================================================================
void sinuca_engine_t::global_print_graph() {
    /// Open the graph file
    if (this->graph_file.is_open() == false && this->arg_graph_file_name != NULL) {
        this->graph_file.open(this->arg_graph_file_name, std::ofstream::app);
        ERROR_ASSERT_PRINTF(this->graph_file.is_open() == true, "Could not open the graph file.\n")

        this->interconnection_controller->print_graph();
        this->graph_file.close();
    }
};


// =============================================================================
void sinuca_engine_t::write_graph(const char *buffer) {
    if (this->graph_file.is_open() == true && this->arg_graph_file_name != NULL) {
        this->graph_file.write(buffer, strlen(buffer));
    }
    else {
        SINUCA_PRINTF("%s", buffer);
    }
};

// =============================================================================
void sinuca_engine_t::global_reset_statistics() {
    if (global_cycle == 0) {
        utils_t::process_mem_usage(&this->stat_vm_allocate, &this->stat_rss_allocate);
        gettimeofday(&stat_timer_start, NULL);
        gettimeofday(&stat_timer_end, NULL);
    }

    /// Save the variables after the warmup
    this->set_reset_cycle(this->global_cycle);

    /// Sinuca Reset Statistics
    for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size() ; i++) {
        this->interconnection_interface_array[i]->reset_statistics();
    }
    this->directory_controller->reset_statistics();
    this->interconnection_controller->reset_statistics();
};

// =============================================================================
void sinuca_engine_t::write_statistics(const char *buffer) {
    if (this->result_file.is_open() == true && this->arg_result_file_name != NULL) {
        this->result_file.write(buffer, strlen(buffer));
    }
    else {
        SINUCA_PRINTF("%s", buffer);
    }
};

// =============================================================================
void sinuca_engine_t::write_statistics_small_separator() {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "#=======================================\n");
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_big_separator() {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "#===============================================================================\n");
    this->write_statistics(buffer);
};

// =============================================================================
void sinuca_engine_t::write_statistics_comments(const char *comment) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "#%s\n", comment);
    this->write_statistics(buffer);
};


// =============================================================================
void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, const char *value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%s\n", obj_type, obj_label, variable_name, value);
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, bool value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%s\n", obj_type, obj_label, variable_name, value ? "TRUE" : "FALSE");
    this->write_statistics(buffer);
};


void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, uint32_t value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%u\n", obj_type, obj_label, variable_name, value);
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%" PRIu64 "\n", obj_type, obj_label, variable_name, value);
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, float value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, value);
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value(const char *obj_type, const char *obj_label, const char *variable_name, double value) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, value);
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value_percentage(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value, uint64_t total) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    if (value != 0 || total != 0) {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f%%\n", obj_type, obj_label, variable_name, static_cast<double> (100.0 *value/total));
    }
    else {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f%%\n", obj_type, obj_label, variable_name, static_cast<double> (0));
    }
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value_ratio(const char *obj_type, const char *obj_label, const char *variable_name, uint64_t value, uint64_t total) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    if (value != 0 || total != 0) {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, static_cast<double> (1.0 *value/total));
    }
    else {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, static_cast<double> (0));
    }
    this->write_statistics(buffer);
};

void sinuca_engine_t::write_statistics_value_ratio(const char *obj_type, const char *obj_label, const char *variable_name, double value, uint64_t total) {
    char buffer[TRACE_LINE_SIZE * 4] = "\0";
    if (value != 0 || total != 0) {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, static_cast<double> (1.0 *value/total));
    }
    else {
        snprintf(buffer, sizeof(buffer), "%s.%s.%s:%f\n", obj_type, obj_label, variable_name, static_cast<double> (0));
    }
    this->write_statistics(buffer);
};


// =============================================================================
void sinuca_engine_t::global_print_statistics() {
    /// Open the statistics file
    if (this->result_file.is_open() == false && this->arg_result_file_name != NULL) {
        this->result_file.open(this->arg_result_file_name, std::ofstream::app);
        ERROR_ASSERT_PRINTF(this->result_file.is_open() == true, "Could not open the result file.\n")
    }

    char title[50] = "";
    snprintf(title, sizeof(title), "Statistics of %s", this->get_label());
    this->write_statistics_big_separator();
    this->write_statistics_comments(title);
    this->write_statistics_big_separator();

    gettimeofday(&stat_timer_end, NULL);
    double time_spent = stat_timer_end.tv_sec - stat_timer_start.tv_sec + ((stat_timer_end.tv_usec - stat_timer_start.tv_usec) / 1000000.0);
    this->write_statistics_value(get_type_component_label(), get_label(), "time_spent_s", time_spent);
    this->write_statistics_value(get_type_component_label(), get_label(), "time_spent_m", time_spent / 60.0);
    this->write_statistics_value(get_type_component_label(), get_label(), "time_spent_h", time_spent / 3600.0);

    this->write_statistics_small_separator();
    utils_t::process_mem_usage(&this->stat_vm_end, &this->stat_rss_end);
    if (stat_vm_end > stat_vm_allocate + 10) {
        WARNING_PRINTF("Check for Memory Leak   VM End > VM Allocate\n")
    }
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_vm_start_mb", stat_vm_start);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_vm_allocate_mb", stat_vm_allocate);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_vm_end_mb", stat_vm_end);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_vm_max_mb", stat_vm_max);

    this->write_statistics_small_separator();
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_rss_start_mb", stat_rss_start);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_rss_allocate_mb", stat_rss_allocate);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_rss_end_mb", stat_rss_end);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_rss_max_mb", stat_rss_max);

    this->write_statistics_small_separator();
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_old_opcode_package", stat_old_opcode_package);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_old_uop_package", stat_old_uop_package);
    this->write_statistics_value(get_type_component_label(), get_label(), "stat_old_memory_package", stat_old_memory_package);

    // =========================================================================
    /// Total (with warm-up)
    this->write_statistics_small_separator();

    this->write_statistics_value(get_type_component_label(), get_label(), "global_cycle", global_cycle);
    this->write_statistics_value(get_type_component_label(), get_label(), "reset_cycle", reset_cycle);

    /// Use the LL in the end of the numbers to guarantee that 32bit compilers will not convert to a signed number
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_1_0ghz_warm", global_cycle - reset_cycle, 1000000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_1_5ghz_warm", global_cycle - reset_cycle, 1500000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_2_0ghz_warm", global_cycle - reset_cycle, 2000000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_2_5ghz_warm", global_cycle - reset_cycle, 2500000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_3_0ghz_warm", global_cycle - reset_cycle, 3000000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_3_5ghz_warm", global_cycle - reset_cycle, 3500000000LL);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "global_cycle_4_0ghz_warm", global_cycle - reset_cycle, 4000000000LL);

    this->write_statistics_small_separator();
    double kilo_instructions_simulated = 0;
    for (uint32_t i = 0 ; i < this->get_processor_array_size() ; i++) {
        kilo_instructions_simulated += this->trace_reader->get_trace_opcode_counter(i) / 1000.0;
    }
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "cycles_per_second_khz", global_cycle / 1000.0, time_spent);
    this->write_statistics_value_ratio(get_type_component_label(), get_label(), "instruction_per_second_kips", kilo_instructions_simulated, time_spent);

    // =========================================================================
    for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size() ; i++) {
        this->interconnection_interface_array[i]->print_statistics();
    }
    this->directory_controller->print_statistics();
    this->interconnection_controller->print_statistics();

    /// Close the statistics file
    if (this->result_file.is_open() == true && this->arg_result_file_name != NULL) {
        this->result_file.close();
    }
};

// =============================================================================
void sinuca_engine_t::global_print_configuration() {
    /// Open the statistics file
    if (this->result_file.is_open() == false && this->arg_result_file_name != NULL) {
        this->result_file.open(this->arg_result_file_name, std::ofstream::app);
        ERROR_ASSERT_PRINTF(this->result_file.is_open() == true, "Could not open the result file.\n")
    }

    char comment[50] = "";
    snprintf(comment, sizeof(comment), "Configuration of %s", this->get_label());
    this->write_statistics_big_separator();
    this->write_statistics_comments(comment);
    this->write_statistics_big_separator();

    this->write_statistics_value(get_type_component_label(), get_label(), "arg_configuration_file_name", arg_configuration_file_name);
    this->write_statistics_value(get_type_component_label(), get_label(), "arg_trace_file_name", arg_trace_file_name);
    this->write_statistics_value(get_type_component_label(), get_label(), "arg_result_file_name", arg_result_file_name);
    this->write_statistics_value(get_type_component_label(), get_label(), "arg_warmup_instructions", arg_warmup_instructions);
    this->write_statistics_value(get_type_component_label(), get_label(), "arg_default_core_affinity", arg_default_affinity);

    char name[100];
    for (uint32_t i = 0; i < sinuca_engine.get_processor_array_size(); i++) {
        sprintf(name, "core_%u_runs_thread", i);
        sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), name, sinuca_engine.thread_affinity[i]);
    }

    this->write_statistics_value(get_type_component_label(), get_label(), "arg_map_file_name", arg_map_file_name);

    this->write_statistics_small_separator();
    snprintf(comment, sizeof(comment), "Defines:");
    this->write_statistics_comments(comment);
    this->write_statistics_value(get_type_component_label(), get_label(), "HEART_BEAT", (uint32_t)HEART_BEAT);
    this->write_statistics_value(get_type_component_label(), get_label(), "MAX_ALIVE_TIME", (uint32_t)MAX_ALIVE_TIME);
    this->write_statistics_value(get_type_component_label(), get_label(), "PERIODIC_CHECK", (uint32_t)PERIODIC_CHECK);
    this->write_statistics_value(get_type_component_label(), get_label(), "INITIALIZE_DEBUG", (uint32_t)INITIALIZE_DEBUG);
    this->write_statistics_value(get_type_component_label(), get_label(), "FINALIZE_DEBUG", (uint32_t)FINALIZE_DEBUG);
    this->write_statistics_value(get_type_component_label(), get_label(), "INFINITE", (uint32_t)INFINITE);
    this->write_statistics_value(get_type_component_label(), get_label(), "UNDESIRABLE", (uint32_t)UNDESIRABLE);
    this->write_statistics_value(get_type_component_label(), get_label(), "TRACE_LINE_SIZE", (uint32_t)TRACE_LINE_SIZE);
    this->write_statistics_value(get_type_component_label(), get_label(), "MAX_UOP_DECODED", (uint32_t)MAX_UOP_DECODED);

    this->write_statistics_small_separator();
    this->write_statistics_value(get_type_component_label(), get_label(), "interconnection_interface_array_size", interconnection_interface_array_size);
    this->write_statistics_value(get_type_component_label(), get_label(), "processor_array_size", processor_array_size);
    this->write_statistics_value(get_type_component_label(), get_label(), "cache_memory_array_size", cache_memory_array_size);
    this->write_statistics_value(get_type_component_label(), get_label(), "memory_controller_array_size", memory_controller_array_size);
    this->write_statistics_value(get_type_component_label(), get_label(), "interconnection_router_array_size", interconnection_router_array_size);

    for (uint32_t i = 0 ; i < this->get_interconnection_interface_array_size() ; i++) {
        this->interconnection_interface_array[i]->print_configuration();
    }
    this->directory_controller->print_configuration();
    this->interconnection_controller->print_configuration();

    /// Close the statistics file
    if (this->result_file.is_open() == true && this->arg_result_file_name != NULL) {
        this->result_file.close();
    }
};
