########################################################################
## INCLUSIVENESS ISPASS - VK2 - SPEC_CPU2006
########################################################################
# Plots all the benchmarks
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.MOTIVATION_LLC*MB.*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.MOTIVATION_*COPYBACK.*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.BASE_INCLUSIVE_LLC.*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.BASE_NON_INCLUSIVE.*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.DLEC_ALL_INCLUSIVE_LLC.*
rm ~/Experiment/benchmarks/results/spec_cpu2006/*.DLEC_ALL_NON_INCLUSIVE.*
# Run the experiments
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_INCLUSIVE_LLC 0 1 $i $i ; \
    echo  MOTIVATION_LLC; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_LLC16MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_LLC32MB 0 1 $i $i ; \
    echo  MOTIVATION_COPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 MOTIVATION_NOCOPYBACK 0 1 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_NON_INCLUSIVE 0 1 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL256_INCLUSIVE_LLC 0 1 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_EVICTION-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_EVICTION_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_COPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_COPYBACK_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_TURNOFF-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_TURNOFF_INCLUSIVE_LLC 0 1 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_INCLUSIVE_LLC_AGGRESSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_INCLUSIVE_LLC_AGGRESSIVE 0 1 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_NON_INCLUSIVE_AGGRESSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_NON_INCLUSIVE_AGGRESSIVE 0 1 $i $i ; \
    $(echo -ne '\r')";
done



rm ~/Experiment/benchmarks/plots/*MOTIVATION*
rm ~/Experiment/benchmarks/plots/*INCLUSIVENESS*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_motivation_spec_cpu2006.cfg ;
python plot.py ISPASS_inclusiveness_spec_cpu2006.cfg ;
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.data ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.pdf ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/


########################################################################
## INCLUSIVENESS ISPASS - VK3 - SPEC_OMP2001
########################################################################
# Plots all the benchmarks
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.MOTIVATION_LLC*MB.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.MOTIVATION_*COPYBACK.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.DLEC_ALL_NON_INCLUSIVE.*
# Run the experiments
for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_INCLUSIVE_LLC 0 1 $i $i ; \
    echo  MOTIVATION_LLC; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC16MB 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC32MB 0 1 $i $i ; \
    echo  MOTIVATION_COPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_NOCOPYBACK 0 1 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_NON_INCLUSIVE 0 1 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL256_INCLUSIVE_LLC 0 8 $i $i ; \
    $(echo -ne '\r')";
done


for i in `seq 1 11` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_INCLUSIVE_LLC_AGGRESSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_INCLUSIVE_LLC_AGGRESSIVE 0 8 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_NON_INCLUSIVE_AGGRESSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_NON_INCLUSIVE_AGGRESSIVE 0 8 $i $i ; \
    $(echo -ne '\r')";
done


byobu -p1 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_INCLUSIVE_LLC 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p2 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_INCLUSIVE_LLC 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p3 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC16MB 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p4 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC32MB 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p5 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_NOCOPYBACK 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p6 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_NON_INCLUSIVE 0 8 1 6 ; $(echo -ne '\r')" ;
byobu -p7 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_NON_INCLUSIVE 0 8 1 6 ; $(echo -ne '\r')" ;
echo ================== ;
echo MIDDLE ;
echo ================== ;
byobu -p8 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_INCLUSIVE_LLC 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p9 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_INCLUSIVE_LLC 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p10 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC16MB 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p11 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_LLC32MB 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p12 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 MOTIVATION_NOCOPYBACK 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p13 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_NON_INCLUSIVE 0 8 7 11 ; $(echo -ne '\r')" ;
byobu -p14 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_NON_INCLUSIVE 0 8 7 11 ; $(echo -ne '\r')" ;



rm ~/Experiment/benchmarks/plots/*MOTIVATION*
rm ~/Experiment/benchmarks/plots/*INCLUSIVENESS*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_motivation_spec_omp2001.cfg ;
python plot.py ISPASS_inclusiveness_spec_omp2001.cfg ;
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.data ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.pdf ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/



########################################################################
## INCLUSIVENESS ISPASS - VK4 - NPB_OMP
########################################################################
# Plots all the benchmarks
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.MOTIVATION_LLC*MB.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.MOTIVATION_*COPYBACK.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.DLEC_ALL_NON_INCLUSIVE.*
# Run the experiments
for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_INCLUSIVE_LLC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_INCLUSIVE_LLC 0 8 $i $i ; \
    echo  MOTIVATION_LLC; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC16MB 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC32MB 0 8 $i $i ; \
    echo  MOTIVATION_COPYBACK; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_NOCOPYBACK 0 8 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_NON_INCLUSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_NON_INCLUSIVE 0 8 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL256_INCLUSIVE_LLC 0 8 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 9` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    echo INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_INCLUSIVE_LLC_AGGRESSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_INCLUSIVE_LLC_AGGRESSIVE 0 8 $i $i ; \
    echo NON_INCLUSIVE; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_NON_INCLUSIVE_AGGRESSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_NON_INCLUSIVE_AGGRESSIVE 0 8 $i $i ; \
    $(echo -ne '\r')";
done


byobu -p1 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_INCLUSIVE_LLC 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p2 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_INCLUSIVE_LLC 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p3 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC16MB 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p4 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC32MB 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p5 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_NOCOPYBACK 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p6 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_NON_INCLUSIVE 0 8 1 5 ; $(echo -ne '\r')" ;
byobu -p7 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_NON_INCLUSIVE 0 8 1 5 ; $(echo -ne '\r')" ;
echo ================== ;
echo MIDDLE ;
echo ================== ;
byobu -p8 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_INCLUSIVE_LLC 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p9 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_INCLUSIVE_LLC 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p10 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC16MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC16MB 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p11 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_LLC32MB-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_LLC32MB 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p12 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-MOTIVATION_NOCOPYBACK-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp MOTIVATION_NOCOPYBACK 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p13 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_NON_INCLUSIVE 0 8 6 9 ; $(echo -ne '\r')" ;
byobu -p14 -X stuff "cd ~/Experiment/SiNUCA/scripts; \
python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_NON_INCLUSIVE 0 8 6 9 ; $(echo -ne '\r')" ;




rm ~/Experiment/benchmarks/plots/*MOTIVATION*
rm ~/Experiment/benchmarks/plots/*INCLUSIVENESS*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_motivation_npb_omp.cfg ;
python plot.py ISPASS_inclusiveness_npb_omp.cfg ;
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.data ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.pdf ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
