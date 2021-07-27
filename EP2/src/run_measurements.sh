#! /bin/bash

set -o xtrace

MEASUREMENTS=15
SIZE=4096
MASTER_TASK=1
THREADS=8

make
mkdir -p results

########### SEQ ###########
NAME='mandelbrot_seq'
perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE >> triple_spiral.log 2>&1
mv *.log results/$NAME
rm output.ppm
########### SEQ ###########


########### PTH & OMP ###########
NAMES=('mandelbrot_pth' 'mandelbrot_omp')

for NAME in ${NAMES[@]}; do
    mkdir results/$NAME

    perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $THREADS >> triple_spiral.log 2>&1

    mv *.log results/$NAME
    rm output.ppm
done
########### PTH & OMP ###########


########### OMPI ###########
NAME='mandelbrot_ompi'

mkdir -p results/$NAME

    for TASKS in 1 8 16 32; do
        let "TOTAL_TASKS = $TASKS + $MASTER_TASK"
        perf stat -r $MEASUREMENTS mpirun --allow-run-as-root --oversubscribe -n $TOTAL_TASKS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE >> triple_spiral.log 2>&1
    done

mv *.log results/$NAME
rm -rf $NAME.dSYM
rm output.ppm
########### OMPI ###########
