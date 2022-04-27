#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=00:30:00
#SBATCH --partition=plgrid-short
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2
gcc -fopenmp -o bucket125.out bucket_sort_3_125k.c
gcc -fopenmp -o bucket250.out bucket_sort_3_250k.c
gcc -fopenmp -o bucket375.out bucket_sort_3_375k.c
gcc -fopenmp -o bucket500.out bucket_sort_3_500k.c
gcc -fopenmp -o bucket750.out bucket_sort_3_750k.c
gcc -fopenmp -o bucket1000.out bucket_sort_3_1000k.c
gcc -fopenmp -o bucket1250.out bucket_sort_3_1250k.c
gcc -fopenmp -o bucket1500.out bucket_sort_3_1500k.c
gcc -fopenmp -o bucket1750.out bucket_sort_3_1750k.c
gcc -fopenmp -o bucket2000.out bucket_sort_3_2000k.c
for attempt in {1..25}
do
    ./bucket125.out 1 10000000
    ./bucket250.out 1 10000000
    ./bucket375.out 1 10000000
    ./bucket500.out 1 10000000
    ./bucket750.out 1 10000000
    ./bucket1000.out 1 10000000
    ./bucket1250.out 1 10000000
    ./bucket1500.out 1 10000000
    ./bucket1750.out 1 10000000
    ./bucket2000.out 1 10000000
done
