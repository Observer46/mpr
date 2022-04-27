#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=00:20:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
gcc -fopenmp -o bucket.out bucket_sort_3.c
for attempt in {1..25}
do
    for proc in {1..12}
    do 
        ./bucket.out ${proc} 10000000 --verbose
    done
done