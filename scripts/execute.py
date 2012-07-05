import sys
import os
import subprocess
import stat

################################################################################
# This prints a passed string into this function
################################################################################
def PRINT( str ):
   print "PYTHON: " + str
   return

################################################################################
BENCHMARK_LIST = ["spec_cpu2000", "spec_cpu2006", "spec_omp2001", "npb_omp"]

if (len(sys.argv) < 6) or (sys.argv[2] not in BENCHMARK_LIST):
    PRINT("Usage: python execute.py config_file "+str(BENCHMARK_LIST)+ " result_base_name warmup_instructions number_threads app_start app_end")
    sys.exit()
else :
    arg_configure = sys.argv[1]
    arg_benchmark = sys.argv[2]
    arg_result = sys.argv[3]
    arg_warmup = int(sys.argv[4])
    arg_threads = int(sys.argv[5])
    arg_app_start = int(sys.argv[6])
    arg_app_end = int(sys.argv[7])

PRINT("APP_START = " + str(arg_app_start))
PRINT("APP_END = " + str(arg_app_end))

# PROJECT_HOME with:
#   |
#   |- benchmarks
#   |   |- pin_points
#   |   |- traces
#   |   |- profiles
#   |   |- results
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
os.system("mkdir " + PROJECT_HOME)
os.system("mkdir " + PROJECT_HOME + "benchmarks")
os.system("mkdir " + PROJECT_HOME + "benchmarks/pin_points")
os.system("mkdir " + PROJECT_HOME + "benchmarks/traces")
os.system("mkdir " + PROJECT_HOME + "benchmarks/profiles")
os.system("mkdir " + PROJECT_HOME + "benchmarks/results")

SINUCA_HOME = PROJECT_HOME + "/SiNUCA/"
PRINT("SINUCA_HOME = " + SINUCA_HOME)
os.putenv("SINUCA_HOME", SINUCA_HOME)

if arg_benchmark == 'spec_cpu2000':
    APP_FILE_NAME   = SINUCA_HOME + "/scripts/command_to_run_spec_cpu2000.txt"
    TRACE_SRC       = PROJECT_HOME + "/benchmarks/traces/spec_cpu2000/"
    RESUTS_DST      = PROJECT_HOME + "/benchmarks/results/spec_cpu2000/"
    TRACE_SUFIX     = ""

elif arg_benchmark == 'spec_cpu2006':
    APP_FILE_NAME   = SINUCA_HOME + "/scripts/command_to_run_spec_cpu2006.txt"
    TRACE_SRC       = PROJECT_HOME + "/benchmarks/traces/spec_cpu2006/"
    RESUTS_DST      = PROJECT_HOME + "/benchmarks/results/spec_cpu2006/"
    TRACE_SUFIX     = ""

elif arg_benchmark == 'spec_omp2001':
    APP_FILE_NAME   = SINUCA_HOME + "/scripts/command_to_run_spec_omp2001.txt"
    TRACE_SRC       = PROJECT_HOME + "/benchmarks/traces/spec_omp2001/"
    RESUTS_DST      = PROJECT_HOME + "/benchmarks/results/spec_omp2001/"
    TRACE_SUFIX     = "_" + str(arg_threads) + "t"

elif arg_benchmark == 'npb_omp':
    APP_FILE_NAME   = SINUCA_HOME + "/scripts/command_to_run_npb_omp.txt"
    TRACE_SRC       = PROJECT_HOME + "/benchmarks/traces/npb_omp/"
    RESUTS_DST      = PROJECT_HOME + "/benchmarks/results/npb_omp/"
    TRACE_SUFIX     = "_" + str(arg_threads) + "t"


PRINT("mkdir " + RESUTS_DST)
os.system("mkdir " + RESUTS_DST)


################################################################################
## BENCHMARK SIMPLE SIMULATION
################################################################################
APP_FILE = open(APP_FILE_NAME, 'r')
PRINT("APP_FILE_NAME = " + APP_FILE_NAME)
PRINT("===================================================================")
PRINT("Simulating " + arg_benchmark)
PRINT("===================================================================")
app_count = 0
for app_line in APP_FILE:
    app_line = app_line.rstrip('\r\n')
    if  (len(app_line) == 0) or (app_line[0] == '#'):
        continue
    app_count += 1
    if (app_count >= arg_app_start) and (app_count <= arg_app_end) :
        PRINT("===========================================================")
        split_app_line = app_line.split(';')

        PROGRAM=split_app_line[0]
        PRINT(str(app_count) + "-PROGRAM = " + PROGRAM)

        INPUT = split_app_line[1]
        TRACE_FILE = split_app_line[2]

        PRINT("INPUT = " + INPUT)
        PRINT("conf = " + arg_configure)
        PRINT("trace = " + TRACE_SRC + TRACE_FILE + TRACE_SUFIX)
        PRINT("result = " + RESUTS_DST + TRACE_FILE + "." + arg_result + ".result")
        PRINT("warmup = " + str(arg_warmup))

        COMMAND = "date; time " + SINUCA_HOME + "sinuca -conf " + arg_configure + " -trace " + TRACE_SRC + TRACE_FILE + TRACE_SUFIX + " -result "+ RESUTS_DST + TRACE_FILE + "." + arg_result + ".result -warmup " + str(arg_warmup) + " > "+ RESUTS_DST + TRACE_FILE + "." + arg_result + ".log"
        PRINT("COMMAND = " + COMMAND)
        os.system(COMMAND)
PRINT("===================================================================")
APP_FILE.close()


