#!/bin/bash

for matrix_size in 100 200 300 400 500 600 700 800 900 1000
do
    for blocks in 10 20 30 40 50 60 70 80 90 100
    do
        for threads in 4 8 12 16 20 24 28 32
        do
            ./matrix_transpose_2.exe $matrix_size $blocks $threads
        done
    done
done