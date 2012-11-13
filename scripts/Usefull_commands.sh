# rename changing the original filename
for i in *; do echo $i; j=`echo $i | sed -e 's/Base\./BaseN\./g'`; echo $j; mv $i $j ; done
for i in *; do echo $i; j=`echo $i | sed -e 's/PP/PP200M/g'`; echo $j; mv $i $j ; done

#Send multiple commands to byobu
for i in `seq 1 30` ; do
    byobu -p$i -X stuff "reset ; echo $i \
                                teste $(echo -ne '\r')";
done

## Run the CPPCHECK on all the source code with all flags enabled
cppcheck  --enable=all -f  *.hpp *.cpp */*.cpp */*.hpp 2> err.txt

## This is how we can find the length of the longest line in a file.
$ awk ' { if ( length > L ) { L=length} }END{ print L}' file.txt
## And also to print the longest line along with the length:
$ awk ' { if ( length > L ) { L=length ;s=$0 } }END{ print L,"\""s"\"" }' file.txt

## Get the libraries linked dynamic
ldd <binary>

## Get the assembly out of a binary
objdump -D <binary>

## The command "nm" lists symbols contained in the object file or shared library.
nm -D libctest.so.1.0

## Arithmetic, use (( ))
marco=$((16 % 8))

#find possible wait points when threads syncronize
rm *.txt
for i in *8t.tid0.stat.out.gz; do
    echo $i;
    k=`echo $i | sed -e 's/.tid0.stat.out.gz//g'`;
    echo $k;
    less $i | grep PAUSE -A3 -B4 | grep "@" > test.txt;
    p=""
    for j in `cat test.txt`; do
        p="--regexp $j $p" ;
    done ;
    m=`echo $p | sed 's/@//g'`;
    echo $m;
    echo Lines > cmp_jnz.txt
    less $k*dyn* | wc -l >> cmp_jnz.txt
    echo Pause >> cmp_jnz.txt
    less $k*dyn* | grep -x $m | sort | uniq -c >> cmp_jnz.txt;
    mv cmp_jnz.txt $k.pause.txt
done

# Check the progress of multiple simulations
for i in `ls ~/Experiment/benchmarks/results/*/*.log`; do tail -n16 $i | grep 'CPU  0' | grep -v 100.000%| grep IPC -m1 ; done
## Shows the benchmarks which failed.
for i in `ls ~/Experiment/benchmarks/results/*/*.log | sed 's/log//g'`; do ls $i*result | grep "No such"; done

## Crop the pdf figures to insert into paper
for i in *pdf ; do echo $i ; pdfcrop $i $i; done

########################################################################
## VALIDATION x86_64 - SPEC CPU 2000 and 2006
########################################################################

# Run the Base and DSBP for all SPEC2000 and SPEC2006
rm ~/Experiment/benchmarks/results/spec_cpu2000/*Validation*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*Validation*
rm ~/Experiment/benchmarks/results/spec_cpu2000_x86_32/*Validation*
rm ~/Experiment/benchmarks/results/spec_cpu2006_x86_32/*Validation*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1core.cfg spec_cpu2000 Validation_x86_64 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1core.cfg spec_cpu2006 Validation_x86_64 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1core.cfg spec_cpu2006_x86_32 Validation_x86_32 0 1 $i $i ; \
    $(echo -ne '\r')";
done

cd ~/Experiment/SiNUCA/scripts
rm ~/Experiment/benchmarks/plots/*
python plot.py parameters_validation.cfg
python plot.py parameters_validation_x86_32.cfg

########################################################################
## Motivation ISPASS
########################################################################
rm ~/Experiment/benchmarks/results/spec_cpu2000/*MOTIVATION_LLC*
rm ~/Experiment/benchmarks/results/spec_omp2001/*MOTIVATION_LLC*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo  MOTIVATION_LLC8MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC8MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 MOTIVATION_LLC8MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC8MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC8MB 0 8 $i $i ; \
    echo  MOTIVATION_LLC16MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 MOTIVATION_LLC16MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC16MB 0 8 $i $i ; \
    echo  MOTIVATION_LLC32MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 MOTIVATION_LLC32MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC32MB 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/results/spec_cpu2000/*MOTIVATION_*COPYBACK*
rm ~/Experiment/benchmarks/results/spec_omp2001/*MOTIVATION_*COPYBACK*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo  MOTIVATION_COPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_COPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 MOTIVATION_COPYBACK 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_COPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_COPYBACK 0 8 $i $i ; \
    echo  MOTIVATION_NOCOPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 MOTIVATION_NOCOPYBACK 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_NOCOPYBACK 0 8 $i $i ; \
    $(echo -ne '\r')";
done


rm ~/Experiment/benchmarks/plots/*MOTIVATION*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_motivation_spec_cpu2000.cfg ;
python plot.py ISPASS_motivation_spec_omp2001.cfg ;
cp ~/Experiment/benchmarks/plots/*MOTIVATION* ~/Dropbox/ISPASS/latex/Figures/

################################################################################

rm ~/Experiment/benchmarks/results/spec_cpu2006/*MOTIVATION_LLC*
rm ~/Experiment/benchmarks/results/npb_omp/*MOTIVATION_LLC*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo  MOTIVATION_LLC8MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC8MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_LLC8MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC8MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC8MB 0 8 $i $i ; \
    echo  MOTIVATION_LLC16MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_LLC16MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC16MB 0 8 $i $i ; \
    echo  MOTIVATION_LLC32MB; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_LLC32MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC32MB 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/results/spec_cpu2006/*MOTIVATION_*COPYBACK*
rm ~/Experiment/benchmarks/results/npb_omp/*MOTIVATION_*COPYBACK*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo  MOTIVATION_COPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_COPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_COPYBACK 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_COPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_COPYBACK 0 8 $i $i ; \
    echo  MOTIVATION_NOCOPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_NOCOPYBACK 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_NOCOPYBACK 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/plots/*MOTIVATION*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_motivation_spec_cpu2006.cfg ;
python plot.py ISPASS_motivation_npb_omp.cfg ;
cp ~/Experiment/benchmarks/plots/*MOTIVATION* ~/Dropbox/ISPASS/latex/Figures/


########################################################################
## SPEC CPU 2000 and 2006
########################################################################
# Plots all the benchmarks
reset;
cd ~/Experiment/SiNUCA/scripts ;
python power.py spec_cpu2000 BASE BASE_200M_spec_cpu2000 ;
python power.py spec_cpu2000 DLEC DLEC_200M_spec_cpu2000 ;
python power.py spec_cpu2006 BASE BASE_200M_spec_cpu2006 ;
python power.py spec_cpu2006 DLEC DLEC_200M_spec_cpu2006 ;
python plot.py parameters_spec2000.cfg ;
python plot.py parameters_spec2006.cfg

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 BASE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 DLEC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC 0 1 $i $i ; \
    $(echo -ne '\r')";
done

# Run the Base and DSBP for all SPEC2000
for i in `seq 1 26` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 DSBP 0 1 $i $i ; \
    $(echo -ne '\r')";
done

# Run the Base and DSBP for all SPEC2006
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 DSBP 0 1 $i $i ; \
    $(echo -ne '\r')";
done

########################################################################
## NPB_OMP and SPEC OMP 2001
########################################################################

# Plots all the benchmarks
reset;
cd ~/Experiment/SiNUCA/scripts ;
python power.py npb_omp BASE BASE_200M_npb_omp ;
python power.py npb_omp DLEC DLEC_200M_npb_omp ;
python power.py spec_omp2001 BASE BASE_200M_spec_omp2001 ;
python power.py spec_omp2001 DLEC DLEC_200M_spec_omp2001 ;
python plot.py parameters_npb_omp.cfg ;
python plot.py parameters_spec_omp2001.cfg ;


# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC 0 8 $i $i ; \
    $(echo -ne '\r')";
done


# Run the Base and DSBP for all NPB_OMP
for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DSBP 0 8 $i $i ; \
    $(echo -ne '\r')";
done


# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DSBP 0 8 $i $i ; \
    $(echo -ne '\r')";
done

########################################################################
# TRACES and PIN_POINTS

# Create pin_point traces for all SPEC2000
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean spec_cpu2000 1 0 0 ; \
python create_pin_points_trace.py prepare spec_cpu2000 1 0 0 ; \
$(echo -ne '\r')";
for i in `seq 1 26` ; do
    byobu -p$i -X stuff "reset ; \
    echo $i ; \
    cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
    python create_pin_points_trace.py pin_point spec_cpu2000 1 $i $i ; \
    python create_pin_points_trace.py trace spec_cpu2000 1 $i $i ; \
    $(echo -ne '\r')";
done

# Create pin_points, traces for all SPEC2006
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean spec_cpu2006 1 0 0 ; \
python create_pin_points_trace.py prepare spec_cpu2006 1 0 0 ; \
$(echo -ne '\r')";
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    echo $i ; \
    cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
    python create_pin_points_trace.py pin_point spec_cpu2006 1 $i $i ; \
    python create_pin_points_trace.py trace spec_cpu2006 1 $i $i ; \
    $(echo -ne '\r')";
done

# Create pin_point traces for all NPB_OMP - 8 Threads
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 8 1 11 ; \
$(echo -ne '\r')";


# Create pin_point traces for all SPEC_OMP2001 - 8 Threads
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 8 1 11 ; \
$(echo -ne '\r')";


# Create pin_point traces for all NPB_OMP and SPEC_OMP2001 - 1,2,4,8 Threads
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 1 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 1 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 2 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 2 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 4 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 4 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 8 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 8 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 16 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 16 1 11 ; \
$(echo -ne '\r')";
