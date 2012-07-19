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

################################################################################
BENCHMARK_LIST = ["spec_cpu2000", "spec_cpu2006", "spec_omp2001", "npb_omp"]
PLOT_LIST = ["lines", "bars", "stacked_bars", "lines_normalized", "bars_normalized", "stacked_bars_normalized"]

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

PLOTS_HOME = PROJECT_HOME + "benchmarks/plots/"
os.system("mkdir -p " + PLOTS_HOME)

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
        arg_input_results_filename = []
        arg_parameter = []
        continue

    elif split_cfg_line[0] == 'plot_filename':
        if (len(split_cfg_line) > 2):
            sys.exit("Too many output_results_filename")
        arg_output_results_filename = PLOTS_HOME + split_cfg_line[1] + ".data"
        arg_gnuplot_filename = PLOTS_HOME + split_cfg_line[1] + ".gnuplot"
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
            arg_input_results_filename.append(split_cfg_line[i])
        PRINT("\t Results Filename:" + str(arg_input_results_filename))

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

        # Print the HEADER
        # ~ output_results_file.write("ConfigFile Application ")
        output_results_file.write("ConfigFile_Application ")
        for parameter_name in arg_parameter:
            parameter_name = parameter_name.split('.')
            output_results_file.write(parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1] + " ")
        output_results_file.write(" Sum_Values\n")


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

        # Iterates over the APPLICATIONS results file
        try:
            app_list_file = open(app_list_filename, 'r')
        except IOError:
            PRINT("\t \t app_list_filename Not Found:" + arg_benchmark + " - File Not Found:" + app_list_filename + " ... skipping")
            exit()
        for app_list_file_line in app_list_file:
            app_list_file_line = app_list_file_line.rstrip('\r\n')
            if  app_list_file_line[0] == '#':
                continue

            # Iterates over the EXPERIMENTS (results base name)
            for results_filename in arg_input_results_filename:


                split_app_list_file_line = app_list_file_line.split(';')
                app_name = split_app_list_file_line[0] + "." + split_app_list_file_line[1]
                app_file_name = results_directory + split_app_list_file_line[2] + "." + results_filename + ".result"
                try:
                    app_file = open(app_file_name, 'r')
                except IOError:
                    PRINT("\t \t Application Not Found:" + app_name + " - File Not Found:" + app_file_name + " ... skipping")
                    continue
                # ~ PRINT("\t Analyzing:" + app_file_name)

                # Iterates over the PARAMETERS wanted
                results = []
                for parameter_name in arg_parameter:

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
                            break;
                # Experiment + App Name
                output_results_file.write(results_filename + "_" + app_name + " ")

                # Only the App Name
                # ~ output_results_file.write(app_name + " ")

                sum_values = 0
                for string in results:
                    output_results_file.write(string + " ")
                    if string.isdigit():
                        sum_values += float(string)
                output_results_file.write(str(sum_values) + " ")
                output_results_file.write("\n")

        app_list_file.close()
        output_results_file.close()


        ########################################################################
        # Open the plot filename
        gnulot_base(arg_gnuplot_filename)
        output_gnuplot_file = open(arg_gnuplot_filename, 'a')

        ########################################################################
        # Lines
        ########################################################################
        if arg_plot_type == 'lines':
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data lines\n")
            output_gnuplot_file.write("set style fill solid border 1\n")
            output_gnuplot_file.write("set boxwidth 1.0\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
            output_gnuplot_file.write("set output '"+arg_output_results_filename+".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (column(" + str(title_size + count) + ")):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (column(" + str(title_size + count) + ")) title '" + str(title) + "'")

        elif arg_plot_type == 'lines_normalized':
            output_gnuplot_file.write("set yrange [ 0.00000 : 100.000 ] noreverse \n")
            output_gnuplot_file.write("set format y '%g%%' \n")
            output_gnuplot_file.write("set ytics 10\n")
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data lines\n")
            output_gnuplot_file.write("set style fill solid border 1\n")
            output_gnuplot_file.write("set boxwidth 1.0\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "_NORMALIZED' \n")
            output_gnuplot_file.write("set output '"+arg_output_results_filename+".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            # Last Column has the Sum of Values
            column_sum = title_size + len(arg_parameter) + 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))) title '" + str(title) + "'")

        ########################################################################
        # Bars
        ########################################################################
        elif arg_plot_type == 'bars':
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data histograms\n")
            output_gnuplot_file.write("set style fill solid border -1\n")
            output_gnuplot_file.write("set boxwidth 0.80\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
            output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            # Last Column has the Sum of Values
            column_sum = title_size + len(arg_parameter) + 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (column(" + str(title_size + count) + ")):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (column(" + str(title_size + count) + ")) title '" + str(title) + "'")

        elif arg_plot_type == 'bars_normalized':
            output_gnuplot_file.write("set yrange [ 0.00000 : 100.000 ] noreverse \n")
            output_gnuplot_file.write("set format y '%g%%' \n")
            output_gnuplot_file.write("set ytics 10\n")
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data histograms\n")
            output_gnuplot_file.write("set style fill solid border -1\n")
            output_gnuplot_file.write("set boxwidth 0.80\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "_NORMALIZED' \n")
            output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            # Last Column has the Sum of Values
            column_sum = title_size + len(arg_parameter) + 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))) title '" + str(title) + "'")

        ########################################################################
        # Stacked Bars
        ########################################################################
        elif arg_plot_type == 'stacked_bars':
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data histograms\n")
            output_gnuplot_file.write("set style histogram rowstack gap 1 title offset character 2, 0.25, 0 \n")
            output_gnuplot_file.write("set style fill solid border -1\n")
            output_gnuplot_file.write("set boxwidth 0.80\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
            output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            # Last Column has the Sum of Values
            column_sum = title_size + len(arg_parameter) + 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (column(" + str(title_size + count) + ")):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (column(" + str(title_size + count) + ")) title '" + str(title) + "'")

        elif arg_plot_type == 'stacked_bars_normalized':
            output_gnuplot_file.write("set yrange [ 0.00000 : 100.000 ] noreverse \n")
            output_gnuplot_file.write("set format y '%g%%' \n")
            output_gnuplot_file.write("set ytics 10\n")
            output_gnuplot_file.write("set border 3\n")
            output_gnuplot_file.write("set style data histograms\n")
            output_gnuplot_file.write("set style histogram rowstack gap 1 title offset character 2, 0.25, 0 \n")
            output_gnuplot_file.write("set style fill solid border -1\n")
            output_gnuplot_file.write("set boxwidth 0.80\n")
            output_gnuplot_file.write("set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000\n")
            output_gnuplot_file.write("set title '" + arg_output_results_filename + "' \n")
            output_gnuplot_file.write("set output '" + arg_output_results_filename + ".jpeg' \n")

            # First two values are Architecture + Application
            title_size = 1
            # Last Column has the Sum of Values
            column_sum = title_size + len(arg_parameter) + 1
            count = 0;
            for parameter_name in arg_parameter:
                parameter_name = parameter_name.split('.')
                title = parameter_name[len(parameter_name)-2] + "." + parameter_name[len(parameter_name)-1]
                count += 1
                if count == 1:
                    output_gnuplot_file.write("plot '" + arg_output_results_filename + "' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))):xtic(1) title '" + str(title) + "'")
                else :
                    output_gnuplot_file.write(", '' using (100*(column(" + str(title_size + count) + ")/column(" + str(column_sum) + "))) title '" + str(title) + "'")


        ########################################################################
        # Close the gnuplot file and plot it !
        output_gnuplot_file.write("\n")
        output_gnuplot_file.close()

        PRINT("gnuplot " + arg_gnuplot_filename)
        os.system("gnuplot " + arg_gnuplot_filename)


cfg_file.close()
sys.exit()
