#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=00:50:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
module add plgrid/tools/openmpi
mpicc -o mc_pi.out mc_pi.c
for size in 2e7 3.17e8 2e10
do
    for np in {1..12}
    do
        mpiexec -np $np ./mc_pi.out $size 1
    done
done
