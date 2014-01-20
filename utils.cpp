//==============================================================================
//
// Copyright (C) 2010, 2011
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
#include <string>

//============================================================================== NEW
uint64_t mystrtol( char *&pen, uint64_t val = 0 ) {
    for ( char c; ( c = *pen ^ '0' ) <= 9; ++ pen ) val = val * 10 + c;
    return val;
}

//==============================================================================
void utils_t::get_path(char *path, char *file_path) {
    char *posp;

    strcpy (path, file_path);
    printf("PATH+FILE:%s\n", path);

    posp = strrchr(path, '/');
    if (posp == NULL) {
        strcpy (path, "./");
    }
    else {
        *(posp+1) = '\0';
    }
    printf("PATH:%s\n", path);
}

//==============================================================================
uint64_t utils_t::get_power_of_two(uint64_t n) {
    if (n == 0) {
        return 0;
    }

    ERROR_ASSERT_PRINTF(check_if_power_of_two(n), "Trying to get a log2 of a non power of two number %"PRIu64".\n", n);
    return log( n ) / log( 2 );
};

//==============================================================================
uint8_t utils_t::check_if_power_of_two(uint64_t n) {
    uint8_t i, count = 0;
    uint64_t pow;
    for (i = 0; i < 64 && n != 0 ; i++) {
        pow = n & (1 << i);
        count += (pow != 0);
        n -= pow;
    }

    if (count == 1) {
        return OK;
    }
    else {
        return FAIL;
    }
};
//==============================================================================
uint64_t utils_t::hash_function(hash_function_t type, uint64_t input1, uint64_t input2, uint64_t bit_size) {
    uint32_t i;
    uint64_t bit_size_mask = 0;
    uint64_t input1_bit_mask = 0;
    uint64_t input2_bit_mask = 0;
    uint64_t output = 0;

    for (i = 0; i < bit_size; i++) {
        bit_size_mask |= 1 << i;
    }

    switch (type) {
        case HASH_FUNCTION_XOR_SIMPLE:
            output = input1 ^ input2;
            output &= bit_size_mask;
        break;

        case HASH_FUNCTION_INPUT1_ONLY:
            output = input1 & bit_size_mask;
        break;

        case HASH_FUNCTION_INPUT2_ONLY:
            output = input2 & bit_size_mask;
        break;

        case HASH_FUNCTION_INPUT1_2BITS:
            ERROR_ASSERT_PRINTF(bit_size >= 2, "Bit size is smaller than the number of bits from input1.\n")
            for (i = 0; i < 2; i++) {
                input1_bit_mask |= (1 << i);
            }
            input1 &= input1_bit_mask;
            input1 = input1 << (bit_size - 2);

            for (i = 0; i < bit_size - 2; i++) {
                input2_bit_mask |= (1 << i);

            }
            input2 &= input2_bit_mask;

            /// Concatenate both inputs
            output = input1 | input2;
        break;

        case HASH_FUNCTION_INPUT1_4BITS:
            ERROR_ASSERT_PRINTF(bit_size >= 4, "Bit size is smaller than the number of bits from input1.\n")
            for (i = 0; i < 4; i++) {
                input1_bit_mask |= (1 << i);
            }
            input1 &= input1_bit_mask;
            input1 = input1 << (bit_size - 4);

            for (i = 0; i < bit_size - 4; i++) {
                input2_bit_mask |= (1 << i);

            }
            input2 &= input2_bit_mask;

            /// Concatenate both inputs
            output = input1 | input2;

        break;

        case HASH_FUNCTION_INPUT1_8BITS:
            ERROR_ASSERT_PRINTF(bit_size >= 8, "Bit size is smaller than the number of bits from input1.\n")
            for (i = 0; i < 8; i++) {
                input1_bit_mask |= (1 << i);
            }
            input1 &= input1_bit_mask;
            input1 = input1 << (bit_size - 8);

            for (i = 0; i < bit_size - 8; i++) {
                input2_bit_mask |= (1 << i);

            }
            input2 &= input2_bit_mask;

            /// Concatenate both inputs
            output = input1 | input2;
        break;

        case HASH_FUNCTION_INPUT1_16BITS:
            ERROR_ASSERT_PRINTF(bit_size >= 16, "Bit size is smaller than the number of bits from input1.\n")
            for (i = 0; i < 16; i++) {
                input1_bit_mask |= (1 << i);
            }
            input1 &= input1_bit_mask;
            input1 = input1 << (bit_size - 16);

            for (i = 0; i < bit_size - 16; i++) {
                input2_bit_mask |= (1 << i);

            }
            input2 &= input2_bit_mask;

            /// Concatenate both inputs
            output = input1 | input2;
        break;

        case HASH_FUNCTION_INPUT1_32BITS:
            ERROR_ASSERT_PRINTF(bit_size >= 32, "Bit size is smaller than the number of bits from input1.\n")
            for (i = 0; i < 32; i++) {
                input1_bit_mask |= (1 << i);
            }
            input1 &= input1_bit_mask;
            input1 = input1 << (bit_size - 32);

            for (i = 0; i < bit_size - 32; i++) {
                input2_bit_mask |= (1 << i);

            }
            input2 &= input2_bit_mask;

            /// Concatenate both inputs
            output = input1 | input2;
        break;
    }
    return output;
}

//==============================================================================
uint64_t utils_t::fill_bit(uint32_t start, uint32_t end) {
    uint32_t i;
    uint64_t r = 0;
    for (i = start ; i <= end ; i++)
        r |= 1 << i;
    return r;
};


//==============================================================================
std::string utils_t::address_to_binary(uint64_t address) {
    /// Auxiliar strings
    std::string answer;
    answer.clear();

    uint64_t z;
    uint32_t count = 0;

    for (z = (uint64_t)INT64_MAX+1; z > 0; z >>= 1) {
        if ((address & z) == z) {
            answer = answer + "1";
        }
        else {
            answer = answer + "0";
        }
        count++;
        if (count%4 == 0) {
            answer = answer + " ";
        }
    }
    return answer;
};

//==============================================================================
std::string utils_t::print_mask_of_bits(uint32_t line_size, uint32_t line_number, uint32_t assoc) {
    uint32_t i, offset = 0, index = 0;

    /// Auxiliar strings
    static char a[65];
    a[0] = '\0';

    offset = get_power_of_two(line_size);
    index = get_power_of_two(line_number/assoc) + offset;
    for (i = 0 ; i < 64 ; i++) {
        if (i < 32) {
            strcat(a , ".");
        }
        else if (i < 64-index) {
            strcat(a , "T");
        }
        else if (i < 64-offset) {
            strcat(a , "I");
        }
        else {
            strcat(a , "O");
        }
    }

    return a;
};

//==============================================================================
std::string utils_t::progress_pretty(uint64_t actual, uint64_t total) {
    /// Auxiliar strings
    std::string answer;
    answer.clear();

    for (uint32_t i = 1 ; i <= 20; i++) {
        if (i <= (uint64_t)20 * ((double)actual / (double)total)) {
            answer += "#";
        }
        else {
            answer += " ";
        }
    }
    answer = "[" + answer + "]";
    return answer;
};


//==============================================================================
std::string utils_t::connections_pretty(cache_memory_t *cache_memory, uint32_t level) {
    /// Auxiliar strings
    char tmp_string[CONVERSION_SIZE];
    std::string answer;
    answer.clear();

    for (uint32_t j = 0; j < level; j++) {
        answer += "|  ";
    }
    answer += "|--- ";
    answer += cache_memory->get_label();
    answer += " (L";
    uint32_to_char(tmp_string, cache_memory->get_hierarchy_level());
    answer += tmp_string;
    answer += ")\n";

    /// Find Next Cache Level
    container_ptr_cache_memory_t *higher_level_cache = cache_memory->get_higher_level_cache();
    for (uint32_t i = 0; i < higher_level_cache->size(); i++) {
        cache_memory_t *higher_cache = higher_level_cache[0][i];
        answer += connections_pretty(higher_cache, level + 1);
    }

    /// Find Processor
    if (higher_level_cache->empty()) {
        for (uint32_t i = 0; i < sinuca_engine.get_processor_array_size(); i++) {
            processor_t *processor = sinuca_engine.processor_array[i];
            if (processor->get_data_cache() == cache_memory || processor->get_inst_cache() == cache_memory) {
                for (uint32_t j = 0; j < level + 1; j++) {
                    answer += "|  ";
                }
                answer += "|--- ";
                answer += processor->get_label();
                answer += "\n";
            }
        }
    }

    return answer;
};

//==============================================================================
void utils_t::bool_to_char(char *string, bool input_int) {
    string[0] = '\0';
    sprintf(string , "%s", input_int ? "T" : "F");
}


//==============================================================================
void utils_t::uint32_to_char(char *string, uint32_t input_int) {
    string[0] = '\0';
    sprintf(string , "%2u", input_int);
}

//==============================================================================
void utils_t::int32_to_char(char *string, int32_t input_int) {
    string[0] = '\0';
    sprintf(string , "%2d", input_int);
}

//==============================================================================
void utils_t::uint64_to_char(char *string, uint64_t input_int) {
    string[0] = '\0';

    if (input_int > 1000000000) {
        sprintf(string , "%16"PRIu64"", input_int);
    }
    else {
        sprintf(string , "%5"PRIu64"", input_int);
    }
}

//==============================================================================
void utils_t::int64_to_char(char *string, int64_t input_int) {
    string[0] = '\0';

    if (input_int > 1000000000) {
        sprintf(string , "%16"PRId64"", input_int);
    }
    else {
        sprintf(string , "%5"PRId64"", input_int);
    }
}

//==============================================================================
uint64_t utils_t::string_to_uint64(char *string) {
    uint64_t result = 0;

    for (char c ; ( c = *string ^ '0' ) <= 9 && c >= 0; ++string) {
        result = result * 10 + c;
    }
    return result;
}


//==============================================================================
std::string  utils_t::bool_to_string(bool input_int) {
    /// Auxiliar strings
    char a[2];
    a[0] = '\0';

    std::string answer;
    answer.clear();

    a[0] = '\0';
    sprintf(a , "%s", input_int ? "T" : "F");
    answer = a;
    return answer;
}

//==============================================================================
std::string utils_t::uint32_to_string(uint32_t input_int) {
    /// Auxiliar strings
    char a[65];
    a[0] = '\0';

    std::string answer;
    answer.clear();

    sprintf(a , "%2u", input_int);
    answer = a;
    return answer;
};

//==============================================================================
std::string utils_t::int32_to_string(int32_t input_int) {
    /// Auxiliar strings
    char a[65];
    a[0] = '\0';

    std::string answer;
    answer.clear();

    sprintf(a , "%2d", input_int);
    answer = a;
    return answer;
};

//==============================================================================
std::string utils_t::uint64_to_string(uint64_t input_int) {
    char a[65];
    a[0] = '\0';
    std::string answer = "";

    if (input_int > 1000000000) {
        sprintf(a , "%16"PRIu64"", input_int);
        answer = a;
    }
    else {
        sprintf(a , "%5"PRIu64"", input_int);
        answer = a;
    }
    return answer;
};

//==============================================================================
std::string utils_t::int64_to_string(int64_t input_int) {
    char a[65];
    a[0] = '\0';
    std::string answer = "";

    if (input_int > 1000000000) {
        sprintf(a , "%16"PRId64"", input_int);
        answer = a;
    }
    else {
        sprintf(a , "%5"PRId64"", input_int);
        answer = a;
    }
    return answer;
};


//==============================================================================
/*! process_mem_usage(double &, double &) - takes two doubles by reference,
 * attempts to read the system-dependent data for a process' virtual memory
 * size and resident set size, and return the results in MB.
 * On failure, returns 0.0, 0.0
 */
void utils_t::process_mem_usage(double& vm_usage, double& resident_set) {
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   /// 'file' stat seems to give the most reliable results
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   /// dummy vars for leading entries in stat that we don't care about
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   /// the two fields we want
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; /// don't care about the rest

   stat_stream.close();

   double page_size_mb = sysconf(_SC_PAGE_SIZE) / 1024.0 / 1024.0; /// in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0 / 1024.0;
   resident_set = rss * page_size_mb;
};
