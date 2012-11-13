import sys
import os
import subprocess
import stat
import re

################################################################################
## Print the GNUPLOT header
################################################################################
def gnulot_base(TMP_GNU_FILE_NAME):
    TMP_GNU_FILE = open(TMP_GNU_FILE_NAME, 'w')
    TMP_GNU_FILE.write("reset\n")
    ##########################
    # Output
    TMP_GNU_FILE.write("set terminal jpeg medium size 1280,1024 font Helvetica 16\n")

    ##########################
    # Scale
    TMP_GNU_FILE.write("set autoscale x\n")
    TMP_GNU_FILE.write("set autoscale y\n")

    TMP_GNU_FILE.write("set xtics nomirror rotate by -90 \n")

    ##########################
    # Colors
    TMP_GNU_FILE.write("red1 = '#F9B0B0' \n")
    TMP_GNU_FILE.write("red2 = '#F96D6D' \n")
    TMP_GNU_FILE.write("red3 = '#E61717' \n")
    TMP_GNU_FILE.write("red4 = '#8F3F3F' \n")
    TMP_GNU_FILE.write("red5 = '#6D0303' \n")

    TMP_GNU_FILE.write("orange1 = '#F9C5B0' \n")
    TMP_GNU_FILE.write("orange2 = '#F9956D' \n")
    TMP_GNU_FILE.write("orange3 = '#E65217' \n")
    TMP_GNU_FILE.write("orange4 = '#8F563F' \n")
    TMP_GNU_FILE.write("orange5 = '#6D2103' \n")

    TMP_GNU_FILE.write("yellow1 = '#F9ECB0' \n")
    TMP_GNU_FILE.write("yellow2 = '#F9E16D' \n")
    TMP_GNU_FILE.write("yellow3 = '#E6C217' \n")
    TMP_GNU_FILE.write("yellow4 = '#8F823F' \n")
    TMP_GNU_FILE.write("yellow5 = '#6D5B03' \n")

    TMP_GNU_FILE.write("green1 = '#A8EDA8' \n")
    TMP_GNU_FILE.write("green2 = '#68ED68' \n")
    TMP_GNU_FILE.write("green3 = '#12B812' \n")
    TMP_GNU_FILE.write("green4 = '#327332' \n")
    TMP_GNU_FILE.write("green5 = '#025702' \n")

    TMP_GNU_FILE.write("cian1 = '#A0E2E2' \n")
    TMP_GNU_FILE.write("cian2 = '#63E2E2' \n")
    TMP_GNU_FILE.write("cian3 = '#0E8A8A' \n")
    TMP_GNU_FILE.write("cian4 = '#265656' \n")
    TMP_GNU_FILE.write("cian5 = '#024141' \n")

    TMP_GNU_FILE.write("blue1 = '#AABCE6' \n")
    TMP_GNU_FILE.write("blue2 = '#7295E6' \n")
    TMP_GNU_FILE.write("blue3 = '#1E439A' \n")
    TMP_GNU_FILE.write("blue4 = '#303E60' \n")
    TMP_GNU_FILE.write("blue5 = '#041949' \n")

    TMP_GNU_FILE.write("pink1 = '#EDA8CF' \n")
    TMP_GNU_FILE.write("pink2 = '#ED68B3' \n")
    TMP_GNU_FILE.write("pink3 = '#B81270' \n")
    TMP_GNU_FILE.write("pink4 = '#733257' \n")
    TMP_GNU_FILE.write("pink5 = '#570232' \n")

    ##########################
    # Subtitle
    TMP_GNU_FILE.write("set key autotitle columnheader\n")
    TMP_GNU_FILE.write("set key outside below center horizontal\n")

    TMP_GNU_FILE.close()
    return
########################################################################

################################################################################
# This prints a passed string into this function
################################################################################
def PRINT( str ):
   print "PYTHON: " + str
   return

########################################################################
def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

########################################################################
def cacti_base_cfg(TMP_CFG_FILE_NAME):
    TMP_CFG_FILE = open(TMP_CFG_FILE_NAME, 'a')

    # following three parameters are meaningful only for main memories
    TMP_CFG_FILE.write("-page size (bits) 8192  \n")
    TMP_CFG_FILE.write("-burst length 8 \n")
    TMP_CFG_FILE.write("-internal prefetch width 8 \n")

    # following parameter can have one of three values -- (itrs-hp, itrs-lstp, itrs-lop)
    TMP_CFG_FILE.write("-Data array peripheral type - \"itrs-hp\" \n")

    # following parameter can have one of three values -- (itrs-hp, itrs-lstp, itrs-lop)
    TMP_CFG_FILE.write("-Tag array peripheral type - \"itrs-hp\" \n")

    # 300-400 in steps of 10
    TMP_CFG_FILE.write("-operating temperature (K) 350 \n")

    # DESIGN OBJECTIVE for UCA (or banks in NUCA)
    TMP_CFG_FILE.write("-design objective (weight delay, dynamic power, leakage power, cycle time, area) 0:0:0:0:100 \n")

    # Percentage deviation from the minimum value
    # Ex: A deviation value of 10:1000:1000:1000:1000 will try to find an organization
    # that compromises at most 10% delay.
    TMP_CFG_FILE.write("-deviate (delay, dynamic power, leakage power, cycle time, area) 60:100000:100000:100000:1000000 \n")

    # Objective for NUCA
    TMP_CFG_FILE.write("-NUCAdesign objective (weight delay, dynamic power, leakage power, cycle time, area) 100:100:0:0:100 \n")
    TMP_CFG_FILE.write("-NUCAdeviate (delay, dynamic power, leakage power, cycle time, area) 10:10000:10000:10000:10000 \n")

    # Set optimize tag to ED or ED^2 to obtain a cache configuration optimized for
    # energy-delay or energy-delay sq. product
    # Note: Optimize tag will disable weight or deviate values mentioned above
    # Set it to NONE to let weight and deviate values determine the
    # appropriate cache configuration
    TMP_CFG_FILE.write("-Optimize ED or ED^2 (ED, ED^2, NONE): \"NONE\" \n")

    # In order for CACTI to find the optimal NUCA bank value the following
    # variable should be assigned 0.
    TMP_CFG_FILE.write("-NUCA bank count 0 \n")

    # NOTE: for nuca network frequency is set to a default value of
    # 5GHz in time.c. CACTI automatically
    # calculates the maximum possible frequency and downgrades this value if necessary

    # By default CACTI considers both full-swing and low-swing
    # wires to find an optimal configuration. However, it is possible to
    # restrict the search space by changing the signalling from "default" to
    # "fullswing" or "lowswing" type.
    TMP_CFG_FILE.write("-Wire signalling (fullswing, lowswing, default) - \"Global_10\" \n")

    # Wire inside the mat "global" or "semi-global"
    TMP_CFG_FILE.write("-Wire inside mat - \"global\" \n")
    # Wire outside the mat
    TMP_CFG_FILE.write("-Wire outside mat - \"global\" \n")

    # Interconnection project type "conservative" or "aggressive"
    TMP_CFG_FILE.write("-Interconnect projection - \"conservative\" \n")

    # Contention in network (which is a function of core count and cache level) is one of
    # the critical factor used for deciding the optimal bank count value
    # core count can be 4, 8, or 16
    TMP_CFG_FILE.write("-Core count 8 \n")

    # Enable ECC error checking
    TMP_CFG_FILE.write("-Add ECC - \"true\" \n")

    # Output detail level "DETAILED" or "CONCISE"
    TMP_CFG_FILE.write("-Print level (DETAILED, CONCISE) - \"DETAILED\" \n")

    # for debugging ("true" or "false")
    TMP_CFG_FILE.write("-Print input parameters - \"true\" \n")

    # force CACTI to model the cache with the
    # following Ndbl, Ndwl, Nspd, Ndsam,
    # and Ndcm values
    TMP_CFG_FILE.write("-Force cache config - \"false\" \n")
    TMP_CFG_FILE.write("-Ndwl 64 \n")
    TMP_CFG_FILE.write("-Ndbl 64 \n")
    TMP_CFG_FILE.write("-Nspd 64 \n")
    TMP_CFG_FILE.write("-Ndcm 1 \n")
    TMP_CFG_FILE.write("-Ndsam1 4 \n")
    TMP_CFG_FILE.write("-Ndsam2 1 \n")

    TMP_CFG_FILE.close()
    return
########################################################################

########################################################################
def cacti_defined_cfg(TMP_CFG_FILE_NAME, CacheLevel, CacheSize, LineSize, CacheAssoc, IO_Output, TechInt, BankCount, CacheType, TAG, UCA, Access, DataArray, TagArray, RW_Port, R_Port, W_Port):
    TMP_CFG_FILE = open(TMP_CFG_FILE_NAME, 'w')

    TMP_CFG_FILE.write("####################################################\n")
    # Cache Level
    TMP_CFG_FILE.write("-Cache level (L2/L3) - \"L"+str(CacheLevel)+"\" \n")
    ### Cache size
    TMP_CFG_FILE.write("-size (bytes) "+str(CacheSize)+" \n")
    ### Line size
    TMP_CFG_FILE.write("-block size (bytes) "+str(LineSize)+" \n")
    ### To model Fully Associative cache, set associativity to zero
    TMP_CFG_FILE.write("-associativity "+str(CacheAssoc)+" \n")
    ### Multiple banks connected using a bus
    TMP_CFG_FILE.write("-UCA bank count "+str(BankCount)+" \n")
    ### Integration Technology -- (0.032, 0.045, 0.068, 0.090)
    TMP_CFG_FILE.write("-technology (u) "+str(TechInt)+" \n")
    ### Bus width include data bits and address bits required by the decoder
    TMP_CFG_FILE.write("-output/input bus width "+str(IO_Output)+" \n")
    # Type of memory - cache (with a tag array) or ram (scratch ram similar to a register file)
    # or main memory (no tag array and every access will happen at a page granularity Ref: CACTI 5.3 report)
    TMP_CFG_FILE.write("-cache type \""+CacheType+"\" \n")
    # to model special structure like branch target buffers, directory, etc.
    # change the tag size parameter if you want cacti to calculate the tagbits, set the tag size to "default"
    #//-tag size (b) "default"
    TMP_CFG_FILE.write("-tag size (b) "+str(TAG)+" \n")# 42(Normal) +8(Sectors)
    # Normal "UCA" or multi-banked "NUCA" cache.
    TMP_CFG_FILE.write("-Cache model (NUCA, UCA)  - \""+str(UCA)+"\" \n")
    # fast - data and tag access happen in parallel
    # sequential - data array is accessed after accessing the tag array
    # normal - data array lookup and tag access happen in parallel
    #          final data block is broadcasted in data array h-tree
    #          after getting the signal from the tag array
    TMP_CFG_FILE.write("-access mode (normal, sequential, fast) - \""+Access+"\" \n")
    # following parameter can have one of five values -- (itrs-hp, itrs-lstp, itrs-lop, lp-dram, comm-dram)
    #TMP_CFG_FILE.write("-Data array cell type - \"comm-dram\" \n")
    TMP_CFG_FILE.write("-Data array cell type - \""+DataArray+"\" \n")
    # following parameter can have one of five values -- (itrs-hp, itrs-lstp, itrs-lop, lp-dram, comm-dram)
    TMP_CFG_FILE.write("-Tag array cell type - \""+TagArray+"\" \n")
    # Input and Output ports
    TMP_CFG_FILE.write("-read-write port "+str(RW_Port)+" \n")
    TMP_CFG_FILE.write("-exclusive read port "+str(R_Port)+" \n")
    TMP_CFG_FILE.write("-exclusive write port "+str(W_Port)+" \n")
    TMP_CFG_FILE.write("-single ended read ports 0 \n")

    TMP_CFG_FILE.write("####################################################\n")
    TMP_CFG_FILE.close()
    return
########################################################################


################################################################################
BENCHMARK_LIST = ["spec_cpu2000", "spec_cpu2006", "spec_omp2001", "npb_omp"]

if (len(sys.argv) < 4) or (sys.argv[1] not in BENCHMARK_LIST):
    sys.exit("Usage: python plot.py benchmark base_input_name base_output_name")
else :
    arg_benchmark =  sys.argv[1]
    arg_input_results_filename = sys.argv[2]
    arg_output_results_filename = sys.argv[3]

# PROJECT_HOME with:
#   |
#   |- benchmarks
#   |   |- pin_points
#   |   |- traces
#   |   |- profiles
#   |   |- results
#   |   |- plots
#   |   |- src
#   |       |- spec_cpu2000
#   |       |- spec_cpu2006
#   |       |- spec_omp2001
#   |       |- npb_omp
#   |
#   |- SiNUCA
#
USER = "mazalves"
PROJECT_HOME = "/home/" + USER + "/Experiment/"
PRINT("PROJECT_HOME = " + PROJECT_HOME)

SINUCA_HOME = PROJECT_HOME + "/SiNUCA/"
PRINT("SINUCA_HOME = " + SINUCA_HOME)
os.putenv("SINUCA_HOME", SINUCA_HOME)

PLOTS_HOME = PROJECT_HOME + "benchmarks/plots/"
os.system("mkdir -p " + PLOTS_HOME)
arg_gnuplot_filename = PLOTS_HOME + "Energy_" + arg_output_results_filename + ".gnuplot"
arg_output_results_filename = PLOTS_HOME + "Energy_" + arg_output_results_filename + ".data"


## Model all CACTI_CONFIGURATIONS on Cacti 6.5
CACTI_HOME = PROJECT_HOME + "/cacti65/"
CACTI_MODEL_CFG_DIR = CACTI_HOME + "/models_cfg/"
CACTI_MODEL_OUTPUT_DIR = CACTI_HOME + "/models_out/"
os.system("mkdir -p " + CACTI_MODEL_CFG_DIR)
os.system("mkdir -p " + CACTI_MODEL_OUTPUT_DIR)
# ~ os.system("rm -f " + CACTI_MODEL_CFG_DIR + "*")
# ~ os.system("rm -f " + CACTI_MODEL_OUTPUT_DIR + "*")
PRINT("CACTI_MODEL_CFG_DIR = " + CACTI_MODEL_CFG_DIR)
PRINT("CACTI_MODEL_OUTPUT_DIR = " + CACTI_MODEL_OUTPUT_DIR)

if arg_benchmark == 'spec_cpu2000':
    app_list_filename   = SINUCA_HOME + "/scripts/command_to_run_spec_cpu2000.txt"
    results_directory       = PROJECT_HOME + "/benchmarks/results/spec_cpu2000/"
elif arg_benchmark == 'spec_cpu2006':
    app_list_filename   = SINUCA_HOME + "/scripts/command_to_run_spec_cpu2006.txt"
    results_directory       = PROJECT_HOME + "/benchmarks/results/spec_cpu2006/"
elif arg_benchmark == 'spec_omp2001':
    app_list_filename   = SINUCA_HOME + "/scripts/command_to_run_spec_omp2001.txt"
    results_directory       = PROJECT_HOME + "/benchmarks/results/spec_omp2001/"
elif arg_benchmark == 'npb_omp':
    app_list_filename   = SINUCA_HOME + "/scripts/command_to_run_npb_omp.txt"
    results_directory       = PROJECT_HOME + "/benchmarks/results/npb_omp/"


PRINT("\t Benchmark:" + arg_benchmark)
PRINT("\t Input Results Filename:" + str(arg_input_results_filename))
PRINT("\t Output Data Filename:" + arg_output_results_filename)
PRINT("\t App List Filename = " + app_list_filename)
PRINT("\t Results Directory = " + results_directory)

# Open the output filename
has_header = 0
header = ""
output_results_file = open(arg_output_results_filename, 'w')

# Iterates over the APPLICATIONS name from benchmark
try:
    app_list_file = open(app_list_filename, 'r')
except IOError:
    PRINT("\t \t app_list_filename Not Found:" + arg_benchmark + " - File Not Found:" + app_list_filename + " ... skipping")
    exit()
for app_list_file_line in app_list_file:
    configurations_inside_file = 0
    app_list_file_line = app_list_file_line.rstrip('\r\n')
    if  app_list_file_line[0] == '#':
        continue

    split_app_list_file_line = app_list_file_line.split(';')
    app_name = split_app_list_file_line[0] + "." + split_app_list_file_line[1]
    app_file_name = results_directory + split_app_list_file_line[2] + "." + arg_input_results_filename + ".result"
    try:
        app_file = open(app_file_name, 'r')
    except IOError:
        PRINT("\t \t Application Not Found:" + app_name + " - File Not Found:" + app_file_name + " ... skipping")
        continue
    # ~ PRINT("\t Analyzing:" + app_file_name)

    ############################################################################
    ### Find Cache Names
    ############################################################################
    cache_label_list = []

    # Iterates over the application results FILE
    app_file.seek(0,0)
    for parameter_line in app_file:
        parameter_line = parameter_line.rstrip('\r\n')
        #Comments
        if (parameter_line == "#Configuration of SINUCA_ENGINE"):
            configurations_inside_file += 1
            if (configurations_inside_file > 1):
                exit("Multiple results for application:" + app_name + " File:" + app_file_name)

        if parameter_line[0] == '#':
            continue

        split_parameter_line = parameter_line.split('.')
        if (split_parameter_line[0] == "CACHE_MEMORY") and (split_parameter_line[1] not in cache_label_list):
            cache_label_list.append(split_parameter_line[1])




    ############################################################################
    ### Find Parameters/Statistics for each Cache Name
    ############################################################################
    cache_global_cycle_list = []
    cache_reset_cycle_list = []

    cache_hierarchy_level_list = []
    cache_line_size_list = []
    cache_line_number_list = []
    cache_associativity_list = []

    cache_line_usage_predictor_type_list = []
    cache_DSBP_usage_counter_bits_list = []
    cache_DSBP_sub_block_size_list = []

    cache_DSBP_PHT_line_number_list = []
    cache_DSBP_PHT_associativity_list = []

    cache_stat_accesses_list = []
    cache_stat_DSBP_PHT_access_list = []
    cache_stat_active_sub_block_per_access_list = []
    cache_stat_active_sub_block_per_cycle_list = []

    cache_label_number = -1
    for cache_label in cache_label_list:
        cache_label_number += 1
        ########################################################################
        # FIND PARAMETERS
        # Iterates over the application results FILE
        app_file.seek(0,0)
        for parameter_line in app_file:
            parameter_line = parameter_line.rstrip('\r\n')
            #Comments
            if parameter_line[0] == '#':
                continue
            split_parameter_line = re.split('\.|:', parameter_line)
            ## Find Cache Parameters
            if (split_parameter_line[0] == "CACHE_MEMORY") and (split_parameter_line[1] == cache_label):
                if (split_parameter_line[2] == "hierarchy_level") :
                    cache_hierarchy_level_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "line_size") :
                    cache_line_size_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "line_number") :
                    cache_line_number_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "associativity") :
                    cache_associativity_list.append(int(split_parameter_line[3]))

            ## Find Line Usage Predictor Parameters
            elif (split_parameter_line[0] == "LINE_USAGE_PREDICTOR") and (split_parameter_line[1] == "Line_Usage_Predictor_" + cache_label):
                if (split_parameter_line[2] == "line_usage_predictor_type") :
                    cache_line_usage_predictor_type_list.append(split_parameter_line[3])
                elif (split_parameter_line[2] == "DSBP_usage_counter_bits") :
                    cache_DSBP_usage_counter_bits_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "DSBP_sub_block_size") :
                    cache_DSBP_sub_block_size_list.append(int(split_parameter_line[3]))
                ## Find PHT Parameters
                elif (split_parameter_line[2] == "DSBP_PHT_line_number") :
                    cache_DSBP_PHT_line_number_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "DSBP_PHT_associativity") :
                    cache_DSBP_PHT_associativity_list.append(int(split_parameter_line[3]))

        ########################################################################
        # FIND STATISTICS
        # Iterates over the application results FILE

        line_size = cache_line_size_list[cache_label_number]
        DSBP_sub_block_size = cache_DSBP_sub_block_size_list[cache_label_number]

        cache_stat_active_sub_block_per_access_list.append([])
        cache_stat_active_sub_block_per_cycle_list.append([])

        for i in range(0, 1+line_size) :
            cache_stat_active_sub_block_per_access_list[cache_label_number].append(0)
            cache_stat_active_sub_block_per_cycle_list[cache_label_number].append(0)

        app_file.seek(0,0)
        for parameter_line in app_file:
            parameter_line = parameter_line.rstrip('\r\n')
            #Comments
            if parameter_line[0] == '#':
                continue
            split_parameter_line = re.split('\.|:', parameter_line)
            ## Find Global Parameters
            if (split_parameter_line[0] == "SINUCA_ENGINE") and (split_parameter_line[1] == "SINUCA_ENGINE"):
                if (split_parameter_line[2] == "global_cycle") :
                    cache_global_cycle_list.append(int(split_parameter_line[3]))
                elif (split_parameter_line[2] == "reset_cycle") :
                    cache_reset_cycle_list.append(int(split_parameter_line[3]))

            ## Find Cache Parameters
            if (split_parameter_line[0] == "CACHE_MEMORY") and (split_parameter_line[1] == cache_label):
                if (split_parameter_line[2] == "stat_accesses") :
                    cache_stat_accesses_list.append(int(split_parameter_line[3]))

            ## Find Line Usage Predictor Parameters
            elif (split_parameter_line[0] == "LINE_USAGE_PREDICTOR") and (split_parameter_line[1] == "Line_Usage_Predictor_" + cache_label):

                # Wanted statistics for Dynamic Power
                if (str.find(split_parameter_line[2], "stat_active_sub_block_per_access_") != -1) :
                    for i in range(0, 1+(line_size/DSBP_sub_block_size)) :
                        if (split_parameter_line[2] == "stat_active_sub_block_per_access_" + str(i * DSBP_sub_block_size)):
                            cache_stat_active_sub_block_per_access_list[cache_label_number][i * DSBP_sub_block_size] = int(split_parameter_line[3])

                # Wanted statistics for Static Power
                elif (str.find(split_parameter_line[2], "stat_active_sub_block_per_cycle_") != -1) :
                    for i in range(0, 1+(line_size/DSBP_sub_block_size)) :
                        if (split_parameter_line[2] == "stat_active_sub_block_per_cycle_" + str(i * DSBP_sub_block_size)):
                            cache_stat_active_sub_block_per_cycle_list[cache_label_number][i * DSBP_sub_block_size] = int(split_parameter_line[3])

                ## Find PHT Parameters
                elif (split_parameter_line[2] == "stat_DSBP_PHT_access") :
                    cache_stat_DSBP_PHT_access_list.append(int(split_parameter_line[3]))


    ############################################################################
    ## All Parameters and Statistics found !
    counter = -1
    for cache_label in cache_label_list:
        counter += 1
        # ~ print   "#" + str(counter)
        # ~ print   " Label:" + cache_label_list[counter]
        # ~ print   " Cycle:" + str(cache_global_cycle_list[counter])
        # ~ print   " Reset:" + str(cache_reset_cycle_list[counter])
        # ~ print   " Level:" + str(cache_hierarchy_level_list[counter])
        # ~ print   " Line Size:" + str(cache_line_size_list[counter])
        # ~ print   " Line Number:" + str(cache_line_number_list[counter])
        # ~ print   " Associativity:" + str(cache_associativity_list[counter])
        # ~ print   " Line Usage Predictor:" + cache_line_usage_predictor_type_list[counter]
        # ~ print   " UsageCounter Bits:" + str(cache_DSBP_usage_counter_bits_list[counter])
        # ~ print   " SubBlock Size:" + str(cache_DSBP_sub_block_size_list[counter])
        # ~ print   " PHT Line Number:" + str(cache_DSBP_PHT_line_number_list[counter])
        # ~ print   " PHT Associativity:" + str(cache_DSBP_PHT_associativity_list[counter])
        # ~ print   " Cache Accesses:" + str(cache_stat_accesses_list[counter])
        # ~ print   " PHT Accesses:" + str(cache_stat_DSBP_PHT_access_list[counter])
        # ~ print   " Sub-Blocks per Accesses:" + str(cache_stat_active_sub_block_per_access_list[counter])
        # ~ print   " Sub-Blocks per Cycles:" + str(cache_stat_active_sub_block_per_cycle_list[counter])

    ############################################################################
    ### Create all the CACTI Models

    total_cache_static_energy = []
    total_cache_dynamic_energy = []
    total_aux_static_energy = []
    total_aux_dynamic_energy = []

    counter = -1
    for cache_label in cache_label_list:
        counter += 1

        # Maintain the Energy for the total cycles
        cache_static_power = 0.0
        # Maintain the Energy for the total access
        cache_dynamic_energy = 0.0
        # Maintain the Energy for different line sizes
        cache_static_power_array = []
        cache_dynamic_energy_array = []

        for i in range(0, 1 + line_size) :
            cache_static_power_array.append(0.0)
            cache_dynamic_energy_array.append(0.0)

        # Maintain the Energy for the Auxiliar Structure (PHT)
        auxiliar_static_power = 0.0
        auxiliar_dynamic_energy = 0.0

        ############################
        # Simulation Parameters
        predictor_type = cache_line_usage_predictor_type_list[counter]
        cache_level = cache_hierarchy_level_list[counter]
        cache_line_number = cache_line_number_list[counter]
        cache_line_size = cache_line_size_list[counter]
        cache_size = cache_line_number * cache_line_size
        cache_associativity = cache_associativity_list[counter]
        ############################
        # User definitions
        cache_bank = 1
        cache_integration_technology = 0.045 # 45nm
        cache_tag_size = 24
        cache_out = cache_line_size_list[counter]
        cache_model = "cache"
        cache_type = "UCA"
        access_type = "normal"
        if (cache_hierarchy_level_list[counter] == 1):
            access_type = "fast"
        data_array = "itrs-hp"
        tag_array = "itrs-hp"
        rw_port = 0
        read_port = 1
        write_port = 1

        ########################################################################
        ## Compute the Normal Cache Energy
        ########################################################################
        if (predictor_type == "DISABLE") or (predictor_type == "DSBP_DISABLE"):

            ### Add the tag overhead
            cache_tag_size += cache_DSBP_sub_block_size_list[counter] # Sector Cache Bits

            TMP_FILE_NAME = "LP_"+ predictor_type + "_L"+ str(cache_level) + "_CS"+ str(cache_size) + "_LS"+ str(cache_line_size) + "_CA"+ str(cache_associativity) \
                            + "_CO"+ str(cache_out) + "_CB"+ str(cache_bank) + "_TI"+ str(cache_integration_technology) + "_CT"+ str(cache_tag_size) + "_"   + cache_type
            TMP_CFG_FILE_NAME = CACTI_MODEL_CFG_DIR + TMP_FILE_NAME + ".CFG"
            TMP_OUT_FILE_NAME = CACTI_MODEL_OUTPUT_DIR + TMP_FILE_NAME + ".OUT"

            try:
                TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')
                #print("Using Old Cacti Model: "+TMP_OUT_FILE_NAME)
            except IOError:
                print("Creating New Cacti Model: "+TMP_OUT_FILE_NAME)
                cacti_defined_cfg(TMP_CFG_FILE_NAME, cache_level, cache_size, cache_line_size, cache_associativity, cache_line_size \
                                                   , cache_integration_technology, cache_bank, cache_model, cache_tag_size, cache_type, access_type, data_array, tag_array, rw_port, read_port, write_port)
                cacti_base_cfg(TMP_CFG_FILE_NAME)
                os.system(CACTI_HOME + "cacti -infile " + TMP_CFG_FILE_NAME + " > " + TMP_OUT_FILE_NAME)
                TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')

            for out_file_line in TMP_OUT_FILE:
                out_file_line = out_file_line.rstrip('\r\n')
                #Total dynamic read energy per access (nJ):
                if  "Total dynamic read energy/access" in out_file_line: #(nJ)
                    out_file_line_split = out_file_line.split(":")
                    for number in out_file_line_split:
                        if is_number(number.strip()):
                            # ~ PRINT("Dynamic:" + number)
                            cache_dynamic_energy += float(number)
                            # ~ PRINT("cache_dynamic_energy:" + str(cache_dynamic_energy))
                #Total leakage power of a bank (mW):
                elif  "Total leakage read/write power of a bank" in out_file_line: #(mW)
                    out_file_line_split = out_file_line.split(":")
                    for number in out_file_line_split:
                        if is_number(number.strip()):
                            # ~ PRINT("Leakage:" + number)
                            cache_static_power += float(number)
                            # ~ PRINT("cache_static_power:" + str(cache_static_power))
            TMP_OUT_FILE.close()

        ########################################################################
        ## Compute the DSBP Energy
        ########################################################################
        if (predictor_type == "DSBP"):
             # Two extra bytes as overhead for VDD transistor
            cache_tag_size += 2 * 8
            ### Add the tag overhead
            cache_tag_size += cache_DSBP_sub_block_size_list[counter] * cache_DSBP_usage_counter_bits_list[counter] # Counters bits
            cache_tag_size += cache_DSBP_sub_block_size_list[counter] # Overflow bits
            cache_tag_size += 10 # PC and Offset
            cache_tag_size += 1 # Learn

            for i in range(0, 1 + cache_line_size_list[counter]) :
                if (i % cache_DSBP_sub_block_size_list[counter] != 0):
                    continue
                cache_line_size = i

                ### Add a small overhead in case no sub-block was turned-on
                if cache_line_size == 0:
                    cache_line_size = 1
                cache_size = cache_line_number * cache_line_size
                TMP_FILE_NAME = "LP_"+ predictor_type + "_L"+ str(cache_level) + "_CS"+ str(cache_size) + "_LS"+ str(cache_line_size) + "_CA"+ str(cache_associativity) \
                                + "_CO"+ str(cache_out) + "_CB"+ str(cache_bank) + "_TI"+ str(cache_integration_technology) + "_CT"+ str(cache_tag_size) + "_"   + cache_type
                TMP_CFG_FILE_NAME = CACTI_MODEL_CFG_DIR + TMP_FILE_NAME + ".CFG"
                TMP_OUT_FILE_NAME = CACTI_MODEL_OUTPUT_DIR + TMP_FILE_NAME + ".OUT"

                try:
                    TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')
                    #print("Using Old Cacti Model: "+TMP_OUT_FILE_NAME)
                except IOError:
                    print("Creating New Cacti Model: "+TMP_OUT_FILE_NAME)
                    cacti_defined_cfg(TMP_CFG_FILE_NAME, cache_level, cache_size, cache_line_size, cache_associativity, cache_line_size \
                                                       , cache_integration_technology, cache_bank, cache_model, cache_tag_size, cache_type, access_type, data_array, tag_array, rw_port, read_port, write_port)
                    cacti_base_cfg(TMP_CFG_FILE_NAME)
                    os.system(CACTI_HOME + "cacti -infile " + TMP_CFG_FILE_NAME + " > " + TMP_OUT_FILE_NAME)
                    TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')

                for out_file_line in TMP_OUT_FILE:
                    out_file_line = out_file_line.rstrip('\r\n')
                    if  "Total dynamic read energy/access" in out_file_line: #(nJ)
                        out_file_line_split = out_file_line.split(":")
                        for number in out_file_line_split:
                            if is_number(number.strip()):
                                # ~ PRINT("Dynamic:" + number)
                                cache_dynamic_energy_array[i] += float(number)
                    elif  "Total leakage read/write power of a bank" in out_file_line: #(mW)
                        out_file_line_split = out_file_line.split(":")
                        for number in out_file_line_split:
                            if is_number(number.strip()):
                                # ~ PRINT("Leakage:" + number)
                                cache_static_power_array[i] += float(number)
                TMP_OUT_FILE.close()

            ####################################################################
            ### PHT ENERGY
            predictor_type = cache_line_usage_predictor_type_list[counter] + "PHT"
            cache_level = 1
            cache_line_number = cache_DSBP_PHT_line_number_list[counter]
            cache_line_size = cache_DSBP_sub_block_size_list[counter] * cache_DSBP_usage_counter_bits_list[counter] # Counters and Overflow bits
            cache_line_size += 24 # PC and Offset bits
            cache_line_size += 1 # Learn bit
            cache_line_size += 1 # Prt bit
            cache_line_size /= 8
            cache_size = cache_line_number * cache_line_size
            cache_associativity = cache_associativity_list[counter]
            cache_bank = 1
            cache_integration_technology = 0.045
            cache_tag_size = 16
            cache_out = 3
            cache_type = "UCA"
            access_type = "sequential"
            data_array = "itrs-hp"
            tag_array = "itrs-hp"
            rw_port = 1
            read_port = 0
            write_port = 0

            TMP_FILE_NAME = "LP_"+ predictor_type + "_L"+ str(cache_level) + "_CS"+ str(cache_size) + "_LS"+ str(cache_line_size) + "_CA"+ str(cache_associativity) \
                            + "_CO"+ str(cache_out) + "_CB"+ str(cache_bank) + "_TI"+ str(cache_integration_technology) + "_CT"+ str(cache_tag_size) + "_"   + cache_type
            TMP_CFG_FILE_NAME = CACTI_MODEL_CFG_DIR + TMP_FILE_NAME + ".CFG"
            TMP_OUT_FILE_NAME = CACTI_MODEL_OUTPUT_DIR + TMP_FILE_NAME + ".OUT"

            try:
                TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')
                #print("Using Old Cacti Model: "+TMP_OUT_FILE_NAME)
            except IOError:
                print("Creating New Cacti Model: "+TMP_OUT_FILE_NAME)
                cacti_defined_cfg(TMP_CFG_FILE_NAME, cache_level, cache_size, cache_line_size, cache_associativity, cache_line_size \
                                                   , cache_integration_technology, cache_bank, cache_model, cache_tag_size, cache_type, access_type, data_array, tag_array, rw_port, read_port, write_port)
                cacti_base_cfg(TMP_CFG_FILE_NAME)
                os.system(CACTI_HOME + "cacti -infile " + TMP_CFG_FILE_NAME + " > " + TMP_OUT_FILE_NAME)
                TMP_OUT_FILE = open(TMP_OUT_FILE_NAME, 'r')

            for out_file_line in TMP_OUT_FILE:
                out_file_line = out_file_line.rstrip('\r\n')
                if  "Total dynamic read energy/access" in out_file_line: #(nJ)
                    out_file_line_split = out_file_line.split(":")
                    for number in out_file_line_split:
                        if is_number(number.strip()):
                            # ~ PRINT("PHT Dynamic:" + number)
                            auxiliar_dynamic_energy += float(number)
                elif  "Total leakage read/write power of a bank" in out_file_line: #(mW)
                    out_file_line_split = out_file_line.split(":")
                    for number in out_file_line_split:
                        if is_number(number.strip()):
                            # ~ PRINT("PHT Leakage:" + number)
                            auxiliar_static_power += float(number)
            TMP_OUT_FILE.close()


        ########################################################################
        ### After get all Simulation and CACTI information
        ########################################################################
        FREQUENCY = 4000000000 # 4GHz 4,000,000,000 cycles/second
        MILI_TO_NANO = 1000000 # 1 mili = 1,000,000 nano

        total_cache_static_energy.append(0.0)
        total_cache_dynamic_energy.append(0.0)
        total_aux_static_energy.append(0.0)
        total_aux_dynamic_energy.append(0.0)

        if (cache_line_usage_predictor_type_list[counter] == "DISABLE") or (cache_line_usage_predictor_type_list[counter] == "DSBP_DISABLE"):
            total_cache_dynamic_energy[counter] = cache_dynamic_energy * cache_stat_accesses_list[counter]
            total_cache_static_energy[counter] = (MILI_TO_NANO * cache_static_power * (cache_global_cycle_list[counter] - cache_reset_cycle_list[counter])) / FREQUENCY

        elif (cache_line_usage_predictor_type_list[counter] == "DSBP"):
            for i in range(0, 1 + cache_line_size_list[counter]) :
                total_cache_dynamic_energy[counter] += cache_dynamic_energy_array[i] * cache_stat_active_sub_block_per_access_list[counter][i]
                total_cache_static_energy[counter] += ((MILI_TO_NANO * cache_static_power_array[i]) * (cache_stat_active_sub_block_per_cycle_list[counter][i] / cache_line_number_list[counter])) / FREQUENCY
            total_aux_dynamic_energy[counter] = auxiliar_dynamic_energy * cache_stat_DSBP_PHT_access_list[counter]
            total_aux_static_energy[counter] = (MILI_TO_NANO * auxiliar_static_power * (cache_global_cycle_list[counter] - cache_reset_cycle_list[counter])) / FREQUENCY

        print("\n=================================")
        PRINT(app_name)
        PRINT("#" + str(counter))
        PRINT("Label:" + cache_label_list[counter])
        PRINT("Cycle:" + str(cache_global_cycle_list[counter]) + " Reset_Cycle:" + str(cache_reset_cycle_list[counter]))

        PRINT("Cache_Accesses:" + str(cache_stat_accesses_list[counter])        + " Cache_Energy:" + str(cache_dynamic_energy)    + " Cache_Power:" + str(cache_static_power))
        PRINT("PHT_Accesses:" + str(cache_stat_DSBP_PHT_access_list[counter]) + " Aux_Energy:" + str(auxiliar_dynamic_energy) + " Aux_Power:" + str(auxiliar_static_power))

        str_cache_stat_active_sub_block_per_access_list = ""
        str_cache_dynamic_energy_array = ""
        str_cache_stat_active_sub_block_per_cycle_list = ""
        str_cache_static_power_array = ""

        total_accesses = 0
        total_cycles = 0
        for i in range(0, 1 + cache_line_size_list[counter]) :
            if (i % cache_DSBP_sub_block_size_list[counter] != 0):
                continue
            total_accesses += cache_stat_active_sub_block_per_access_list[counter][i]
            str_cache_stat_active_sub_block_per_access_list += " " + str(cache_stat_active_sub_block_per_access_list[counter][i])
            str_cache_dynamic_energy_array += " " + str(cache_dynamic_energy_array[i])

            total_cycles += cache_stat_active_sub_block_per_cycle_list[counter][i]
            str_cache_stat_active_sub_block_per_cycle_list += " " + str(cache_stat_active_sub_block_per_cycle_list[counter][i])
            str_cache_static_power_array += " " + str(cache_static_power_array[i])

        if (cache_line_usage_predictor_type_list[counter] == "DSBP"):
            if (total_accesses != cache_stat_accesses_list[counter]):
                PRINT("WARNING !!!\n Wrong number of accesses !!!")
            if (total_cycles / cache_line_number_list[counter]  != cache_global_cycle_list[counter] - cache_reset_cycle_list[counter] + 1):
                print("total_cycles:" + str(total_cycles))
                print("cache_line_number_list[counter]:" + str(cache_line_number_list[counter]))
                print("cache_global_cycle_list[counter]:" + str(cache_global_cycle_list[counter]))
                print("cache_reset_cycle_list[counter]:" + str(cache_reset_cycle_list[counter]))
                PRINT("WARNING !!!\n Wrong number of cycles !!!")

        PRINT("Sub_Blocks_per_Accesses:" + str_cache_stat_active_sub_block_per_access_list)
        PRINT("Dyn_Energy:" + str_cache_dynamic_energy_array)

        PRINT("Sub-Blocks_per_Cycles:" + str_cache_stat_active_sub_block_per_cycle_list)
        PRINT("Stat_Power:" + str_cache_static_power_array)

        PRINT("Cache_Dynamic_Energy:" + str(total_cache_dynamic_energy[counter]))
        PRINT("Cache_Static_Energy:" + str(total_cache_static_energy[counter]))
        PRINT("Aux_Dynamic_Energy:" + str(total_aux_dynamic_energy[counter]))
        PRINT("Aux_Static_Energy:" + str(total_aux_static_energy[counter]))

    app_file.close()

    ############################################################################
    ### Write the output file
    ############################################################################
    ## Output file header
    if has_header == 0:
        has_header = 1
        header = ""
        output_results_file.write("Experiment")
        for cache_label in cache_label_list:
            #~ header += " " + cache_label + "_Static"
            #~ header += " " + cache_label + "_Dynamic"
            #~ header += " Aux_" + cache_label + "_Static"
            #~ header += " Aux_" + cache_label + "_Dynamic"
            header += " " + cache_label
            header += " Aux_" + cache_label

        output_results_file.write(header)
        output_results_file.write(" Sum\n")


    # Experiment + App Name
    output_results_file.write(arg_input_results_filename + "_" + app_name + " ")
    # Only the App Name
    # ~ output_results_file.write(app_name + " ")

    sum_values = 0
    counter = -1
    for cache_label in cache_label_list:
        counter += 1
        #~ output_results_file.write(str(total_cache_static_energy[counter]) + " ")
        #~ output_results_file.write(str(total_cache_dynamic_energy[counter]) + " ")
        #~ output_results_file.write(str(total_aux_static_energy[counter]) + " ")
        #~ output_results_file.write(str(total_aux_dynamic_energy[counter]) + " ")
        output_results_file.write(str(total_cache_static_energy[counter] + total_cache_dynamic_energy[counter]) + " ")
        output_results_file.write(str(total_aux_static_energy[counter] + total_aux_dynamic_energy[counter]) + " ")

        sum_values += total_cache_static_energy[counter]
        sum_values += total_cache_dynamic_energy[counter]
        sum_values += total_aux_static_energy[counter]
        sum_values += total_aux_dynamic_energy[counter]
    output_results_file.write(str(sum_values) + " ")
    output_results_file.write("\n")
app_list_file.close()
output_results_file.close()


########################################################################
# Stacked Bars
########################################################################

# Open the plot filename
gnulot_base(arg_gnuplot_filename)
output_gnuplot_file = open(arg_gnuplot_filename, 'a')

output_gnuplot_file.write("set border 3\n")
output_gnuplot_file.write("set style data histograms\n")
output_gnuplot_file.write("set style histogram rowstack gap 1 title offset character 2, 0.25, 0 \n")
output_gnuplot_file.write("set style fill solid border -1\n")
output_gnuplot_file.write("set boxwidth 0.80\n")
output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

# ~ output_gnuplot_file.write("set yrange [ 0.00000 : 100.000 ] noreverse \n")
# ~ output_gnuplot_file.write("set format y '%g%%' \n")
# ~ output_gnuplot_file.write("set ytics 10\n")
# ~ output_gnuplot_file.write("set border 3\n")
# ~ output_gnuplot_file.write("set style data histograms\n")
# ~ output_gnuplot_file.write("set style histogram rowstack gap 1 title offset character 2, 0.25, 0 \n")
# ~ output_gnuplot_file.write("set style fill solid border -1\n")
# ~ output_gnuplot_file.write("set boxwidth 0.80\n")
# ~ output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
# ~ output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
# ~ output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

header = header.strip()
header_list = header.split(" ")
# First two values are Architecture + Application
title_size = 1
# Last Column has the Sum of Values
column_sum = title_size + len(header_list) + 1
count = 0;
for parameter_name in header_list:
    parameter_name = parameter_name.split('.')
    title = parameter_name[0]
    count += 1
    if count == 1:
        output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (column(" + str(title_size + count) + ")):xtic(1) title '" + str(title) + "'")
    else :
        output_gnuplot_file.write(", '' using (column(" + str(title_size + count) + ")) title '" + str(title) + "'")

    # ~ if count == 1:
        # ~ output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))):xtic(1) title '" + str(title) + "'")
    # ~ else :
        # ~ output_gnuplot_file.write(", '' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))) title '" + str(title) + "'")


########################################################################
# Close the gnuplot file and plot it !
output_gnuplot_file.write("\n")
output_gnuplot_file.close()

PRINT("gnuplot " + arg_gnuplot_filename)
os.system("gnuplot " + arg_gnuplot_filename)



sys.exit()
