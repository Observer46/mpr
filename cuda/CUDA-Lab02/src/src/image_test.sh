#!/bin/bash

for scale in 2.0 3.0
do
    for image in voyager2.pgm aerosmith-double.pgm
    do
        for blocks in 10 20 30 40 50 60 70 80 90 100
        do
            for threads in 4 8 12 16 20 24 28 32
            do
                ./image_scaling.exe $blocks $threads $image $image-output $scale
            done
        done
    done
done