# rename changing the original filename
for i in *; do echo $i; j=`echo $i | sed -e 's/Base\./BaseN\./g'`; echo $j; mv $i $j ; done
for i in *; do echo $i; j=`echo $i | sed -e 's/PP/PP200M/g'`; echo $j; mv $i $j ; done

#Send multiple commands to byobu
for i in `seq 1 30` ; do
    byobu -p$i -X stuff "reset ; echo $i \
                                teste $(echo -ne '\r')";
done

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

## Shows the benchmarks which failed.
for i in `ls *.log | sed 's/log//g'`; do ls $i*result | grep "No such"; done

########################################################################
## SPEC CPU 2000 and 2006
########################################################################
# Plots all the benchmarks
reset;
python power.py spec_cpu2000 Base base_200M_spec_cpu2000 ;
python power.py spec_cpu2000 DSBP_Reinstall dsbp_200M_spec_cpu2000 ;
python power.py spec_cpu2000 DSBP_NoReinstall dsbp_200M_spec_cpu2000 ;
python power.py spec_cpu2006 Base base_200M_spec_cpu2006 ;
python power.py spec_cpu2006 DSBP_Reinstall dsbp_200M_spec_cpu2006 ;
python power.py spec_cpu2006 DSBP_NoReinstall dsbp_200M_spec_cpu2006 ;
python plot.py parameters_spec2000.cfg ;
python plot.py parameters_spec2006.cfg

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncALL_DSBP_NoReinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncALL_DSBP_NoReinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncLLC_DSBP_NoReinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncLLC_DSBP_NoReinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 NonInc_DSBP_NoReinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 NonInc_DSBP_NoReinstall 0 1 $i $i ; \

    $(echo -ne '\r')";
done


# Run the Base and DSBP for all SPEC2000 and SPEC2006
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncALL_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncALL_DSBP_Reinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncALL_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncALL_DSBP_Reinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncLLC_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 IncLLC_DSBP_Reinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncLLC_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 IncLLC_DSBP_Reinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 NonInc_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 NonInc_DSBP_Reinstall 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 NonInc_Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 NonInc_DSBP_Reinstall 0 1 $i $i ; \
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
python power.py npb_omp Base base_200M_npb_omp ;
python power.py npb_omp DSBP_Reinstall dsbp_200M_npb_omp ;
python power.py npb_omp DSBP_NoReinstall dsbp_200M_npb_omp ;
python power.py spec_omp2001 Base base_200M_spec_omp2001 ;
python power.py spec_omp2001 DSBP_Reinstall dsbp_200M_spec_omp2001 ;
python power.py spec_omp2001 DSBP_NoReinstall dsbp_200M_spec_omp2001 ;
python plot.py parameters_npb_omp.cfg ;
python plot.py parameters_spec_omp2001.cfg ;


for i in *pdf ; do echo $i ; pdfcrop $i $i; done


# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncALL_DSBP_NoReinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncALL_DSBP_NoReinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncLLC_DSBP_NoReinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncLLC_DSBP_NoReinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp NonInc_DSBP_NoReinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 NonInc_DSBP_NoReinstall 0 8 $i $i ; \
    $(echo -ne '\r')";
done


# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncALL_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncALL_DSBP_Reinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncALL_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_ALL/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncALL_DSBP_Reinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncLLC_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp IncLLC_DSBP_Reinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncLLC_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Inclusive_LLC/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 IncLLC_DSBP_Reinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp NonInc_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp NonInc_DSBP_Reinstall 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 NonInc_Base 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/Non_Inclusive/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 NonInc_DSBP_Reinstall 0 8 $i $i ; \
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



byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 4 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 4 1 11 ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 16 1 11 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 16 1 11 ; \
$(echo -ne '\r')";
