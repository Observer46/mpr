#!/bin/bash
module add plgrid/tools/openmpi
mpicc -o mc_pi.out mc_pi.c
for size in 1e6 1e8 1e10 1e11 1e12 1e13
do
    for np in {1..12}
    do
        mpiexec -np $np ./mc_pi.out $size 1
    done
done
