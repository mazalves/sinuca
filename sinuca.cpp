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
#include "./sinuca.hpp"
sinuca_engine_t sinuca_engine;

//==============================================================================
static void display_use() {
    char usage_str[100000];
    strcpy(usage_str, "Simulates multi-core architectures with non-uniform cache architectures.\n\n");

    strcat(usage_str, "Usage: sinuca CONFIGURATION TRACE [OPTIONS] \n");
    strcat(usage_str, "\t CONFIGURATION is a configuration file which specify the architectural parameters.\n");
    strcat(usage_str, "\t TRACE is the base name for the three (static instruction, dynamic instruction, dynamic memory) trace files.\n");
    strcat(usage_str, "\t Example: ./sinuca -conf CONFIGURATION -trace TRACE\n\n");

    strcat(usage_str, " DESCRIPTION\n");
    strcat(usage_str, "\t -conf       \t FILE          \t Configuration file which describes the architecture.\n");
    strcat(usage_str, "\t -trace      \t FILE          \t Trace file name.\n");
    strcat(usage_str, "\t -result     \t FILE          \t Output result file name. Default is \"stdout\".\n");
    strcat(usage_str, "\t -warmup     \t INSTRUCTIONS  \t Warm-up instructions (opcodes) before start statistics. Default is 0.\n");
    strcat(usage_str, "\t -compressed \t BOOL          \t Set between the compressed (true) and uncompressed (false) trace file. Default is true.\n");
    strcat(usage_str, "\t -graph      \t FILE          \t Output graph file name to be used with GraphViz. Default is \"stdout\".\n");

    SINUCA_PRINTF("%s\n", usage_str);
    exit(EXIT_FAILURE);
};

//==============================================================================
static void process_argv(int argc, char **argv) {
    uint32_t req_args_processed = 0;

    sinuca_engine.arg_configuration_file_name = NULL;
    sinuca_engine.arg_trace_file_name = NULL;
    sinuca_engine.arg_result_file_name = NULL;
    sinuca_engine.arg_warmup_instructions = 0;
    sinuca_engine.arg_is_compressed = true;
    sinuca_engine.arg_graph_file_name = NULL;

    while (argc > 0) {
        if (strcmp(*argv, "-conf") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_configuration_file_name = *argv;
            req_args_processed++;
        }

        else if (strcmp(*argv, "-trace") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_trace_file_name = *argv;
            req_args_processed++;
        }

        else if (strcmp(*argv, "-result") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_result_file_name = *argv;
        }

        else if (strcmp(*argv, "-warmup") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_warmup_instructions = atoi(*argv);
        }

        else if (strcmp(*argv, "-compressed") == 0) {
            argc--;
            argv++;
            if (strcmp(*argv, "true") == 0) {
                sinuca_engine.arg_is_compressed = true;
            }
            else if (strcmp(*argv, "false") == 0) {
                sinuca_engine.arg_is_compressed = false;
            }
            else {
                display_use();
            }
        }

        else if (strcmp(*argv, "-graph") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_graph_file_name = *argv;
        }


        else if (strncmp(*argv, "-", 1) == 0) {
            SINUCA_PRINTF("Unknown option %s\n", *argv);
            display_use();
        }

        else {
            display_use();
        }
        argc--;
        argv++;

    }

    if (req_args_processed < 2) {
        display_use();
    }

    uint32_t configuration_file_size;
    configuration_file_size = strlen(sinuca_engine.arg_configuration_file_name) + 1;
    sinuca_engine.arg_configuration_path = utils_t::template_allocate_array<char>(configuration_file_size);
    utils_t::get_path(sinuca_engine.arg_configuration_path, sinuca_engine.arg_configuration_file_name);

    SINUCA_PRINTF("CONFIGURATION FILE:      \t %s\n", sinuca_engine.arg_configuration_file_name     != NULL ? sinuca_engine.arg_configuration_file_name : "MISSING");
    SINUCA_PRINTF("CONFIGURATION PATH:      \t %s\n", sinuca_engine.arg_configuration_path          != NULL ? sinuca_engine.arg_configuration_path      : "MISSING");
    SINUCA_PRINTF("TRACE FILE:              \t %s\n", sinuca_engine.arg_trace_file_name             != NULL ? sinuca_engine.arg_trace_file_name         : "MISSING");
    SINUCA_PRINTF("RESULT FILE:             \t %s\n", sinuca_engine.arg_result_file_name            != NULL ? sinuca_engine.arg_result_file_name        : "MISSING");
    SINUCA_PRINTF("WARM-UP INSTRUCTIONS:    \t %u\n", sinuca_engine.arg_warmup_instructions);
    SINUCA_PRINTF("COMPRESSED TRACE:        \t %s\n", sinuca_engine.arg_is_compressed                       ? "TRUE" : "FALSE");
    SINUCA_PRINTF("GRAPH FILE:              \t %s\n", sinuca_engine.arg_graph_file_name             != NULL ? sinuca_engine.arg_graph_file_name        : "MISSING");

};

//==============================================================================
void premature_termination(int param) {
    SINUCA_PRINTF("\n\nCode(%d):", param);
    sinuca_engine.premature_termination();
    exit(EXIT_FAILURE);
}

//==============================================================================
//  > CPU  7 - Opcode[         1/   2095984] -   0.000% [                    ] IPC(  inf) [   SYNC_FREE] [ON]
std::string simulation_status_to_string() {
    std::string final_report;
    char processor_report[1000];
    char tmp_char[1000];

    for (uint32_t cpu = 0 ; cpu < sinuca_engine.get_processor_array_size() ; cpu++) {
        uint64_t ActualLength = sinuca_engine.trace_reader->get_trace_opcode_counter(cpu);
        uint64_t FullLength = sinuca_engine.trace_reader->get_trace_opcode_max(cpu) + 1;
        sprintf(tmp_char, "  > CPU %2d", cpu);
        strcpy(processor_report, tmp_char);

        sprintf(tmp_char, " - Opcode[%10"PRIu64"/%10"PRIu64"]", ActualLength, FullLength);
        strcat(processor_report, tmp_char);

        double percentage_complete = 100.0 * ((double)ActualLength / (double)FullLength);
        sprintf(tmp_char, " - %7.3lf%%", percentage_complete);
        strcat(processor_report, tmp_char);

        sprintf(tmp_char, " %s", utils_t::progress_pretty(ActualLength, FullLength).c_str());
        strcat(processor_report, tmp_char);

        sprintf(tmp_char, " IPC(%5.3lf)", (double)ActualLength / (double)sinuca_engine.get_global_cycle() );
        strcat(processor_report, tmp_char);

        sprintf(tmp_char, " [%12s]", get_enum_sync_char(sinuca_engine.processor_array[cpu]->get_sync_status()));
        strcat(processor_report, tmp_char);

        sprintf(tmp_char, " [%s]", sinuca_engine.is_processor_trace_eof[cpu] ? "OFF" : "ON");
        strcat(processor_report, tmp_char);

        gettimeofday(&sinuca_engine.stat_timer_end, NULL);
        double seconds_spent = sinuca_engine.stat_timer_end.tv_sec - sinuca_engine.stat_timer_start.tv_sec +
                                ((sinuca_engine.stat_timer_end.tv_usec - sinuca_engine.stat_timer_start.tv_usec) / 1000000.0);

        uint64_t seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
        sprintf(tmp_char, " ETC(%02.0f:%02.0f:%02.0f)", floor(seconds_remaining/3600.0), floor(fmod(seconds_remaining,3600.0)/60.0), fmod(seconds_remaining,60.0));
        strcat(processor_report, tmp_char);

        strcat(processor_report, "\n");

        final_report += processor_report;
    }
    return final_report;
};

//==============================================================================
int main(int argc, char **argv) {
    process_argv(argc-1, argv+1);

    /// ========================================================================
    /// Abnormal termination handler
    void (*prev_fn)(int);
    /// (Signal Terminate) Termination request sent to program.
    prev_fn = signal(SIGTERM, premature_termination);
    /// (Signal Interrupt) Interactive attention signal. Generally generated by the application user.
    prev_fn = signal(SIGINT, premature_termination);

    if (prev_fn == SIG_IGN) {
        signal(SIGTERM, SIG_IGN);
    }

    sinuca_engine.initialize();
    sinuca_engine.is_processor_trace_eof = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_processor_array_size(), false);
    sinuca_engine.trace_reader->allocate(sinuca_engine.arg_trace_file_name, sinuca_engine.arg_is_compressed, sinuca_engine.get_processor_array_size());

    sinuca_engine.global_reset_statistics();

    SINUCA_PRINTF("WARM-UP START - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );

    /// Start CLOCK
    while (sinuca_engine.get_is_simulation_allocated() && sinuca_engine.alive()) {

        /// Spawn Warmup - is_warm_up is set inside the trace_reader
        if (sinuca_engine.is_warm_up == true) {
            SINUCA_PRINTF("WARM-UP END - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
            sinuca_engine.global_reset_statistics();
            sinuca_engine.is_warm_up = false;
        }

        /// Progress Information
        if ((sinuca_engine.get_global_cycle() % HEART_BEAT) == 0) {
            SINUCA_PRINTF("HEART-BEAT - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
            SINUCA_PRINTF("%s\n", simulation_status_to_string().c_str());
        }

        /// Spawn Periodic Check
        if ((sinuca_engine.get_global_cycle() % PERIODIC_CHECK) == 0) {
            sinuca_engine.global_periodic_check();
        }

        /// Spawn Clock Signal
        sinuca_engine.global_clock();
    }

    /// Evict all the cache lines.
    bool all_evicted = false;
    while (all_evicted != true){
        /// Progress Information
        if ((sinuca_engine.get_global_cycle() % HEART_BEAT) == 0) {
            SINUCA_PRINTF("Mass Eviction... HEART-BEAT - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
            SINUCA_PRINTF("%s\n", simulation_status_to_string().c_str());
        }

        /// Spawn Periodic Check
        if ((sinuca_engine.get_global_cycle() % PERIODIC_CHECK) == 0) {
            sinuca_engine.global_periodic_check();
        }

        all_evicted = sinuca_engine.directory_controller->coherence_evict_all();

        /// Spawn Clock Signal
        sinuca_engine.global_clock();
    }

    SINUCA_PRINTF("SIMULATION FINISHED - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
    SINUCA_PRINTF("%s\n", simulation_status_to_string().c_str());

    sinuca_engine.global_print_configuration();
    sinuca_engine.global_print_statistics();
    sinuca_engine.global_print_graph();

    exit(EXIT_SUCCESS);
};

