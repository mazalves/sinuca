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

#ifdef TRACE_READER_DEBUG
    #define TRACE_READER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_READER_DEBUG_PRINTF(...)
#endif

// =============================================================================
trace_reader_t::trace_reader_t() {
    this->DynamicTraceFile = NULL;
    this->MemoryTraceFile = NULL;

    this->gzStaticTraceFile = NULL;
    this->gzDynamicTraceFile = NULL;
    this->gzMemoryTraceFile = NULL;

    this->trace_opcode_total = 0;

    this->insideBBL = NULL;
    this->trace_opcode_counter = NULL;
    this->trace_opcode_max = NULL;

    this->actual_bbl = NULL;
    this->actual_bbl_opcode = NULL;

    this->is_compressed_trace_file = true;

    this->line_dynamic = NULL;
    this->line_static = NULL;
    this->line_memory = NULL;
};

// =============================================================================
trace_reader_t::~trace_reader_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<FILE*>(DynamicTraceFile);
    utils_t::template_delete_array<FILE*>(MemoryTraceFile);

    utils_t::template_delete_array<gzFile>(gzDynamicTraceFile);
    utils_t::template_delete_array<gzFile>(gzMemoryTraceFile);

    utils_t::template_delete_array<bool>(insideBBL);
    utils_t::template_delete_array<uint64_t>(trace_opcode_counter);
    utils_t::template_delete_array<uint64_t>(trace_opcode_max);

    utils_t::template_delete_array<uint32_t>(actual_bbl);
    utils_t::template_delete_array<uint32_t>(actual_bbl_opcode);

    utils_t::template_delete_array<char>(line_dynamic);
    utils_t::template_delete_array<char>(line_static);
    utils_t::template_delete_array<char>(line_memory);

    this->static_dictionary.clear();
};

// =============================================================================
void trace_reader_t::allocate(char *in_file, bool is_compact, uint32_t ncpus) {
    uint32_t i;
    this->is_compressed_trace_file = is_compact;

    this->line_dynamic = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    this->line_static = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);
    this->line_memory = utils_t::template_allocate_array<char>(TRACE_LINE_SIZE);

    strcpy(this->line_dynamic, "");
    strcpy(this->line_static, "");
    strcpy(this->line_memory, "");

    // =======================================================================
    // Static Trace File
    // =======================================================================
    char stat_file_name[500];
    stat_file_name[0] = '\0';

    TRACE_READER_DEBUG_PRINTF("Inserted Output File Name = %s\n", in_file);

    if (this->is_compressed_trace_file) {
        sprintf(stat_file_name , "%s.tid0.stat.out.gz", in_file);
        gzStaticTraceFile = gzopen(stat_file_name, "ro");   /// Open the .gz file
        ERROR_ASSERT_PRINTF(gzStaticTraceFile != NULL, "Could not open the file.\n%s\n", stat_file_name);
    }
    else {
        sprintf(stat_file_name , "%s.tid0.stat.out", in_file);
        this->StaticTraceFile = fopen(stat_file_name, "rw");
        ERROR_ASSERT_PRINTF(StaticTraceFile != NULL, "Could not open the file.\n%s\n", stat_file_name);
    }
    TRACE_READER_DEBUG_PRINTF("Static File = %s => READY !\n", stat_file_name);
    this->generate_static_dictionary();

    ERROR_ASSERT_PRINTF(!static_dictionary.empty(), "Generated dictionary with 0.\n");

    // =======================================================================
    // Dynamic Trace Files
    // =======================================================================
    char dyn_file_name[500];

    if (this->is_compressed_trace_file) {
        gzDynamicTraceFile = utils_t::template_allocate_array<gzFile>(ncpus);
        for (i = 0; i < ncpus; i++) {
            dyn_file_name[0] = '\0';
            sprintf(dyn_file_name , "%s.tid%d.dyn.out.gz", in_file, i);

            /// Create missing files in order to run single threaded app into a multi-core
            struct stat st;
            /// file do not exist
            if(stat(dyn_file_name, &st) != 0) {
                WARNING_PRINTF("FILE NOT FOUND %s. ", dyn_file_name);
                sprintf(dyn_file_name , "/tmp/NULL_tid%d.dyn.out.gz", i);
                WARNING_PRINTF("=> CREATED %s.\n", dyn_file_name);
                gzDynamicTraceFile[i] = gzopen(dyn_file_name, "wo");    /// Open the .gz file
                gzwrite(gzDynamicTraceFile[i], "#Empty Trace\n", strlen("#Empty Trace\n"));
                gzclose(gzDynamicTraceFile[i]);
            }

            gzDynamicTraceFile[i] = gzopen(dyn_file_name, "ro");    /// Open the .gz file
            ERROR_ASSERT_PRINTF(gzDynamicTraceFile[i] != NULL, "Could not open the file.\n%s\n", dyn_file_name);
            TRACE_READER_DEBUG_PRINTF("Dynamic File = %s => READY !\n", dyn_file_name);
        }
    }
    else {
        this->DynamicTraceFile = utils_t::template_allocate_array<FILE*>(ncpus);
        for (i = 0; i < ncpus; i++) {
            dyn_file_name[0] = '\0';
            sprintf(dyn_file_name ,  "%s.tid%d.dyn.out", in_file, i);

            /// Create missing files in order to run single threaded app into a multi-core
            struct stat st;
            /// file do not exist
            if(stat(dyn_file_name, &st) != 0) {
                WARNING_PRINTF("FILE NOT FOUND %s. ", dyn_file_name);
                sprintf(dyn_file_name , "/tmp/NULL_tid%d.dyn.out", i);
                WARNING_PRINTF("=> CREATED %s.\n", dyn_file_name);
                DynamicTraceFile[i] = fopen(dyn_file_name, "w");    /// Open the .out file
                ERROR_ASSERT_PRINTF(DynamicTraceFile[i] != NULL, "Could not create the fake file.\n%s\n", dyn_file_name);
                fprintf(DynamicTraceFile[i], "#Empty Trace\n");
                fclose(DynamicTraceFile[i]);
            }

            this->DynamicTraceFile[i] = fopen (dyn_file_name, "rw");
            ERROR_ASSERT_PRINTF(DynamicTraceFile[i] != NULL, "Could not open the file.\n%s\n", dyn_file_name);
            TRACE_READER_DEBUG_PRINTF("Dynamic File = %s => READY !\n", dyn_file_name);
        }
    }

    // =======================================================================
    // Memory Trace Files
    // =======================================================================
    char mem_file_name[500];

    if (this->is_compressed_trace_file) {
        gzMemoryTraceFile = utils_t::template_allocate_array<gzFile>(ncpus);
        for (i = 0; i < ncpus; i++) {
            mem_file_name[0] = '\0';
            sprintf(mem_file_name , "%s.tid%d.mem.out.gz", in_file, i);

            /// Create missing files in order to run single threaded app into a multi-core
            struct stat st;
            /// file do not exist
            if(stat(mem_file_name, &st) != 0) {
                WARNING_PRINTF("FILE NOT FOUND %s. ", mem_file_name);
                sprintf(mem_file_name , "/tmp/NULL_tid%d.mem.out.gz", i);
                WARNING_PRINTF("=> CREATED %s.\n", mem_file_name);
                gzMemoryTraceFile[i] = gzopen(mem_file_name, "wo");    /// Open the .gz file
                gzwrite(gzMemoryTraceFile[i], "#Empty Trace\n", strlen("#Empty Trace\n"));
                gzclose(gzMemoryTraceFile[i]);
            }

            gzMemoryTraceFile[i] = gzopen(mem_file_name, "ro");     /// Open the .gz file
            ERROR_ASSERT_PRINTF(gzMemoryTraceFile[i] != NULL, "Could not open the file.\n%s\n", mem_file_name);
            TRACE_READER_DEBUG_PRINTF("Memory File = %s => READY !\n", mem_file_name);
        }
    }
    else {
        this->MemoryTraceFile = utils_t::template_allocate_array<FILE*>(ncpus);
        for (i = 0; i < ncpus; i++) {
            mem_file_name[0] = '\0';
            sprintf(mem_file_name , "%s.tid%d.mem.out", in_file, i);

            /// Create missing files in order to run single threaded app into a multi-core
            struct stat st;
            /// file do not exist
            if(stat(mem_file_name, &st) != 0) {
                WARNING_PRINTF("FILE NOT FOUND %s. ", mem_file_name);
                sprintf(mem_file_name , "/tmp/NULL_tid%d.mem.out", i);
                WARNING_PRINTF("=> CREATED %s.\n", mem_file_name);
                MemoryTraceFile[i] = fopen(mem_file_name, "w");    /// Open the .out file
                ERROR_ASSERT_PRINTF(MemoryTraceFile[i] != NULL, "Could not create the fake file.\n%s\n", mem_file_name);
                fprintf(MemoryTraceFile[i], "#Empty Trace\n");
                fclose(MemoryTraceFile[i]);
            }

            this->MemoryTraceFile[i] = fopen(mem_file_name, "r");
            ERROR_ASSERT_PRINTF(MemoryTraceFile[i] != NULL, "Could not open the file.\n%s\n", mem_file_name);
            TRACE_READER_DEBUG_PRINTF("Memory File = %s => READY !\n", mem_file_name);
        }
    }

    this->insideBBL = utils_t::template_allocate_initialize_array<bool>(ncpus, false);
    this->trace_opcode_counter = utils_t::template_allocate_initialize_array<uint64_t>(ncpus, 1);
    this->trace_opcode_max = utils_t::template_allocate_array<uint64_t>(ncpus);
    for (i = 0; i < ncpus; i++) {
        this->trace_opcode_max[i] = this->trace_size(i);
    }

    this->actual_bbl = utils_t::template_allocate_initialize_array<uint32_t>(ncpus, 0);
    this->actual_bbl_opcode = utils_t::template_allocate_initialize_array<uint32_t>(ncpus, 0);
};

// =============================================================================
/// Get the total number of opcodes
uint64_t trace_reader_t::trace_size(uint32_t cpuid) {
    uint32_t BBL = 0;
    uint64_t trace_size = 0;
    this->line_dynamic[0] = '\0';
    bool trace_status = OK;

    while (trace_status) {
        if (is_compressed_trace_file) {
            if (gzeof(this->gzDynamicTraceFile[cpuid])) {
                trace_status = FAIL;
            }
            else {
                gzgets(this->gzDynamicTraceFile[cpuid], line_dynamic, TRACE_LINE_SIZE);
            }
        }
        else {
            if (feof(this->DynamicTraceFile[cpuid]) ||
            fgets(this->line_dynamic,  TRACE_LINE_SIZE,  this->DynamicTraceFile[cpuid]) == NULL) {
                trace_status = FAIL;
            }
        }
        if (strlen(this->line_dynamic) != 0 && this->line_dynamic[0] != '#' && this->line_dynamic[0] != '$') {
            BBL = (uint32_t)strtoul(this->line_dynamic, NULL, 10);
            trace_size += static_dictionary[BBL].size();
        }
    }

    /// Reset the Dynamic / Memory file positions
    if (is_compressed_trace_file) {
        gzclearerr(this->gzDynamicTraceFile[cpuid]);
        gzseek(this->gzDynamicTraceFile[cpuid], 0, SEEK_SET);

        gzclearerr(this->gzMemoryTraceFile[cpuid]);
        gzseek(this->gzMemoryTraceFile[cpuid], 0, SEEK_SET);
    }
    else {
        rewind(this->DynamicTraceFile[cpuid]);
        rewind(this->MemoryTraceFile[cpuid]);
    }

    return(trace_size);
};

// =============================================================================
void trace_reader_t::gzgetline(const gzFile& file, std::string& new_line) {
    char  line[TRACE_LINE_SIZE];
    gzgets(file, line, TRACE_LINE_SIZE);
    new_line = line;
};

// =============================================================================
uint32_t trace_reader_t::trace_next_dynamic(uint32_t cpuid, sync_t *new_sync) {
    uint32_t BBL = 0;

    /// Auxiliar strings
    this->line_dynamic[0] = '\0';

    bool valid_dynamic = false;
    sync_t &sync_found = *new_sync;

    while (!valid_dynamic) {
        if (is_compressed_trace_file) {
            if (gzeof(this->gzDynamicTraceFile[cpuid])) {
                sinuca_engine.set_is_processor_trace_eof(cpuid);
                return FAIL;
            }
            gzgets(this->gzDynamicTraceFile[cpuid], this->line_dynamic, TRACE_LINE_SIZE);
        }
        else {
            if (feof(this->DynamicTraceFile[cpuid]) ||
            fgets(this->line_dynamic, TRACE_LINE_SIZE, this->DynamicTraceFile[cpuid]) == NULL) {
                sinuca_engine.set_is_processor_trace_eof(cpuid);
                return FAIL;
            }
        }

        if (strlen(this->line_dynamic) == 0) {
            continue;
        }
        else if (this->line_dynamic[0] == '#') {
            TRACE_READER_DEBUG_PRINTF("cpu[%d] - %s\n", cpuid, line_dynamic);
            continue;
        }
        else if (this->line_dynamic[0] == '$') {
            sync_found = (sync_t)strtoul(this->line_dynamic + 1, NULL, 10);
            return FAIL;
        }
        else {
            /// BBL is always greater than 0
            /// If strtoul==0 the line could not be converted.
            TRACE_READER_DEBUG_PRINTF("cpu[%d] BBL = %s\n", cpuid, this->line_dynamic);
            BBL = (uint32_t)strtoul(this->line_dynamic, NULL, 10);
            if (BBL != 0) {
                valid_dynamic = true;
            }
        }
    }
    return BBL;
};

// =============================================================================
void trace_reader_t::trace_next_memory(uint32_t cpuid) {

    this->line_memory[0] = '\0';

    bool valid_memory = false;
    bool eof = false;

    while (!valid_memory) {
        if (is_compressed_trace_file) {
            ERROR_ASSERT_PRINTF(!gzeof(this->gzDynamicTraceFile[cpuid]), "MemoryTraceFile EOF - cpu id %d\n", cpuid);
            gzgets(this->gzMemoryTraceFile[cpuid], this->line_memory, TRACE_LINE_SIZE);

        }
        else {
            eof = fgets(this->line_memory, TRACE_LINE_SIZE, this->MemoryTraceFile[cpuid]) == NULL;
            ERROR_ASSERT_PRINTF(!feof(this->MemoryTraceFile[cpuid]) && eof == false, "MemoryTraceFile EOF - cpu id %d\n", cpuid);
        }

        if (strlen(this->line_memory) != 0 && this->line_memory[0] != '#') {
            valid_memory = true;
        }
    }
};

// =============================================================================
bool trace_reader_t::trace_fetch(uint32_t cpuid, opcode_package_t *m) {

    opcode_package_t NewOpcode;
    sync_t sync_found = SYNC_FREE;

    //==========================================================================
    /// Fetch new BBL inside the dynamic file.
    //==========================================================================
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
            ERROR_ASSERT_PRINTF(static_dictionary[this->actual_bbl[cpuid]].size() != 0,
                                "Actual cpuid[%d] BBL [%d] size = %d.\n", cpuid, this->actual_bbl[cpuid], uint32_t(static_dictionary[this->actual_bbl[cpuid]].size()));
            ERROR_ASSERT_PRINTF(this->actual_bbl[cpuid] != 0 &&
                                this->actual_bbl[cpuid] < this->static_dictionary.size() &&
                                this->actual_bbl_opcode[cpuid] < static_dictionary[this->actual_bbl[cpuid]].size(),
                                "Wrong Vector[%d] or Deque[%d] Position.\n", this->actual_bbl[cpuid], this->actual_bbl_opcode[cpuid]);
        }
    }


    //==========================================================================
    /// Fetch new INSTRUCTION inside the static file.
    //==========================================================================
    NewOpcode = this->static_dictionary[this->actual_bbl[cpuid]][this->actual_bbl_opcode[cpuid]];
    uint32_t deque_size = static_dictionary[this->actual_bbl[cpuid]].size();
    this->actual_bbl_opcode[cpuid]++;
    if (this->actual_bbl_opcode[cpuid] >= deque_size) {
        this->insideBBL[cpuid] = false;
        this->actual_bbl_opcode[cpuid] = 0;
    }


    //==========================================================================
    /// Add SiNUCA information
    //==========================================================================
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

    //==========================================================================
    /// If it is LOAD/STORE -> Fetch new MEMORY inside the memory file
    //==========================================================================
    if (m->is_read) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_read(this->line_memory, this->actual_bbl[cpuid]);
    }

    if (m->is_read2) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_read2(this->line_memory, this->actual_bbl[cpuid]);
    }

    if (m->is_write) {
        this->trace_next_memory(cpuid);
        m->trace_string_to_write(this->line_memory, this->actual_bbl[cpuid]);
    }

    TRACE_READER_DEBUG_PRINTF("CPU[%d] Found Operation [%s]. Found Memory [%s].\n", cpuid, m->content_to_string(), this->line_memory);
    return OK;
};

// =============================================================================
void trace_reader_t::generate_static_dictionary() {
    bool file_eof = false;
    static_dictionary.clear();                  /// Vector[BBL] => Deques of Opcodes

    container_opcode_package_t NewBBL;          /// Deques of Opcodes
    NewBBL.clear();

    uint32_t BBL = 0;                           /// Actual BBL (Index of the Vector)
    opcode_package_t NewOpcode;                 /// Actual Opcode

    if (this->is_compressed_trace_file) {
        gzclearerr(this->gzStaticTraceFile);
        gzseek(this->gzStaticTraceFile, 0, SEEK_SET);  /// Go to the Begin of the File
        file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
        ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")
    }
    else {
        rewind(this->StaticTraceFile);
        file_eof = feof(this->StaticTraceFile);         /// Check is file not EOF
        ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")
    }

    while (!file_eof) {
        this->line_static[0] = '\0';
        this->line_dynamic[0] = '\0';

        if (this->is_compressed_trace_file) {
            gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
            file_eof = gzeof(this->gzStaticTraceFile);
        }
        else {
            file_eof = fgets(line_static, TRACE_LINE_SIZE, this->StaticTraceFile) == NULL;
            file_eof = file_eof || feof(this->StaticTraceFile);
        }
        TRACE_READER_DEBUG_PRINTF("Read: %s\n", line_static);
        if (strlen(this->line_static) == 0 || this->line_static[0] == '#') {     /// If Comment, then ignore
            continue;
        }
        else if (this->line_static[0] == '@') {                       /// If New BBL
            ERROR_ASSERT_PRINTF(BBL == (uint32_t)this->static_dictionary.size(), "Static dictionary has more BBLs than static file.\n");
            TRACE_READER_DEBUG_PRINTF("BBL %u with %u instructions.\n",BBL, (uint32_t)NewBBL.size());
            for (uint32_t i = 0; i < NewBBL.size(); i++){
                TRACE_READER_DEBUG_PRINTF("\t%s", NewBBL[i].opcode_to_trace_string());
            }

            ERROR_ASSERT_PRINTF(BBL < (uint32_t)this->static_dictionary.max_size(), "Static file has less BBLs than dynamic file.\n");
            this->static_dictionary.push_back(NewBBL);  /// Save the Deque of Opcodes
            NewBBL.clear();

            BBL = (uint32_t)strtoul(this->line_static + 1, NULL, 10);
        }
        else {                                                  /// If Inside BBL
            NewOpcode.trace_string_to_opcode(this->line_static);
            NewBBL.push_back(NewOpcode);                        /// Insert the Opcode into the Deque
        }
    }
    TRACE_READER_DEBUG_PRINTF("BBL %u with %u instructions.\n",BBL, (uint32_t)NewBBL.size());
    this->static_dictionary.push_back(NewBBL);                  /// Save the Last Deque into the Vector
    this->check_static_dictionary();
};

// =============================================================================
/// Check if the map was successfully generated
void trace_reader_t::check_static_dictionary() {
    bool file_eof = false;
    opcode_package_t trace_opcode, dict_opcode;  /// Actual Opcode

    if (is_compressed_trace_file) {
        gzclearerr(this->gzStaticTraceFile);
        gzseek(this->gzStaticTraceFile, 0, SEEK_SET);   /// Go to the Begin of the File
        file_eof = gzeof(this->gzStaticTraceFile);      /// Check is file not EOF
        ERROR_ASSERT_PRINTF(!file_eof, "Static File Unexpected EOF.\n")
    }
    else {
        rewind(this->StaticTraceFile);
        file_eof = feof(this->StaticTraceFile);         /// Check is file not EOF
        ERROR_ASSERT_PRINTF(!file_eof && this->StaticTraceFile != NULL, "Static File Unexpected EOF.\n")
    }

    bool valid = false;
    uint32_t vector_size = this->static_dictionary.size();
    uint32_t deque_size;

    for (uint32_t i = 0; i < vector_size; i++) {
        deque_size  = static_dictionary[i].size();
        ERROR_ASSERT_PRINTF(i == 0 || deque_size != 0, "BBL[%d] has no instruction inside.\n", i);
        for (uint32_t j = 0; j < deque_size; j++) {
            dict_opcode = this->static_dictionary[i][j];
            valid = false;
            while (!valid) {
                if (is_compressed_trace_file) {
                    gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
                    file_eof = gzeof(this->gzStaticTraceFile);
                }
                else {
                    file_eof = fgets(this->line_static, TRACE_LINE_SIZE, this->StaticTraceFile) == NULL;   /// Get Line
                    file_eof = file_eof || feof(this->StaticTraceFile);
                }
                ERROR_ASSERT_PRINTF(!file_eof, "Static File Smaller than Map.\n")

                if (strlen(this->line_static) != 0 && this->line_static[0] != '#' && this->line_static[0] != '@') {
                    valid = true;
                }
            }
            trace_opcode.trace_string_to_opcode(this->line_static);
            dict_opcode.opcode_to_trace_string(this->line_dynamic);
            ERROR_ASSERT_PRINTF(trace_opcode == dict_opcode,
                        "Input and Output Variable are different\n In:  [%s]\n Out: [%s]\n ", this->line_static, this->line_dynamic)
        }
    }

    if (is_compressed_trace_file) {
        file_eof = gzeof(this->gzStaticTraceFile);
    }
    else {
        file_eof = feof(this->StaticTraceFile);
    }

    valid = false;
    while (!valid) {
        if (file_eof) {
            if (is_compressed_trace_file) {
                gzgets(this->gzStaticTraceFile, this->line_static, TRACE_LINE_SIZE);
                file_eof = gzeof(this->gzStaticTraceFile);
            }
            else {
                file_eof = fgets(this->line_static, TRACE_LINE_SIZE, this->StaticTraceFile) == NULL;   /// Get Line
                file_eof = file_eof || feof(this->StaticTraceFile);
            }

            if (strlen(this->line_static) != 0 && this->line_static[0] != '#' && this->line_static[0] != '@') {
                ERROR_ASSERT_PRINTF(file_eof, "Map is smaller than file.\n %s \n", this->line_static);
            }
        }
        else {
            valid = true;
        }
    }
};

