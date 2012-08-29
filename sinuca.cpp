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
    sprintf(usage_str, " Simulates multi-core architectures with non-uniform cache architectures.\n\n");

    sprintf(usage_str, "%s Usage: sinuca CONFIGURATION TRACE [OPTION]\n", usage_str);
    sprintf(usage_str, "%s\t CONFIGURATION is a configuration file which specify the architectural parameters.\n", usage_str);
    sprintf(usage_str, "%s\t TRACE is the base name for the three (static instruction, dynamic instruction, dynamic memory) trace files.\n", usage_str);
    sprintf(usage_str, "%s\t Example: ./sinuca -conf CONFIGURATION -trace TRACE\n\n", usage_str);

    sprintf(usage_str, "%s DESCRIPTION\n", usage_str);
    sprintf(usage_str, "%s\t -conf       \t FILE   \t Configuration file which describes the architecture.\n", usage_str);
    sprintf(usage_str, "%s\t -trace      \t FILE   \t Trace file name.\n", usage_str);
    sprintf(usage_str, "%s\t -result     \t FILE   \t Output result file name.\n", usage_str);
    sprintf(usage_str, "%s\t -warmup     \t NUMBER \t Warm-up instructions (opcodes) before start statistics.\n", usage_str);
    sprintf(usage_str, "%s\t -compressed \t BOOL   \t Force to use an uncompressed trace file.\n", usage_str);

    SINUCA_PRINTF("%s\n", usage_str);
    exit(EXIT_FAILURE);
};

//==============================================================================
static void process_argv(int argc, char **argv) {
    uint32_t args_processed = 0;

    sinuca_engine.arg_configuration_file_name = NULL;
    sinuca_engine.arg_trace_file_name = NULL;
    sinuca_engine.arg_result_file_name = NULL;
    sinuca_engine.arg_warmup_instructions = 0;
    sinuca_engine.arg_is_compressed = true;

    while (argc > 0) {
        if (strcmp(*argv, "-conf") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_configuration_file_name = *argv;
            args_processed++;
        }

        else if (strcmp(*argv, "-trace") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_trace_file_name = *argv;
            args_processed++;
        }

        else if (strcmp(*argv, "-result") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_result_file_name = *argv;
            args_processed++;
        }

        else if (strcmp(*argv, "-warmup") == 0) {
            argc--;
            argv++;
            sinuca_engine.arg_warmup_instructions = atoi(*argv);
            args_processed++;
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

    SINUCA_PRINTF("CONFIGURATION FILE:      \t %s\n", sinuca_engine.arg_configuration_file_name    != NULL ? sinuca_engine.arg_configuration_file_name : "MISSING");
    SINUCA_PRINTF("TRACE FILE:              \t %s\n", sinuca_engine.arg_trace_file_name            != NULL ? sinuca_engine.arg_trace_file_name         : "MISSING");
    SINUCA_PRINTF("RESULT FILE:             \t %s\n", sinuca_engine.arg_result_file_name           != NULL ? sinuca_engine.arg_result_file_name        : "MISSING");
    SINUCA_PRINTF("WARM-UP INSTRUCTIONS:    \t %u\n", sinuca_engine.arg_warmup_instructions);
    SINUCA_PRINTF("COMPRESSED TRACE:        \t %s\n", sinuca_engine.arg_is_compressed ? "TRUE" : "FALSE");

    if (args_processed < 2) {
        display_use();
    }
};

//==============================================================================
void premature_termination(int param) {
    SINUCA_PRINTF("\n\nCode(%d):", param);
    sinuca_engine.premature_termination();
    exit(EXIT_FAILURE);
}

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

        /// Spawn Warmup
        if (sinuca_engine.is_warm_up == true) {
            SINUCA_PRINTF("WARM-UP END - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
            sinuca_engine.global_reset_statistics();
            sinuca_engine.is_warm_up = false;
        }

        /// Progress Information
        if ((sinuca_engine.get_global_cycle() % HEART_BEAT) == 0) {
            SINUCA_PRINTF("HEART-BEAT - CYCLE: %"PRIu64"\n", sinuca_engine.get_global_cycle() );
            char processor_report[1000];
            for (uint32_t cpu = 0 ; cpu < sinuca_engine.get_processor_array_size() ; cpu++) {
                uint64_t ActualLength = sinuca_engine.trace_reader->get_trace_opcode_counter(cpu);
                uint64_t FullLength = sinuca_engine.trace_reader->get_trace_opcode_max(cpu);
                sprintf(processor_report, "\t -- CPU %d", cpu);
                sprintf(processor_report, "%s - Opcode[%10"PRIu64"/%10"PRIu64"]", processor_report, ActualLength, FullLength);
                sprintf(processor_report, "%s - (%7.3lf%%)", processor_report, 100.0 * ((double)ActualLength / (double)FullLength));
                sprintf(processor_report, "%s - %s", processor_report, utils_t::progress_pretty(ActualLength, FullLength).c_str());
                sprintf(processor_report, "%s - IPC(%5.3lf)", processor_report, (double)ActualLength / (double)sinuca_engine.get_global_cycle() );
                sprintf(processor_report, "%s - [%s]", processor_report, sinuca_engine.is_processor_trace_eof[cpu] ? "OFF-LINE" : "ON-LINE");
                SINUCA_PRINTF("%s\n",processor_report);
            }
            SINUCA_PRINTF("\n");
        }

        /// Spawn Periodic Check
        if ((sinuca_engine.get_global_cycle() % PERIODIC_CHECK) == 0) {
            sinuca_engine.global_periodic_check();
        }

        /// Spawn Clock Signal
        sinuca_engine.global_clock();
    }

    sinuca_engine.global_print_configuration();
    sinuca_engine.global_print_statistics();

    exit(EXIT_SUCCESS);
};

