#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=00:55:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
gcc -fopenmp -o random.out analyze_random.c
for proc in 1 5 12
do 
    for bucket_space_overhead in 2 3
    do
        for bucket_size in 10 20
        do
            ./random.out ${proc} 10000000 ${bucket_space_overhead} ${bucket_size} 100
        done
    done
done