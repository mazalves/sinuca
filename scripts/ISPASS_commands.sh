########################################################################
## INCLUSIVENESS ISPASS - VK3
########################################################################
# Plots all the benchmarks
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2000/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2000/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2000/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2000/*.DLEC_ALL_NON_INCLUSIVE.*
# ~
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_omp2001/*.DLEC_ALL_NON_INCLUSIVE.*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 BASE_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_INCLUSIVE_LLC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 BASE_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE_NON_INCLUSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 DLEC_ALL_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_INCLUSIVE_LLC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 DLEC_ALL_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_ALL_NON_INCLUSIVE 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/plots/*INCLUSIVENESS*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_inclusiveness_spec_cpu2000.cfg ;
python plot.py ISPASS_inclusiveness_spec_omp2001.cfg ;
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.data ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.pdf ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/



########################################################################
## INCLUSIVENESS ISPASS - VK4
########################################################################
# Plots all the benchmarks
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/spec_cpu2006/*.DLEC_ALL_NON_INCLUSIVE.*
# ~
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.BASE_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.BASE_NON_INCLUSIVE.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.DLEC_ALL_INCLUSIVE_LLC.*
# ~ rm ~/Experiment/benchmarks/results/npb_omp/*.DLEC_ALL_NON_INCLUSIVE.*
# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_INCLUSIVE_LLC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE_NON_INCLUSIVE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_INCLUSIVE_LLC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_ALL-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_INCLUSIVE_LLC 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_ALL_NON_INCLUSIVE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC_NON_INCLUSIVE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_ALL_NON_INCLUSIVE 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/plots/*INCLUSIVENESS*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_inclusiveness_spec_cpu2006.cfg ;
python plot.py ISPASS_inclusiveness_npb_omp.cfg ;
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.data ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/
cp ~/Experiment/benchmarks/plots/*INCLUSIVENESS*.pdf ~/Dropbox/ISPASS/latex/Figures/Inclusiveness/

########################################################################
## Motivation ISPASS - VK3
########################################################################
#rm ~/Experiment/benchmarks/results/spec_cpu2000/*MOTIVATION_LLC*
#rm ~/Experiment/benchmarks/results/spec_omp2001/*MOTIVATION_LLC*
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

#rm ~/Experiment/benchmarks/results/spec_cpu2000/*MOTIVATION_*COPYBACK*
#rm ~/Experiment/benchmarks/results/spec_omp2001/*MOTIVATION_*COPYBACK*
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
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/

########################################################################
## Motivation ISPASS - VK4
########################################################################
#rm ~/Experiment/benchmarks/results/spec_cpu2006/*MOTIVATION_LLC*
#rm ~/Experiment/benchmarks/results/npb_omp/*MOTIVATION_LLC*
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

#rm ~/Experiment/benchmarks/results/spec_cpu2006/*MOTIVATION_*COPYBACK*
#rm ~/Experiment/benchmarks/results/npb_omp/*MOTIVATION_*COPYBACK*
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
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_LLC*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_LLC_Size/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*COPYBACK*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_No_Copyback/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.data ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/
cp ~/Experiment/benchmarks/plots/*MOTIVATION_*ENERGY*.pdf ~/Dropbox/ISPASS/latex/Figures/Motivation_Energy/


########################################################################
## AHT_C OFFSET/PC ISPASS - VK3
########################################################################
# Plots all the benchmarks
#rm ~/Experiment/benchmarks/results/spec_cpu2000/*BASE*
#rm ~/Experiment/benchmarks/results/spec_cpu2000/*DLEC_NEC_OFFSET*
#rm ~/Experiment/benchmarks/results/spec_cpu2000/*DLEC_NEC_PC*
#rm ~/Experiment/benchmarks/results/spec_omp2001/*BASE*
#rm ~/Experiment/benchmarks/results/spec_omp2001/*DLEC_NEC_OFFSET*
#rm ~/Experiment/benchmarks/results/spec_omp2001/*DLEC_NEC_PC*
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 BASE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 BASE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 DLEC_NEC_OFFSET 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_NEC_OFFSET 0 8 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2000 DLEC_NEC_PC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_omp2001 DLEC_NEC_PC 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/plots/*PC_OFFSET*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_pc_offset_spec_cpu2000.cfg ;
python plot.py ISPASS_pc_offset_spec_omp2001.cfg ;
cp ~/Experiment/benchmarks/plots/*PC_OFFSET*.data ~/Dropbox/ISPASS/latex/Figures/NEC_AHTC_PC_or_PC_Offset/
cp ~/Experiment/benchmarks/plots/*PC_OFFSET*.pdf ~/Dropbox/ISPASS/latex/Figures/NEC_AHTC_PC_or_PC_Offset/



########################################################################
## AHT_C OFFSET/PC ISPASS - VK4
########################################################################
# Plots all the benchmarks
#rm ~/Experiment/benchmarks/results/spec_cpu2006/*BASE*
#rm ~/Experiment/benchmarks/results/spec_cpu2006/*DLEC_NEC_OFFSET*
#rm ~/Experiment/benchmarks/results/spec_cpu2006/*DLEC_NEC_PC*
#rm ~/Experiment/benchmarks/results/npb_omp/*BASE*
#rm ~/Experiment/benchmarks/results/npb_omp/*DLEC_NEC_OFFSET*
#rm ~/Experiment/benchmarks/results/npb_omp/*DLEC_NEC_PC*
# Run the Base and DSBP for all SPEC_OMP2001
for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 BASE 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-BASE-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp BASE 0 8 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_NEC_OFFSET 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_NEC_OFFSET 0 8 $i $i ; \
    $(echo -ne '\r')";
done

for i in `seq 1 29` ; do
    byobu -p$i -X stuff "reset ; \
    cd ~/Experiment/SiNUCA/scripts ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg spec_cpu2006 DLEC_NEC_PC 0 1 $i $i ; \
    python execute.py ~/Experiment/SiNUCA/examples/configurations/ISPASS-DLEC-8CoresPrefetch/main_8cores_8cachel2_1cacheL3.cfg npb_omp DLEC_NEC_PC 0 8 $i $i ; \
    $(echo -ne '\r')";
done

rm ~/Experiment/benchmarks/plots/*PC_OFFSET*
cd ~/Experiment/SiNUCA/scripts ;
python plot.py ISPASS_pc_offset_spec_cpu2006.cfg ;
python plot.py ISPASS_pc_offset_npb_omp.cfg ;
cp ~/Experiment/benchmarks/plots/*PC_OFFSET*.data ~/Dropbox/ISPASS/latex/Figures/NEC_AHTC_PC_or_PC_Offset/
cp ~/Experiment/benchmarks/plots/*PC_OFFSET*.pdf ~/Dropbox/ISPASS/latex/Figures/NEC_AHTC_PC_or_PC_Offset/
