
OUTPUT=mandelbrot

IMAGE=.ppm

CC=gcc
CC_OMPI=mpicc
CC_OPT=--pedantic -Wall

CC_OMP=-fopenmp
CC_PTH=-pthread

.PHONY: all
all: $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_seq $(OUTPUT)_ompi $(OUTPUT)_ompi_pth $(OUTPUT)_ompi_omp

$(OUTPUT)_omp: $(OUTPUT)_omp.c
	$(CC) -o $(OUTPUT)_omp $(CC_OPT) $(CC_OMP) $(OUTPUT)_omp.c

$(OUTPUT)_pth: $(OUTPUT)_pth.c
	$(CC) -o $(OUTPUT)_pth $(CC_OPT) $(CC_PTH) $(OUTPUT)_pth.c

$(OUTPUT)_seq: $(OUTPUT)_seq.c
	$(CC) -o $(OUTPUT)_seq $(CC_OPT) $(OUTPUT)_seq.c

$(OUTPUT)_ompi: $(OUTPUT)_ompi.c
	$(CC_OMPI) -o $(OUTPUT)_ompi $(CC_OPT) $(OUTPUT)_ompi.c

$(OUTPUT)_ompi_pth: $(OUTPUT)_ompi_pth.c
	$(CC_OMPI) -o $(OUTPUT)_ompi_pth $(CC_OPT) $(CC_PTH) $(OUTPUT)_ompi_pth.c

$(OUTPUT)_ompi_omp: $(OUTPUT)_ompi_omp.c
	$(CC_OMPI) -o $(OUTPUT)_ompi_omp $(CC_OPT) $(CC_OMP) $(OUTPUT)_ompi_omp.c

.PHONY: clean
clean:
	rm -f $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_seq $(OUTPUT)_ompi $(OUTPUT)_ompi_pth $(OUTPUT)_ompi_omp *$(IMAGE)
