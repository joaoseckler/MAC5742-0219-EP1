OUTPUT=mandelbrot

IMAGE=.ppm

CC=gcc
CC_OMPI=mpicc
CC_OPT=--pedantic -Wall

CC_OMP=-fopenmp
CC_PTH=-pthread

.PHONY: all
all: $(OUTPUT)_ompi

$(OUTPUT)_ompi: $(OUTPUT)_ompi.c
	$(CC_OMPI) -o $(OUTPUT)_ompi $(CC_OPT) $(OUTPUT)_ompi.c

.PHONY: clean
clean:
	rm -f $(OUTPUT)_ompi *$(IMAGE)
