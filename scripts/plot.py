import sys
import os
import subprocess
import stat

################################################################################
## Print the GNUPLOT header
################################################################################
def gnulot_base(TMP_GNU_FILE_NAME):
    TMP_GNU_FILE = open(TMP_GNU_FILE_NAME, 'w')
    TMP_GNU_FILE.write("reset\n")
    ##########################
    # Output
    TMP_GNU_FILE.write("set terminal jpeg medium size 1600,1024 font Helvetica 16\n")

    ##########################
    # Scale
    TMP_GNU_FILE.write("set autoscale x\n")
    TMP_GNU_FILE.write("set autoscale y\n")
    TMP_GNU_FILE.write("set yrange [ 0.00000 : 100.000 ] noreverse \n")
    TMP_GNU_FILE.write("set xtics nomirror rotate by -90 \n")
    TMP_GNU_FILE.write("set ytics 10\n")
    TMP_GNU_FILE.write("set format y '%g%%' \n")

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
    # Stacked Bars
    TMP_GNU_FILE.write("set border 3\n")
    TMP_GNU_FILE.write("set style data histograms\n")
    TMP_GNU_FILE.write("set style histogram rowstacked\n")
    TMP_GNU_FILE.write("set style histogram rowstack gap 1 title offset character 2, 0.25, 0 \n")
    TMP_GNU_FILE.write("set style fill solid border -1\n")
    TMP_GNU_FILE.write("set boxwidth 0.80\n")
    TMP_GNU_FILE.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
    ##########################
    # Subtitle
    TMP_GNU_FILE.write("set key autotitle columnheader\n")
    TMP_GNU_FILE.write("set key outside below center horizontal\n")
    TMP_GNU_FILE.write("set datafile separator ';' \n")

    TMP_GNU_FILE.close()
    return
########################################################################

################################################################################
# This prints a passed string into this function
################################################################################
def PRINT( str ):
   print "PYTHON: " + str
   return

################################################################################
BENCHMARK_LIST = ["spec_cpu2000", "spec_cpu2006", "spec_omp2001", "npb_omp"]
PLOT_LIST = ["lines", "bars"]

if (len(sys.argv) < 2):
    sys.exit("Usage: python plot.py config_file")
else :
    arg_configure = sys.argv[1]

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
os.system("mkdir " + PROJECT_HOME)
os.system("mkdir " + PROJECT_HOME + "benchmarks")
os.system("mkdir " + PROJECT_HOME + "benchmarks/plots")

SINUCA_HOME = PROJECT_HOME + "/SiNUCA/"
PRINT("SINUCA_HOME = " + SINUCA_HOME)
os.putenv("SINUCA_HOME", SINUCA_HOME)

arg_output_results_filename = ""
arg_gnuplot_filename = ""

cfg_file = open(arg_configure, 'r')
PRINT("===================================================================")
PRINT("Config File = " + arg_configure)
for cfg_line in cfg_file:
    cfg_line = cfg_line.rstrip('\r\n')
    cfg_line = cfg_line.strip()
    #Comments
    if (len(cfg_line) == 0) or (cfg_line[0] == '#'):
        continue

    split_cfg_line = cfg_line.split('=')

    #Erase all the parameters for the new plot
    if cfg_line.lower() == 'begin':
        PRINT("===================================================================")
        arg_results_filename = []
        arg_parameter = []
        continue

    elif split_cfg_line[0] == 'plot_filename':
        if (len(split_cfg_line) > 2):
            sys.exit("Too many output_results_filename")
        arg_output_results_filename = PROJECT_HOME + "benchmarks/plots/" + split_cfg_line[1] + ".data"
        arg_gnuplot_filename = PROJECT_HOME + "benchmarks/plots/" + split_cfg_line[1] + ".gnuplot"
        PRINT("\t Data Filename:" + arg_output_results_filename)
        PRINT("\t Gnuplot Filename:" + arg_gnuplot_filename)

    elif split_cfg_line[0].lower() == 'benchmark':
        if (len(split_cfg_line) > 2):
            sys.exit("Too many benchmark")
        elif (split_cfg_line[1].lower() not in BENCHMARK_LIST):
            sys.exit("Wrong benchmark not in" + str(BENCHMARK_LIST))
        arg_benchmark = split_cfg_line[1].lower()
        PRINT("\t Benchmark:" + arg_benchmark)

    elif split_cfg_line[0].lower() == 'plot_type':
        if (len(split_cfg_line) > 2):
            sys.exit("Too many plot_type")
        elif (split_cfg_line[1].lower() not in PLOT_LIST):
            sys.exit("Wrong plot_type not in" + str(PLOT_LIST))
        arg_plot_type = split_cfg_line[1].lower()
        PRINT("\t Plot Type:" + arg_plot_type)

    elif split_cfg_line[0].lower() == 'results_filename':
        split_cfg_line = split_cfg_line[1].split(',')
        for i in range(0, len(split_cfg_line)):
            split_cfg_line[i] = split_cfg_line[i].rstrip('\r\n')
            split_cfg_line[i] = split_cfg_line[i].rstrip('\t')
            split_cfg_line[i] = split_cfg_line[i].strip()
            arg_results_filename.append(split_cfg_line[i])
        PRINT("\t Results Filename:" + str(arg_results_filename))

    elif split_cfg_line[0].lower() == 'parameter':
        split_cfg_line = split_cfg_line[1].split(',')
        for i in range(0, len(split_cfg_line)):
            split_cfg_line[i] = split_cfg_line[i].rstrip('\r\n')
            split_cfg_line[i] = split_cfg_line[i].rstrip('\t')
            split_cfg_line[i] = split_cfg_line[i].strip()
            arg_parameter.append(split_cfg_line[i])
            PRINT("\t Parameter:" + arg_parameter[len(arg_parameter)-1])


    #Plot !
    elif cfg_line.lower() == 'end':
        PRINT("\t Plotting...")

        # Open the plot filename
        output_results_file = open(arg_output_results_filename, 'w')

        # Print the header
        output_results_file.write("ConfigFile Application ")
        for string in arg_parameter:
            string = string.split('.')
            output_results_file.write(string[len(string)-1] + " ")
        output_results_file.write("\n")


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

        # Iterates over the EXPERIMENTS (results base name)
        for results_filename in arg_results_filename:
            PRINT("\t Analyzing:" + results_filename)

            # Iterates over the APPLICATIONS results file
            app_list_file = open(app_list_filename, 'r')
            for app_list_file_line in app_list_file:
                app_list_file_line = app_list_file_line.rstrip('\r\n')
                if  app_list_file_line[0] == '#':
                    continue

                split_app_list_file_line = app_list_file_line.split(';')
                app_name = split_app_list_file_line[0] + "." + split_app_list_file_line[1] + ".PP."
                app_file_name = results_directory + app_name + results_filename + ".result"
                try:
                    app_file = open(app_file_name, 'r')
                except IOError:
                    PRINT("\t \t Application Not Found:"+app_name+" - File Not Found:"+app_file_name+" ... skipping")
                    continue
                # ~ PRINT("\t Analyzing:" + app_file_name)

                # Iterates over the PARAMETERS wanted
                results = []
                for parameter_name in arg_parameter:
                    # ~ PRINT("\t Grepping:" + parameter_name)

                    # Iterates over the RESULTS_PARAMETERS (APPLICATIONS results file)
                    app_file.seek(0,0)
                    for parameter_line in app_file:
                        parameter_line = parameter_line.rstrip('\r\n')
                        #Comments
                        if parameter_line[0] == '#':
                            continue

                        split_parameter_line = parameter_line.split(':')
                        if (str.find(split_parameter_line[0].lower(), parameter_name.lower()) != -1):
                            results.append(split_parameter_line[1])
                            # ~ PRINT("\t =>" + split_parameter_line[0] + "=" + split_parameter_line[1])
                            break;

                output_results_file.write(results_filename + " " + app_name + " ")
                for string in results:
                    output_results_file.write(string + " ")
                output_results_file.write("\n")

            app_list_file.close()

        # Open the plot filename
        gnulot_base(arg_gnuplot_filename)
        output_gnuplot_file = open(arg_gnuplot_filename, 'a')


            ########################################################################
            ## PLOT NORMAL
            ########################################################################
            # ~ TMP_GNU_FILE_NAME = OUTPUT_DIR+"/"+BASE_NAME+DESCRIPTION_NAME+".gnuplot"
            # ~ gnulot_base(TMP_GNU_FILE_NAME)
# ~
            # ~ TMP_GNU_FILE = open(TMP_GNU_FILE_NAME, 'a')
            # ~ TMP_GNU_FILE.write("set title 'Histogram Line Usage' \n")
            # ~ TMP_GNU_FILE.write("set output '"+TMP_OUT_FILE_NAME+".jpeg' \n")
            # ~ TMP_GNU_FILE.write("plot '"+TMP_OUT_FILE_NAME+".dat' using (100*(column(6)/column(2))):xtic(1) title '"+str(BLOCK)+"'")
            # ~ for k in range(2, 1+(64/BLOCK)):
                # ~ TMP_GNU_FILE.write(", '' using (100*(column("+str(k+5)+")/column(2))) title '"+str(k*BLOCK)+"'")
            # ~ TMP_GNU_FILE.write("\n")
            # ~ TMP_GNU_FILE.close()
# ~
            # ~ print "PLOTING: NORMAL..."
            # ~ os.system("gnuplot "+TMP_GNU_FILE_NAME)



        output_gnuplot_file.close()

cfg_file.close()
sys.exit()


