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

/// Trace Reader
// ============================================================================
 /*! Read the instruction/memory trace files
  * Open the static trace => Dictionary of instruction per Basic Block
  * Open the dynamic trace(s) => Reads the threads Basic Block execution trace
  * Open the memory trace(s) => Match the memory addresses for R/W Instructions
  */
class trace_reader_t {
    private:
        gzFile gzStaticTraceFile;
        gzFile *gzDynamicTraceFile;
        gzFile *gzMemoryTraceFile;

        bool *insideBBL;
        uint64_t *trace_opcode_counter;
        uint64_t *trace_opcode_max;
        uint64_t trace_opcode_total;

        uint32_t total_bbls;
        uint32_t *bbl_size;

        uint32_t total_cores;

        opcode_package_t **static_dict;
        uint32_t *actual_bbl;
        uint32_t *actual_bbl_opcode;

        /// Used to handle the trace reader
        char *line_static;
        char **line_dynamic;
        char **line_memory;

    public:
        // ====================================================================
        /// Methods
        // ====================================================================
        trace_reader_t();
        ~trace_reader_t();
        void allocate(char *in_file, uint32_t ncpus);  /// must be called after the parameters are set properly
        inline const char* get_label() {
            return "TRACE_READER";
        };
        inline const char* get_type_component_label() {
            return "TRACE_READER";
        };

        void define_total_bbls();
        void define_total_bbl_size();


        uint64_t trace_size(uint32_t cpuid);
        uint32_t trace_next_dynamic(uint32_t cpuid, sync_t *sync_found);
        void trace_next_memory(uint32_t cpuid);
        bool trace_fetch(uint32_t cpuid, opcode_package_t *m);

        void generate_static_dict();
        void check_static_dict();

        /// Progress
        uint64_t get_trace_opcode_counter(uint32_t cpuid) { return this->trace_opcode_counter[cpuid]; }
        uint64_t get_trace_opcode_max(uint32_t cpuid) { return this->trace_opcode_max[cpuid]; }
        INSTANTIATE_GET_SET_ADD(uint64_t, trace_opcode_total);
};
