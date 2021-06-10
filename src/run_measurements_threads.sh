#!/bin/bash

set -o xtrace

MEASUREMENTS=10
ITERATIONS=10
INITIAL_SIZE=16

SIZE=$INITIAL_SIZE

NAMES=('mandelbrot_pth')

make
mkdir results

for THREAD in 1 2 4 8 16 32; do
    for NAME in ${NAMES[@]}; do
        mkdir results/$NAME$THREAD

        for ((i=1; i<=$ITERATIONS; i++)); do
                perf stat -r $MEASUREMENTS ./$NAME -2.5 1.5 -2.0 2.0 $SIZE $THREAD>> full.log 2>&1
                perf stat -r $MEASUREMENTS ./$NAME -0.8 -0.7 0.05 0.15 $SIZE $THREAD>> seahorse.log 2>&1
                perf stat -r $MEASUREMENTS ./$NAME 0.175 0.375 -0.1 0.1 $SIZE $THREAD>> elephant.log 2>&1
                perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $THREAD>> triple_spiral.log 2>&1
                SIZE=$(($SIZE * 2))
        done

        SIZE=$INITIAL_SIZE

        mv *.log results/$NAME$THREAD
        rm output.ppm
    done
done