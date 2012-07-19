# Plots all the benchmarks
reset;
python power.py spec_cpu2000 Base base_200M_spec_cpu2000 ;
python power.py spec_cpu2000 DSBP dsbp_200M_spec_cpu2000 ;
python power.py spec_cpu2006 Base base_200M_spec_cpu2006 ;
python power.py spec_cpu2006 DSBP dsbp_200M_spec_cpu2006 ;
python plot.py parameters_spec2000.cfg ;
python plot.py parameters_spec2006.cfg

# Run the Base and DSBP for all SPEC2000 and SPEC2006
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 DSBP 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 Base 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 DSBP 0 1 $i $i ; \
    $(echo -ne '\r')";
done


# Plots all the benchmarks
reset;
python power.py npb_omp Base base_200M_npb_omp ;
python power.py npb_omp DSBP dsbp_200M_npb_omp ;
python plot.py parameters_npb_omp.cfg ;


# Run the Base and DSBP for all NPB_OMP
for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-Baseline-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp Base 0 8 $i $i ; \
    $(echo -ne '\r')";
    byobu -p1$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-DSBP-8CoresNoPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DSBP 0 8 $i $i ; \
    $(echo -ne '\r')";
done


for i in *pdf ; do echo $i ; pdfcrop $i $i; done



# Run the Base and DSBP for all SPEC2000
for i in `seq 1 26` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 BaseN 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2000 DSBPN 0 1 $i $i ; \
    $(echo -ne '\r')";
done

# Run the Base and DSBP for all SPEC2006
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-Baseline-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 BaseN 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/configurations/SBAC-DSBP-1CoreNoPrefetch/main_1core_1cachel2_1cachel3.cfg spec_cpu2006 DSBPN 0 1 $i $i ; \
    $(echo -ne '\r')";
done




# rename changing the original filename 
for i in *; do echo $i; j=`echo $i | sed -e 's/Base\./BaseN\./g'`; echo $j; mv $i $j ; done
for i in *; do echo $i; j=`echo $i | sed -e 's/PP/PP200M/g'`; echo $j; mv $i $j ; done




# Create pin_point traces all SPEC2000 and SPEC2006
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean spec_cpu2000 1 0 0 ; \
python create_pin_points_trace.py prepare spec_cpu2000 1 0 0 ; \
python create_pin_points_trace.py clean spec_cpu2006 1 0 0 ; \
python create_pin_points_trace.py prepare spec_cpu2006 1 0 0 ; \
$(echo -ne '\r')";
for i in `seq 1 26` ; do
    byobu -p$i -X stuff "reset ; \
    echo $i ; \
    cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
    python create_pin_points_trace.py pin_point spec_cpu2000 1 $i $i ; \
    python create_pin_points_trace.py trace spec_cpu2000 1 $i $i ; \
    python create_pin_points_trace.py pin_point spec_cpu2006 1 $i $i ; \
    python create_pin_points_trace.py trace spec_cpu2006 1 $i $i ; \
    $(echo -ne '\r')";
done




# Create pin_point traces all SPEC2000
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

# Create pin_points, traces and run the Base and DSBP for all SPEC2006
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


#Send multiple commands to byobu
for i in `seq 1 30` ; do
    byobu -p$i -X stuff "reset ; echo $i \
                                teste $(echo -ne '\r')";
done


## Arithmetic, use (( ))
marco=$((16 % 8))
