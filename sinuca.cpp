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
#include <string>

sinuca_engine_t sinuca_engine;

// =============================================================================
static void display_use() {
    SINUCA_PRINTF("SiNUCA simulates multi-core architectures with non-uniform cache architectures.\n\n");

    SINUCA_PRINTF("Usage: sinuca CONFIGURATION TRACE [OPTIONS] \n");
    SINUCA_PRINTF("\t CONFIGURATION is a configuration file which specify the architectural parameters.\n");
    SINUCA_PRINTF("\t TRACE is the base name for the three (static instruction, dynamic instruction, dynamic memory) trace files.\n");
    SINUCA_PRINTF("\t Example: ./sinuca -config CONFIGURATION -trace TRACE\n\n");

    SINUCA_PRINTF(" DESCRIPTION\n");
    SINUCA_PRINTF("\t -config     \t FILE          \t Configuration file which describes the architecture. **Required\n");
    SINUCA_PRINTF("\t -trace      \t FILE          \t Trace file base name. **Required\n");
    SINUCA_PRINTF("\t -result     \t FILE          \t Output result file name. Default is \"stdout\".\n");
    SINUCA_PRINTF("\t -warmup     \t INSTRUCTIONS  \t Warm-up instructions (opcodes) before start statistics. Default is 0.\n");
    SINUCA_PRINTF("\t -compressed \t BOOL          \t Set between the compressed (true) and uncompressed (false) trace file. Default is true.\n");
    SINUCA_PRINTF("\t -graph      \t FILE          \t Output graph file name to be used with GraphViz.\n");
    SINUCA_PRINTF("\t -map        \t THREADS       \t Inform a different mapping between the trace files and the cores.\n");

    exit(EXIT_FAILURE);
};

// =============================================================================
static void process_argv(int argc, char **argv) {
    uint32_t req_args_processed = 0;
    uint32_t core_map = 0;

    sinuca_engine.arg_configuration_file_name = NULL;
    sinuca_engine.arg_trace_file_name = NULL;
    sinuca_engine.arg_result_file_name = NULL;
    sinuca_engine.arg_warmup_instructions = 0;
    sinuca_engine.arg_is_compressed = true;
    sinuca_engine.arg_graph_file_name = NULL;

    // Should start using Getopt

    struct stat buf;

    while (argc > 0) {
        if (strcmp(*argv, "-config") == 0) {
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
            if (stat(sinuca_engine.arg_result_file_name, &buf) == false) {
                SINUCA_PRINTF("Result file already exist: %s\n\n", sinuca_engine.arg_result_file_name)
                display_use();
            }
        }
        else if (strcmp(*argv, "-warmup") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_warmup_instructions = atoi(*argv);
            if (atoi(*argv) < 0) {
                SINUCA_PRINTF(">> Warm-up instructions should be greater or equal than zero.\n\n")
                display_use();
            }
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
            if (stat(sinuca_engine.arg_graph_file_name, &buf) == false) {
                SINUCA_PRINTF("Result file already exist: %s\n\n", sinuca_engine.arg_graph_file_name)
                display_use();
            }
        }
        else if (strcmp(*argv, "-map") == 0) {
            argc--;
            argv++;
            char * pch;
            pch = strtok(*argv, ",");
            core_map = 0;
            while (pch != NULL) {
                sinuca_engine.thread_map[core_map++] = atoi(pch);
                pch = strtok(NULL, ",");
            }
        }
        else if (strncmp(*argv, "-", 1) == 0) {
            SINUCA_PRINTF(">> Unknown option %s\n\n", *argv);
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

    sinuca_engine.global_open_output_files();

    SINUCA_PRINTF("=======================  SiNUCA  =======================\n");
    SINUCA_PRINTF("CONFIGURATION FILE: %s\n", sinuca_engine.arg_configuration_file_name != NULL ? sinuca_engine.arg_configuration_file_name : "MISSING");
    SINUCA_PRINTF("CONFIGURATION PATH: %s\n", sinuca_engine.arg_configuration_path      != NULL ? sinuca_engine.arg_configuration_path      : "MISSING");
    SINUCA_PRINTF("TRACE FILE:         %s\n", sinuca_engine.arg_trace_file_name         != NULL ? sinuca_engine.arg_trace_file_name         : "MISSING");
    SINUCA_PRINTF("RESULT FILE:        %s\n", sinuca_engine.arg_result_file_name        != NULL ? sinuca_engine.arg_result_file_name        : "MISSING");
    SINUCA_PRINTF("WARM-UP OPCODES:    %u\n", sinuca_engine.arg_warmup_instructions);
    SINUCA_PRINTF("COMPRESSED TRACE:   %s\n", sinuca_engine.arg_is_compressed                   ? "TRUE" : "FALSE");
    SINUCA_PRINTF("GRAPH FILE:         %s\n", sinuca_engine.arg_graph_file_name         != NULL ? sinuca_engine.arg_graph_file_name        : "MISSING");
    SINUCA_PRINTF("MAP:                %s\n", core_map == 0 ? "DEFAULT" : "USER DEFINED");
    for (uint32_t i=0; i<core_map; i++) {
        SINUCA_PRINTF("\t Trace[%"PRIu32"] -> Core[%"PRIu32"]\n", sinuca_engine.thread_map[i], i);
    }

};

// =============================================================================
static void premature_termination(int signo) {
    switch(signo) {
        case SIGABRT : SINUCA_PRINTF("SiNUCA received SIGABRT signal."); break;
        case SIGILL  : SINUCA_PRINTF("SiNUCA received SIGILL  signal."); break;
        case SIGINT  : SINUCA_PRINTF("SiNUCA received SIGINT  signal."); break;
        case SIGSEGV : SINUCA_PRINTF("SiNUCA received SIGSEGV signal."); break;
        case SIGTERM : SINUCA_PRINTF("SiNUCA received SIGTERM signal."); break;
    }

    sinuca_engine.premature_termination();
    exit(EXIT_FAILURE);
}

// =============================================================================
std::string simulation_status_to_string() {
    std::string final_report;
    char tmp_char[1000];

    uint64_t ActualLength = 0;
    uint64_t FullLength = 0;
    uint32_t active_cores = 0;

    /// Get seconds without micro-seconds
    gettimeofday(&sinuca_engine.stat_timer_end, NULL);
    double seconds_spent = sinuca_engine.stat_timer_end.tv_sec - sinuca_engine.stat_timer_start.tv_sec;

    /// Get global statistics from all the cores
    for (uint32_t cpu = 0 ; cpu < sinuca_engine.get_processor_array_size() ; cpu++) {
        ActualLength += sinuca_engine.trace_reader->get_trace_opcode_counter(cpu);
        FullLength += sinuca_engine.trace_reader->get_trace_opcode_max(cpu) + 1;
        active_cores += !sinuca_engine.is_processor_trace_eof[cpu];
    }
    // =====================================================================
    /// Computer the Global Percentage Completed
    double percentage_complete = 100.0 * (static_cast<double>(ActualLength) / static_cast<double>(FullLength));
    snprintf(tmp_char, sizeof(tmp_char), " Progress: %8.4lf%%", percentage_complete);
    final_report += tmp_char;

    snprintf(tmp_char, sizeof(tmp_char), " %s", utils_t::progress_pretty(ActualLength, FullLength).c_str());
    final_report += tmp_char;

    snprintf(tmp_char, sizeof(tmp_char), " Active Cores: %02"PRIu32"", active_cores);
    final_report += tmp_char;

    /// Compute the Global Estimate Time to Complete (GETC)
    uint64_t seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
    snprintf(tmp_char, sizeof(tmp_char), "     Global ETC(%02.0f:%02.0f:%02.0f)\n", floor(seconds_remaining / 3600.0),
                                                                floor(fmod(seconds_remaining, 3600.0) / 60.0),
                                                                fmod(seconds_remaining, 60.0));
    final_report += tmp_char;

    for (uint32_t cpu = 0 ; cpu < sinuca_engine.get_processor_array_size() ; cpu++) {
        ActualLength = sinuca_engine.trace_reader->get_trace_opcode_counter(cpu);
        FullLength = sinuca_engine.trace_reader->get_trace_opcode_max(cpu) + 1;

        snprintf(tmp_char, sizeof(tmp_char), "  > CPU %02d", cpu);
        final_report += tmp_char;

        snprintf(tmp_char, sizeof(tmp_char), " - Opcode[%10"PRIu64"/%10"PRIu64"]", ActualLength, FullLength);
        final_report += tmp_char;

        percentage_complete = 100.0 * (static_cast<double>(ActualLength) / static_cast<double>(FullLength));;
        snprintf(tmp_char, sizeof(tmp_char), " %8.4lf%%", percentage_complete);
        final_report += tmp_char;

        snprintf(tmp_char, sizeof(tmp_char), " %s", utils_t::progress_pretty(ActualLength, FullLength).c_str());
        final_report += tmp_char;

        snprintf(tmp_char, sizeof(tmp_char), " IPC(%5.3lf)", static_cast<double>(ActualLength) / static_cast<double>(sinuca_engine.get_global_cycle()));
        final_report += tmp_char;

        snprintf(tmp_char, sizeof(tmp_char), " [%9s]", get_enum_sync_char(sinuca_engine.processor_array[cpu]->get_sync_status()));
        final_report += tmp_char;

        snprintf(tmp_char, sizeof(tmp_char), " [%s]", sinuca_engine.is_processor_trace_eof[cpu] ? "OFF" : "ON");
        final_report += tmp_char;

        // =====================================================================
        /// Compute the Estimate Time to Complete (ETC)
        seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
        snprintf(tmp_char, sizeof(tmp_char), " ETC(%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_remaining / 3600.0),
                                                floor(fmod(seconds_remaining, 3600.0) / 60.0),
                                                fmod(seconds_remaining, 60.0));
        final_report += tmp_char;
    }
    return final_report;
};

// =============================================================================
int main(int argc, char **argv) {
    process_argv(argc-1, argv+1);

    // =========================================================================
    /// Register a user-defined signal handler.
    signal(SIGABRT, premature_termination);   ///< It is sent to a process to tell it to abort, i.e. to terminate.
    signal(SIGILL,  premature_termination);   ///< It is sent to a process when it attempts to execute an illegal instruction.
    signal(SIGINT,  premature_termination);   ///< It is sent to a process by its controlling terminal when a user interrupt the process.
    signal(SIGSEGV, premature_termination);   ///< It is sent to a process when it makes an invalid virtual memory reference, or seg. fault.
    signal(SIGTERM, premature_termination);   ///< It is sent to a process to request its termination.

    sinuca_engine.initialize();


    sinuca_engine.is_processor_trace_eof = utils_t::template_allocate_initialize_array<bool>(sinuca_engine.get_processor_array_size(), false);
    sinuca_engine.trace_reader->allocate(sinuca_engine.arg_trace_file_name, sinuca_engine.arg_is_compressed, sinuca_engine.get_processor_array_size());

    sinuca_engine.global_reset_statistics();

    SINUCA_PRINTF("\n");
    SINUCA_PRINTF("=====================  Simulating  =====================\n");

    SINUCA_PRINTF("Warm-Up Start - Cycle: %-12"PRIu64"\n", sinuca_engine.get_global_cycle() );

    /// Start CLOCK
    while (sinuca_engine.get_is_simulation_allocated() && sinuca_engine.alive()) {
        /// Spawn Warmup - is_warm_up is set inside the trace_reader
        if (sinuca_engine.is_warm_up == true) {
            SINUCA_PRINTF("Warm-Up End - Cycle: %-12"PRIu64"\n", sinuca_engine.get_global_cycle() );

            sinuca_engine.global_reset_statistics();
            sinuca_engine.is_warm_up = false;
        }

        /// Progress Information
        if ((sinuca_engine.get_global_cycle() % HEART_BEAT) == 0) {
            SINUCA_PRINTF("Heart-Beat - Cycle: %-12"PRIu64"", sinuca_engine.get_global_cycle() );
            SINUCA_PRINTF("%s\n", simulation_status_to_string().c_str());
        }

        /// Spawn Periodic Check
        if ((sinuca_engine.get_global_cycle() % PERIODIC_CHECK) == 0) {
            sinuca_engine.global_periodic_check();
        }

        /// Spawn Clock Signal
        sinuca_engine.global_clock();
    }

    SINUCA_PRINTF("Evicting all cache lines... \n")

    /// Evict all the cache lines.
    bool all_evicted = false;
    while (all_evicted != true){
        /// Progress Information
        if ((sinuca_engine.get_global_cycle() % HEART_BEAT) == 0) {
            SINUCA_PRINTF("Heart-Beat - Cycle: %-12"PRIu64"\n", sinuca_engine.get_global_cycle() );
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

    SINUCA_PRINTF("!Finished! - Cycle: %-12"PRIu64"", sinuca_engine.get_global_cycle() );
    SINUCA_PRINTF("%s\n", simulation_status_to_string().c_str());

    sinuca_engine.global_print_configuration();
    sinuca_engine.global_print_statistics();
    sinuca_engine.global_print_graph();

    exit(EXIT_SUCCESS);
};

