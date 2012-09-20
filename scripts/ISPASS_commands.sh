
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
