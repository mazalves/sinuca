# rename changing the original filename
for i in *; do echo $i; j=`echo $i | sed -e 's/Base\./BaseN\./g'`; echo $j; mv $i $j ; done
for i in *; do echo $i; j=`echo $i | sed -e 's/PP/PP200M/g'`; echo $j; mv $i $j ; done

#Send multiple commands to byobu
for i in `seq 1 30` ; do
    byobu -p$i -X stuff "reset ; echo $i \
                                teste $(echo -ne '\r')";
done

# Redirect the python script into a file
python 3>&1 4>&2 >foo.txt 2>&1

# Highlight the source code
highlight -i printable_directory.cpp -o printable_directory.html -S cpp -l -doc --style seashell


# CREATE Fake Trace
echo \#FAKE_TRACE > fake_trace.tid0.mem.out
for i in `seq 1 10`; do for i in `seq 64 64 640000`; do echo W 8 $i 1 >> fake_trace.tid0.mem.out; done ; done

echo \#FAKE_TRACE > fake_trace.tid0.dyn.out
for i in `seq 8 8 80000`; do echo 1 >> fake_trace.tid0.dyn.out; done

echo \#FAKE_TRACE > fake_trace.tid0.stat.out
echo \@1 >> fake_trace.tid0.stat.out
for i in `seq 64 64 640`; do echo MOV 9 0x00$i 16 0 0 0 0 1 0 0 0 >> fake_trace.tid0.stat.out; done

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

#Check the footprint of a memory trace
# zcat art.M_1t.tid0.mem.out.gz | awk '{print rshift($3,6)}' | sort | uniq | wc -l
for i in `ls *_8t.tid*.mem*` ; do echo $i >> FOOTPRINT.txt ; time zcat $i | awk '{print rshift($3,6)}' | sort -n | uniq | wc -l >> FOOTPRINT.txt; done

# Check the average memory reuse of a memory trace
# awk '{tot += $1; count++} END { print tot/count}'
for i in `ls *_8t.tid*.mem*` ; do echo $i >> REUSE.txt ; time zcat $i | awk '{print rshift($3,6)}' | sort -n | uniq -c | awk '{tot += $1; count++} END { print tot/count}' >> REUSE.txt; done

# Check the average difference between the addresses
# awk 's{print ((s>$3)?s-$3:$3-s)}{s=$3}'
for i in `ls *_8t.tid*.mem*` ; do echo $i >> DIFF.txt ; time zcat $i | awk '{print rshift($3,6)}' | sort -n | uniq -c | awk 's{print ((s>$0)?s-$0:$0-s)}{s=$0}' | awk '{tot += $1; count++} END { print tot/count}' >> DIFF.txt; done


# Check the progress of multiple simulations
for i in `ls ~/Experiment/benchmarks/results/*/*.log`; do tail -n16 $i | grep 'CPU  0' | grep -v 100.000%| grep IPC -m1 ; done
## Shows the benchmarks which failed.
for i in `ls ~/Experiment/benchmarks/results/*/*.log | sed 's/log//g'`; do ls $i*result | grep "No such"; done

# Split multiple results to different files
for i in *.result ; do cat $i | awk '/Configuration of SINUCA_ENGINE/{n++}{print > $i_ n}' ; done
for i in *.result ; do echo $i ; awk '/Configuration\ of\ SINUCA\_ENGINE/{n++}{print > f n}' f=$i $i; done
awk '/Configuration of SINUCA_ENGINE/{n++}{print > f n}' f=*.result

## Crop the pdf figures to insert into paper
for i in *pdf ; do echo $i ; pdfcrop $i $i; done

## Real time for validation of the simulator - Size S
rm -f npb_S_1t.txt npb_S_2t.txt npb_S_4t.txt npb_S_8t.txt ;
for i in `ls *S.x` ; do
    echo $i >> npb_S_1t.txt;
    echo $i >> npb_S_2t.txt;
    echo $i >> npb_S_4t.txt;
    echo $i >> npb_S_8t.txt;
    for j in `seq 0 9` ; do
        sudo GOMP_CPU_AFFINITY=0 OMP_NUM_THREADS=1 perf stat -x " " -a -A -e cycles -o npb_S_1t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2 OMP_NUM_THREADS=2 perf stat -x " " -a -A -e cycles -o npb_S_2t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6 OMP_NUM_THREADS=4 perf stat -x " " -a -A -e cycles -o npb_S_4t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6,1,3,5,7 OMP_NUM_THREADS=8 perf stat -x " " -a -A -e cycles -o npb_S_8t.txt --append ./$i > /dev/null;
    done ;
done;
for i in `ls npb_S_*t.txt` ; do
    sed -i -e 's/CPU0 //' -e 's/CPU1 //' -e 's/CPU2 //' -e 's/CPU3 //' -e 's/CPU4 //' -e 's/CPU5 //' -e 's/CPU6 //' -e 's/CPU7 //' $i;
    sed -i -e 's/ cycles//' $i;
    sed -i '/#/d' $i;
done;

## Real time for validation of the simulator - Size W
rm -f npb_W_1t.txt npb_W_2t.txt npb_W_4t.txt npb_W_8t.txt ;
for i in `ls *W.x` ; do
    echo $i >> npb_W_1t.txt;
    echo $i >> npb_W_2t.txt;
    echo $i >> npb_W_4t.txt;
    echo $i >> npb_W_8t.txt;
    for j in `seq 0 9` ; do
        sudo GOMP_CPU_AFFINITY=0 OMP_NUM_THREADS=1 perf stat -x " " -a -A -e cycles -o npb_W_1t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2 OMP_NUM_THREADS=2 perf stat -x " " -a -A -e cycles -o npb_W_2t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6 OMP_NUM_THREADS=4 perf stat -x " " -a -A -e cycles -o npb_W_4t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6,1,3,5,7 OMP_NUM_THREADS=8 perf stat -x " " -a -A -e cycles -o npb_W_8t.txt --append ./$i > /dev/null;
    done ;
done;
for i in `ls npb_W_*t.txt` ; do
    sed -i -e 's/CPU0 //' -e 's/CPU1 //' -e 's/CPU2 //' -e 's/CPU3 //' -e 's/CPU4 //' -e 's/CPU5 //' -e 's/CPU6 //' -e 's/CPU7 //' $i;
    sed -i -e 's/ cycles//' $i;
    sed -i '/#/d' $i;
done;

## Real time for validation of the simulator - Size A
rm -f npb_A_1t.txt npb_A_2t.txt npb_A_4t.txt npb_A_8t.txt ;
for i in `ls *A.x` ; do
    echo $i >> npb_A_1t.txt;
    echo $i >> npb_A_2t.txt;
    echo $i >> npb_A_4t.txt;
    echo $i >> npb_A_8t.txt;
    for j in `seq 0 9` ; do
        sudo GOMP_CPU_AFFINITY=0 OMP_NUM_THREADS=1 perf stat -x " " -a -A -e cycles -o npb_A_1t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2 OMP_NUM_THREADS=2 perf stat -x " " -a -A -e cycles -o npb_A_2t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6 OMP_NUM_THREADS=4 perf stat -x " " -a -A -e cycles -o npb_A_4t.txt --append ./$i > /dev/null;
        sudo GOMP_CPU_AFFINITY=0,2,4,6,1,3,5,7 OMP_NUM_THREADS=8 perf stat -x " " -a -A -e cycles -o npb_A_8t.txt --append ./$i > /dev/null;
    done ;
done;
for i in `ls npb_A_*t.txt` ; do
    sed -i -e 's/CPU0 //' -e 's/CPU1 //' -e 's/CPU2 //' -e 's/CPU3 //' -e 's/CPU4 //' -e 's/CPU5 //' -e 's/CPU6 //' -e 's/CPU7 //' $i;
    sed -i -e 's/ cycles//' $i;
    sed -i '/#/d' $i;
done;



########################################################################
## VALIDATION x86_64 - SPEC CPU 2000 and 2006
########################################################################

# Run the Validation for all SPEC2000 and SPEC2006
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2000/*Validation_x86_64*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006/*Validation_x86_64*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006_x86_32/*Validation*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo spec_cpu2006
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core/Core2Duo_1core.cfg spec_cpu2006 Validation_x86_64 0 1 $i $i ; \
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core_stride/Core2Duo_1core.cfg spec_cpu2006 Validation_x86_64_stride 0 1 $i $i ; \
    echo spec_cpu2000
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core/Core2Duo_1core.cfg spec_cpu2000 Validation_x86_64 0 1 $i $i ; \
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core_stride/Core2Duo_1core.cfg spec_cpu2000 Validation_x86_64_stride 0 1 $i $i ; \
    echo spec_cpu2006_x86_32
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core/Core2Duo_1core.cfg spec_cpu2006_x86_32 Validation_x86_32 0 1 $i $i ; \
        python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Core2Duo_1Core_stride/Core2Duo_1core.cfg spec_cpu2006_x86_32 Validation_x86_32_stride 0 1 $i $i ; \
    $(echo -ne '\r')";
done

cd ~/Experiment/SiNUCA/scripts
rm ~/Experiment/benchmarks/plots/*
python plot.py parameters_validation.cfg
python plot.py parameters_validation_stride.cfg

#$ lstopo
#$ export GOMP_CPU_AFFINITY=0,2,4,6,1,3,5,7

# Run the Validation for all NPB_OMP 1, 2, 4, 8 threads for size S.
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*S.Validation*
for i in `seq 19 27` ; do

# ~ rm ~/Experiment/benchmarks/results/npb_omp/*W.Validation*
for i in `seq 10 18` ; do

# ~ rm ~/Experiment/benchmarks/results/npb_omp/*A.Validation*
for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Nehalem_2x4cores/main_8cores_8cachel2_2cacheL3.cfg npb_omp Validation_1t 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Nehalem_2x4cores/main_8cores_8cachel2_2cacheL3.cfg npb_omp Validation_2t 0 2 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Nehalem_2x4cores/main_8cores_8cachel2_2cacheL3.cfg npb_omp Validation_4t 0 4 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/validation/Nehalem_2x4cores/main_8cores_8cachel2_2cacheL3.cfg npb_omp Validation_8t 0 8 $i $i ; \
    $(echo -ne '\r')";
done

cd ~/Experiment/SiNUCA/scripts
rm ~/Experiment/benchmarks/plots/*
python plot.py parameters_validation_multithreaded.cfg

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


# Create pin_point traces for all NPB_OMP (A) and SPEC_OMP2001 - 1,2,4,8,16 Threads
byobu -p0 -X stuff \
"reset ; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 1 1 9 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 1 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 2 1 9 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 2 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 4 1 9 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 4 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 8 1 9 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 8 1 11 ; \
reset;
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 16 1 9 ; \
python create_pin_points_trace.py clean spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py prepare spec_omp2001 1 0 0 ; \
python create_pin_points_trace.py parallel_trace spec_omp2001 16 1 11 ; \
$(echo -ne '\r')";

# Create pin_point traces for all NPB_OMP (W) - 1,2,4,8 Threads
byobu -p0 -X stuff \
"reset ; \
export GOMP_CPU_AFFINITY=0; \
cd ~/Experiment/SiNUCA/trace_generator/source/tools/sinuca_tracer/scripts ; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 1 10 18 ; \
echo `uname -a`--`date`--TRACE_1t_READY >> ~/Dropbox/TRACE.READY ; \
reset;
export GOMP_CPU_AFFINITY=0,2; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 2 10 18 ; \
echo `uname -a`--`date`--TRACE_2t_READY >> ~/Dropbox/TRACE.READY ; \
reset;
export GOMP_CPU_AFFINITY=0,2,4,6; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 4 10 18 ; \
echo `uname -a`--`date`--TRACE_4t_READY >> ~/Dropbox/TRACE.READY ; \
reset;
export GOMP_CPU_AFFINITY=0,2,4,6,1,3,5,7; \
python create_pin_points_trace.py clean npb_omp 1 0 0 ; \
python create_pin_points_trace.py prepare npb_omp 1 0 0 ; \
python create_pin_points_trace.py parallel_trace npb_omp 8 10 18 ; \
echo `uname -a`--`date`--TRACE_8t_READY >> ~/Dropbox/TRACE.READY ; \
echo `uname -a`--`date`--ALL_CLEAR >> ~/Dropbox/TRACE.READY ; \
$(echo -ne '\r')";

