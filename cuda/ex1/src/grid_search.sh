#!/bin/bash

for size in 100000 200000 300000 400000 500000 600000 700000 800000 900000 1000000
do
    for blocks in 50 100 150 200 250 300 350 400 450 500
    do
        for threads in 1 5 10 25 50 75 100 200 300 400 500
        do
            ./vectorAdd.exe $size c $blocks $threads
        done 
    done
done