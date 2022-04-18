#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 10
#SBATCH --time=00:50:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
gcc -fopenmp -o bucket.out bucket_sort_3.c
for proc in {1..10}
do 
    for p_size in 10000 100000 1000000 10000000 100000000
    do
        ./bucket.out ${proc} ${p_size}
    done
done