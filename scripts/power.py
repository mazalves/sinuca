import sys
import os
import subprocess
import stat
import re
################################################################################
# This prints a passed string into this function
################################################################################
def PRINT( str ):
   print "PYTHON: " + str
   return


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
def cacti_defined_cfg(TMP_CFG_FILE_NAME, CacheLevel, CacheSize, LineSize, CacheAssoc, BlockCount, TechInt, OutPut, CacheType, TAG, UCA, Access, DataArray, TagArray, RW_Port, R_Port, W_Port):
    TMP_CFG_FILE = open(TMP_CFG_FILE_NAME, 'w')

    TMP_CFG_FILE.write("####################################################\n")
    # Cache Level
    TMP_CFG_FILE.write("-Cache level (L2/L3) - \""+CacheLevel+"\" \n")
    ### Cache size
    TMP_CFG_FILE.write("-size (bytes) "+str(CacheSize)+" \n")
    ### Line size
    TMP_CFG_FILE.write("-block size (bytes) "+str(LineSize)+" \n")
    ### To model Fully Associative cache, set associativity to zero
    TMP_CFG_FILE.write("-associativity "+str(CacheAssoc)+" \n")
    ### Multiple banks connected using a bus
    TMP_CFG_FILE.write("-UCA bank count "+str(BlockCount)+" \n")
    ### Integration Technology -- (0.032, 0.045, 0.068, 0.090)
    TMP_CFG_FILE.write("-technology (u) "+str(TechInt)+" \n")
    ### Bus width include data bits and address bits required by the decoder
    TMP_CFG_FILE.write("-output/input bus width "+str(OutPut)+" \n")
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
    sys.exit("Usage: python plot.py benchamrk base_input_name base_output_name")
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
os.putenv("PROJECT_HOME", PROJECT_HOME)
os.system("mkdir -p " + PROJECT_HOME + "benchmarks/plots")

SINUCA_HOME = PROJECT_HOME + "/SiNUCA/"
PRINT("SINUCA_HOME = " + SINUCA_HOME)
os.putenv("SINUCA_HOME", SINUCA_HOME)

PRINT("\t Benchmark:" + arg_benchmark)
PRINT("\t Input Results Filename:" + str(arg_input_results_filename))
PRINT("\t Output Data Filename:" + arg_output_results_filename)

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

PRINT("\t App List Filename = " + app_list_filename)
PRINT("\t Results Directory = " + results_directory)


# Iterates over the APPLICATIONS name from benchmark
try:
    app_list_file = open(app_list_filename, 'r')
except IOError:
    PRINT("\t \t app_list_filename Not Found:" + arg_benchmark + " - File Not Found:" + app_list_filename + " ... skipping")
    exit()
for app_list_file_line in app_list_file:
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
        print   "#" + str(counter)
        print   " Label:" + cache_label_list[counter]
        print   " Cycle:" + str(cache_global_cycle_list[counter])
        # ~ print   " Reset:" + str(cache_reset_cycle_list[counter])
        print   " Level:" + str(cache_hierarchy_level_list[counter])
        print   " Line Size:" + str(cache_line_size_list[counter])
        print   " Line Number:" + str(cache_line_number_list[counter])
        print   " Associativity:" + str(cache_associativity_list[counter])
        print   " Line Usage Predictor:" + cache_line_usage_predictor_type_list[counter]
        print   " UsageCounter Bits:" + str(cache_DSBP_usage_counter_bits_list[counter])
        print   " SubBlock Size:" + str(cache_DSBP_sub_block_size_list[counter])
        print   " PHT Line Number:" + str(cache_DSBP_PHT_line_number_list[counter])
        print   " PHT Associativity:" + str(cache_DSBP_PHT_associativity_list[counter])

        print   " Cache Accesses:" + str(cache_stat_accesses_list[counter])
        print   " PHT Accesses:" + str(cache_stat_DSBP_PHT_access_list[counter])
        print   " Sub-Blocks per Accesses:" + str(cache_stat_active_sub_block_per_access_list[counter])
        print   " Sub-Blocks per Cycles:" + str(cache_stat_active_sub_block_per_cycle_list[counter])




    app_file.close()
app_list_file.close()

