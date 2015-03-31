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

#include "../sinuca.hpp"
#include <string>

#ifdef TRACE_READER_DEBUG
    #define TRACE_READER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_READER_DEBUG_PRINTF(...)
#endif

// =============================================================================
trace_reader_t::trace_reader_t() {
    this->gzStaticTraceFile = NULL;
    this->gzDynamicTraceFile = NULL;
    this->gzMemoryTraceFile = NULL;

    this->trace_opcode_total = 0;

    this->insideBBL = NULL;
    this->trace_opcode_counter = NULL;
    this->trace_opcode_max = NULL;

    this->actual_bbl = NULL;
    this->actual_bbl_opcode = NULL;

    this->total_bbls = 0;
    this->bbl_size = NULL;

    this->line_static = NULL;
    this->line_dynamic = NULL;
    this->line_memory = NULL;
};

// =============================================================================
trace_reader_t::~trace_reader_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<char>(line_static);
    utils_t::template_delete_matrix<char>(line_dynamic, TRACE_LINE_SIZE);
    utils_t::template_delete_matrix<char>(line_memory, TRACE_LINE_SIZE);

    utils_t::template_delete_array<gzFile>(gzDynamicTraceFile);
    utils_t::template_delete_array<gzFile>(gzMemoryTraceFile);

    utils_t::template_delete_array<bool>(insideBBL);
    utils_t::template_delete_array<uint64_t>(trace_opcode_counter);
    utils_t::template_delete_array<uint64_t>(trace_opcode_max);

    utils_t::template_delete_array<uint32_t>(actual_bbl);
    utils_t::template_delete_array<uint32_t>(actual_bbl_opcode);

};

// =============================================================================
void trace_reader_t::allocate(char *in_file, uint32_t number_cores) {
    uint32_t i, k;

    this->line_static = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    this->line_dynamic = utils_t::template_allocate_matrix<char>(number_cores, TRACE_LINE_SIZE);
    this->line_memory = utils_t::template_allocate_matrix<char>(number_cores, TRACE_LINE_SIZE);

    this->line_static[0] = '\0';
    for (i = 0; i < number_cores; i++) {
        this->line_dynamic[i][0] = '\0';
        this->line_memory[i][0] = '\0';
    }

    // =======================================================================
    // Static Trace File
    // =======================================================================
    char stat_file_name[500];
    stat_file_name[0] = '\0';

    TRACE_READER_DEBUG_PRINTF("Inserted Output File Name = %s\n", in_file);

    snprintf(stat_file_name, sizeof(stat_file_name), "%s.tid0.stat.out.gz", in_file);
    gzStaticTraceFile = gzopen(stat_file_name, "ro");   /// Open the .gz file
    ERROR_ASSERT_PRINTF(gzStaticTraceFile != NULL, "Could not open the file.\n%s\n", stat_file_name);

    TRACE_READER_DEBUG_PRINTF("Static File = %s => READY !\n", stat_file_name);

    // =======================================================================
    // Dynamic Trace Files
    // =======================================================================
    char dyn_file_name[500];

    gzDynamicTraceFile = utils_t::template_allocate_array<gzFile>(number_cores);
    for (i = 0; i < number_cores; i++) {
        /// Make the thread affinity (mapping)
        k = sinuca_engine.thread_map[i];

        dyn_file_name[0] = '\0';
        snprintf(dyn_file_name, sizeof(dyn_file_name), "%s.tid%d.dyn.out.gz", in_file, k);

        /// Create missing files in order to run single threaded app into a multi-core
        struct stat st;
        /// file do not exist
        if (stat(dyn_file_name, &st) != 0) {
            WARNING_PRINTF("FILE NOT FOUND %s. ", dyn_file_name);
            snprintf(dyn_file_name, sizeof(dyn_file_name), "/tmp/NULL_tid%d.dyn.out.gz", k);
            WARNING_PRINTF("=> CREATED %s.\n", dyn_file_name);
            gzDynamicTraceFile[i] = gzopen(dyn_file_name, "wo");    /// Open the .gz file
            gzwrite(gzDynamicTraceFile[i], "#Empty Trace\n", strlen("#Empty Trace\n"));
            gzclose(gzDynamicTraceFile[i]);
        }

        gzDynamicTraceFile[i] = gzopen(dyn_file_name, "ro");    /// Open the .gz file
        ERROR_ASSERT_PRINTF(gzDynamicTraceFile[i] != NULL, "Could not open the file.\n%s\n", dyn_file_name);
        TRACE_READER_DEBUG_PRINTF("Dynamic File = %s => READY !\n", dyn_file_name);
    }

    // =======================================================================
    // Memory Trace Files
    // =======================================================================
    char mem_file_name[500];
    mem_file_name[0] = '\0';

    gzMemoryTraceFile = utils_t::template_allocate_array<gzFile>(number_cores);
    for (i = 0; i < number_cores; i++) {
        /// Make the thread affinity (mapping)
        k = sinuca_engine.thread_map[i];

        snprintf(mem_file_name, sizeof(mem_file_name), "%s.tid%d.mem.out.gz", in_file, k);

        /// Create missing files in order to run single threaded app into a multi-core
        struct stat st;
        /// file do not exist
        if (stat(mem_file_name, &st) != 0) {
            WARNING_PRINTF("FILE NOT FOUND %s. ", mem_file_name);
            snprintf(mem_file_name, sizeof(mem_file_name), "/tmp/NULL_tid%d.mem.out.gz", k);
            WARNING_PRINTF("=> CREATED %s.\n", mem_file_name);
            gzMemoryTraceFile[i] = gzopen(mem_file_name, "wo");    /// Open the .gz file
            gzwrite(gzMemoryTraceFile[i], "#Empty Trace\n", strlen("#Empty Trace\n"));
            gzclose(gzMemoryTraceFile[i]);
        }

        gzMemoryTraceFile[i] = gzopen(mem_file_name, "ro");     /// Open the .gz file
        ERROR_ASSERT_PRINTF(gzMemoryTraceFile[i] != NULL, "Could not open the file.\n%s\n", mem_file_name);
        TRACE_READER_DEBUG_PRINTF("Memory File = %s => READY !\n", mem_file_name);
    }

    /// Obtain the total of BBLs
    this->define_total_bbls();
    this->bbl_size = utils_t::template_allocate_initialize_array<uint32_t>(this->total_bbls, 0);
    this->define_total_bbl_size();

    /// Allocate only the required space for the static file packages
    this->static_dict = utils_t::template_allocate_array<opcode_package_t*>(this->total_bbls);
    for (i = 1; i < this->total_bbls; i++) {
        this->static_dict[i] = utils_t::template_allocate_array<opcode_package_t>(this->bbl_size[i]);
    }

    this->generate_static_dict();

    this->insideBBL = utils_t::template_allocate_initialize_array<bool>(number_cores, false);
    this->trace_opcode_counter = utils_t::template_allocate_initialize_array<uint64_t>(number_cores, 1);
    this->trace_opcode_max = utils_t::template_allocate_array<uint64_t>(number_cores);
    for (i = 0; i < number_cores; i++) {
        this->trace_opcode_max[i] = this->trace_size(i);
    }

    this->actual_bbl = utils_t::template_allocate_initialize_array<uint32_t>(number_cores, 0);
    this->actual_bbl_opcode = utils_t::template_allocate_initialize_array<uint32_t>(number_cores, 0);
};


// =============================================================================
void trace_reader_t::define_total_bbls() {
    bool file_eof = false;
    uint32_t BBL = 0;
    this->total_bbls = 0;

    gzclearerr(this->gzStaticTraceFile);
    gzseek(this->gzStaticTraceFile, 0, SEEK_SET);   /// Go to the Begin of the File
    file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
    ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")

    while (!file_eof) {
        this->line_static[0] = '\0';

        gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
        file_eof = gzeof(this->gzStaticTraceFile);

        if (this->line_static[0] == '@') {
            BBL = (uint32_t)strtoul(this->line_static + 1, NULL, 10);
            this->total_bbls++;
            ERROR_ASSERT_PRINTF(BBL == this->total_bbls, "Expected sequenced BBLs.\n")
        }
    }

    this->total_bbls++;
    TRACE_READER_DEBUG_PRINTF("Total of BBLs %u", this->total_bbls);
};

// =============================================================================
void trace_reader_t::define_total_bbl_size() {
    bool file_eof = false;
    uint32_t BBL = 0;

    gzclearerr(this->gzStaticTraceFile);
    gzseek(this->gzStaticTraceFile, 0, SEEK_SET);   /// Go to the Begin of the File
    file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
    ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")

    while (!file_eof) {
        gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
        file_eof = gzeof(this->gzStaticTraceFile);

        if (this->line_static[0] == '\0' || this->line_static[0] == '#') {     /// If Comment, then ignore
            continue;
        }
        else if (this->line_static[0] == '@') {
            BBL++;
        }
        else {
            this->bbl_size[BBL]++;
        }
    }
};


// =============================================================================
/// Get the total number of opcodes
uint64_t trace_reader_t::trace_size(uint32_t cpuid) {
    bool file_eof = false;
    uint32_t BBL = 0;
    uint64_t trace_size = 0;

    gzclearerr(this->gzDynamicTraceFile[cpuid]);
    gzseek(this->gzDynamicTraceFile[cpuid], 0, SEEK_SET);   /// Go to the Begin of the File
    file_eof = gzeof(this->gzDynamicTraceFile[cpuid]);      /// Check is file not EOF
    ERROR_ASSERT_PRINTF(!file_eof, "Dynamic File Unexpected EOF.\n")

    while (!file_eof) {
        gzgets(this->gzDynamicTraceFile[cpuid], this->line_dynamic[cpuid], TRACE_LINE_SIZE);
        file_eof = gzeof(this->gzDynamicTraceFile[cpuid]);

        if (this->line_dynamic[cpuid][0] != '\0' && this->line_dynamic[cpuid][0] != '#' && this->line_dynamic[cpuid][0] != '$') {
            BBL = (uint32_t)strtoul(this->line_dynamic[cpuid], NULL, 10);
            trace_size += this->bbl_size[BBL];
        }
    }

    gzclearerr(this->gzDynamicTraceFile[cpuid]);            /// Go to the Begin of the File
    gzseek(this->gzDynamicTraceFile[cpuid], 0, SEEK_SET);

    return(trace_size);
};

// =============================================================================
uint32_t trace_reader_t::trace_next_dynamic(uint32_t cpuid, sync_t *new_sync) {
    uint32_t BBL = 0;

    /// Auxiliary strings
    this->line_dynamic[cpuid][0] = '\0';

    bool valid_dynamic = false;
    sync_t &sync_found = *new_sync;

    while (!valid_dynamic) {
        if (gzeof(this->gzDynamicTraceFile[cpuid])) {
            sinuca_engine.set_is_processor_trace_eof(cpuid);
            return FAIL;
        }
        gzgets(this->gzDynamicTraceFile[cpuid], this->line_dynamic[cpuid], TRACE_LINE_SIZE);

        if (this->line_dynamic[cpuid][0] == '\0' || this->line_dynamic[cpuid][0] == '#') {
            TRACE_READER_DEBUG_PRINTF("cpu[%d] - %s\n", cpuid, this->line_dynamic[cpuid]);
            continue;
        }
        else if (this->line_dynamic[cpuid][0] == '$') {
            sync_found = (sync_t)strtoul(this->line_dynamic[cpuid] + 1, NULL, 10);
            return FAIL;
        }
        else {
            /// BBL is always greater than 0
            /// If strtoul==0 the line could not be converted.
            TRACE_READER_DEBUG_PRINTF("cpu[%d] BBL = %s\n", cpuid, this->line_dynamic[cpuid]);
            BBL = (uint32_t)strtoul(this->line_dynamic[cpuid], NULL, 10);
            if (BBL != 0) {
                valid_dynamic = true;
            }
        }
    }
    return BBL;
};

// =============================================================================
void trace_reader_t::trace_next_memory(uint32_t cpuid) {
    bool valid_memory = false;

    this->line_memory[cpuid][0] = '\0';
    while (!valid_memory) {

        ERROR_ASSERT_PRINTF(!gzeof(this->gzDynamicTraceFile[cpuid]), "MemoryTraceFile EOF - cpu id %d\n", cpuid);
        char *buffer = gzgets(this->gzMemoryTraceFile[cpuid], this->line_memory[cpuid], TRACE_LINE_SIZE);
        ERROR_ASSERT_PRINTF(buffer != NULL, "MemoryTraceFile EOF - cpu id %d\n", cpuid);

        if (strlen(this->line_memory[cpuid]) != 0 && this->line_memory[cpuid][0] != '#') {
            valid_memory = true;
        }
    }
};

// =============================================================================
bool trace_reader_t::trace_fetch(uint32_t cpuid, opcode_package_t *m) {
    opcode_package_t NewOpcode;
    sync_t sync_found = SYNC_FREE;

    // =========================================================================
    /// Fetch new BBL inside the dynamic file.
    // =========================================================================
    if (!this->insideBBL[cpuid]) {
        uint32_t new_BBL = this->trace_next_dynamic(cpuid, &sync_found);
        if (new_BBL == FAIL) {
            if (sync_found == SYNC_FREE) {      /// EOF
                return FAIL;
            }
            else {                              /// SYNC
                /// SINUCA control variables.
                m->opcode_operation = INSTRUCTION_OPERATION_NOP;
                m->state = PACKAGE_STATE_UNTREATED;
                m->born_cycle = sinuca_engine.get_global_cycle();
                m->ready_cycle = sinuca_engine.get_global_cycle();
                m->opcode_number = trace_opcode_counter[cpuid];
                m->sync_type = sync_found;
                return OK;
            }
        }
        else {
            this->actual_bbl[cpuid] = new_BBL;
            this->actual_bbl_opcode[cpuid] = 0;
            this->insideBBL[cpuid] = true;
            ERROR_ASSERT_PRINTF(this->bbl_size[this->actual_bbl[cpuid]] != 0,
                            "Actual cpuid[%d] BBL [%d] size = %d.\n", cpuid, this->actual_bbl[cpuid], this->bbl_size[this->actual_bbl[cpuid]]);
            ERROR_ASSERT_PRINTF(this->actual_bbl[cpuid] != 0 &&
                            this->actual_bbl[cpuid] < this->total_bbls &&
                            this->bbl_size[this->actual_bbl[cpuid] > 0],
                            "Wrong Vector[%d] or Deque[%d] Position.\n", this->actual_bbl[cpuid], this->actual_bbl_opcode[cpuid]);
        }
    }

    // =========================================================================
    /// Fetch new INSTRUCTION inside the static file.
    // =========================================================================
    ERROR_ASSERT_PRINTF(this->actual_bbl[cpuid] != 0, "Vai dar merdar essa porra");
    NewOpcode = this->static_dict[this->actual_bbl[cpuid]][this->actual_bbl_opcode[cpuid]];
    // ~ printf("CPU:%u  BBL:%u  OPCODE:%u = %s\n",cpuid, this->actual_bbl[cpuid], this->actual_bbl_opcode[cpuid], NewOpcode.content_to_string().c_str());

    this->actual_bbl_opcode[cpuid]++;
    if (this->actual_bbl_opcode[cpuid] >= this->bbl_size[this->actual_bbl[cpuid]]) {
        this->insideBBL[cpuid] = false;
        this->actual_bbl_opcode[cpuid] = 0;
    }

    // =========================================================================
    /// Add SiNUCA information
    // =========================================================================
    *m = NewOpcode;
    /// SINUCA control variables.
    m->state = PACKAGE_STATE_UNTREATED;
    m->born_cycle = sinuca_engine.get_global_cycle();
    m->ready_cycle = sinuca_engine.get_global_cycle();
    m->opcode_number = trace_opcode_counter[cpuid];
    m->sync_type = sync_found;
    this->trace_opcode_counter[cpuid]++;

    /// Spawn Warmup
    if (trace_opcode_total == sinuca_engine.arg_warmup_instructions) {
        /// Next cycle all the statistics will be reset
        sinuca_engine.set_is_warm_up(true);
    }
    this->trace_opcode_total++;

    // =========================================================================
    /// If it is LOAD/STORE -> Fetch new MEMORY inside the memory file
    // =========================================================================
    if (m->is_read) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_read(this->line_memory[cpuid], this->actual_bbl[cpuid]);
    }

    if (m->is_read2) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_read2(this->line_memory[cpuid], this->actual_bbl[cpuid]);
    }

    if (m->is_write) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_write(this->line_memory[cpuid], this->actual_bbl[cpuid]);
    }

    TRACE_READER_DEBUG_PRINTF("CPU[%d] Found Operation [%s]. Found Memory [%s].\n", cpuid, m->content_to_string().c_str(), this->line_memory[cpuid]);
    return OK;
};

// =============================================================================
void trace_reader_t::generate_static_dict() {
    bool file_eof = false;
    uint32_t BBL = 0;                           /// Actual BBL (Index of the Vector)
    uint32_t opcode = 0;
    opcode_package_t NewOpcode;                 /// Actual Opcode

    gzclearerr(this->gzStaticTraceFile);
    gzseek(this->gzStaticTraceFile, 0, SEEK_SET);  /// Go to the Begin of the File
    file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
    ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")

    while (!file_eof) {
        gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
        file_eof = gzeof(this->gzStaticTraceFile);

        TRACE_READER_DEBUG_PRINTF("Read: %s\n", this->line_static);
        if (this->line_static[0] == '\0' || this->line_static[0] == '#') {     /// If Comment, then ignore
            continue;
        }
        else if (this->line_static[0] == '@') {                       /// If New BBL
            TRACE_READER_DEBUG_PRINTF("BBL %u with %u instructions.\n", BBL, opcode);

            opcode = 0;
            BBL = (uint32_t)strtoul(this->line_static + 1, NULL, 10);
            ERROR_ASSERT_PRINTF(BBL < this->total_bbls, "Static has more BBLs than previous analyzed static file.\n");
        }
        else {                                                  /// If Inside BBL
            NewOpcode.trace_string_to_opcode(this->line_static);
            ERROR_ASSERT_PRINTF(NewOpcode.opcode_address != 0, "Static trace file generating opcode address equal to zero.\n")
            this->static_dict[BBL][opcode++] = NewOpcode;                 /// Save the new opcode
        }
    }

    this->check_static_dict();
};


// =============================================================================
/// Check if the map was successfully generated
void trace_reader_t::check_static_dict() {
    bool file_eof = false;
    opcode_package_t trace_opcode, dict_opcode;  /// Actual Opcode

    gzclearerr(this->gzStaticTraceFile);
    gzseek(this->gzStaticTraceFile, 0, SEEK_SET);   /// Go to the Begin of the File
    file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
    ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")

    for (uint32_t bbl = 1; bbl < this->total_bbls; bbl++) {
        ERROR_ASSERT_PRINTF(this->bbl_size[bbl], "BBL[%d] has no instruction inside.\n", bbl);

        for (uint32_t j = 0; j < this->bbl_size[bbl]; j++) {
            dict_opcode = this->static_dict[bbl][j];

            while (true) {
                gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
                file_eof = gzeof(this->gzStaticTraceFile);
                ERROR_ASSERT_PRINTF(!file_eof, "Static File Smaller than StaticDict.\n")

                if (this->line_static[0] != '\0' && this->line_static[0] != '#' && this->line_static[0] != '@') {
                    break;
                }
            }
            trace_opcode.trace_string_to_opcode(this->line_static);
            dict_opcode.opcode_to_trace_string(this->line_dynamic[0]);
            ERROR_ASSERT_PRINTF(trace_opcode == dict_opcode,
                        "Input and Output Variable are different\n In:  [%s]\n Out: [%s]\n ", this->line_static, this->line_dynamic[0])
        }
    }

    file_eof = gzeof(this->gzStaticTraceFile);
    while (!file_eof) {
        gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
        file_eof = gzeof(this->gzStaticTraceFile);

        if (this->line_static[0] != '\0' && this->line_static[0] != '#' && this->line_static[0] != '@') {
            ERROR_ASSERT_PRINTF(file_eof, "Map is smaller than file.\n %s \n", this->line_static);
        }
    }
};

