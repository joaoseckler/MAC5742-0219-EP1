#! /bin/bash

trap 'echo Tchau!; rm -rf *.log *.dSYM output.ppm; exit 0' SIGINT SIGTERM

set -o xtrace

MEASUREMENTS=15
SIZE=4096
MASTER_TASK=1

make
mkdir -p results

########### SEQ ###########
mkdir -p results/$NAME
NAME='mandelbrot_seq'
perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE 2>&1 |
    tee -a triple_spiral.log
mv *.log results/$NAME/
rm output.ppm
########### SEQ ###########


########### PTH & OMP ###########
NAMES=('mandelbrot_pth' 'mandelbrot_omp')

for NAME in ${NAMES[@]}; do
    mkdir -p results/$NAME
    for THREADS in 2 4 8 16 32; do

        perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $THREADS 2>&1 |
            tee -a triple_spiral.log

    done
    mv *.log results/$NAME
    rm output.ppm
done
########### PTH & OMP ###########

########### OMPI ###########
NAMES=('mandelbrot_ompi')

for NAME in "${NAMES[@]}"; do
  mkdir -p results/$NAME
      for TASKS in 1 8 16 32 64; do
          perf stat -r $MEASUREMENTS mpirun --bind-to none \
          --allow-run-as-root --oversubscribe -n $TASKS \
          ./$NAME -0.188 -0.012 0.554 0.754 $SIZE 2>&1 |
              grep -v 'Ignoring PCI device' |
              grep -v 'Pass --enable-32bits' |
              grep -v 'warning: it would break the' |
              tee -a triple_spiral.log
      done

  mv *.log results/$NAME
  rm -rf $NAME.dSYM
  rm output.ppm
done
########### OMPI ###########


########### OMPI+PTH & OMPI+OMP ###########
NAMES=('mandelbrot_ompi_omp' 'mandelbrot_ompi_pth')
for NAME in "${NAMES[@]}"; do
  mkdir -p results/$NAME
      for TASKS in 1 8 16 32 64; do
          for THREADS in 2 4 8 16 32; do
              perf stat -r $MEASUREMENTS mpirun --bind-to none \
              --allow-run-as-root --oversubscribe -n $TASKS \
              ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $THREADS 2>&1 |
                  grep -v 'Ignoring PCI device' |
                  grep -v 'Pass --enable-32bits' |
                  grep -v 'warning: it would break the' |
                  tee -a triple_spiral.log
          done
      done

  mv *.log results/$NAME
  rm -rf $NAME.dSYM
  rm output.ppm
done
########### OMPI+PTH & OMPI+OMP ###########
