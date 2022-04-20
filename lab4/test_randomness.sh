#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 10
#SBATCH --time=00:55:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
gcc -fopenmp -o random.out analyze_random.c
for proc in {1..10}
do 
    for bucket_overhead in 2 3 4 4.5 5 5.5 6 7 8 9 10
    do
        for bucket_size in 3 5 10 15 20 25
        do
            for p_size in 10000 100000 1000000 10000000 100000000
            do
                ./random.out ${proc} ${p_size} ${bucket_overhead} ${bucket_size} 5
            done
        done
    done
done