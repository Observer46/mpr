#!/bin/bash
module add plgrid/tools/openmpi
for size in 1e6 1e7 1e8 1e9 1e10 1e11 1e12 1e13
do
    for np in {1..12}
    do
        mpiexec -np $np ./mc_pi.out $size 1
    done
done
