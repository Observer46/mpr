#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=01:00:00
#SBATCH --partition=plgrid
#SBATCH --account=plgmpr22
module add plgrid/tools/openmpi
mpicc -o ex1 ex1.c
mpiexec -np 12 ./ex1
