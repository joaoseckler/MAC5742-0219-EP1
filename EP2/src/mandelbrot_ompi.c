#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#define MASTER 0

double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char *flat;
int flat_size;
int flat_offset;

int i_x_max;
int i_y_max;
int image_slice;

int gradient_size = 16;
int colors[17][3] = {
    {66, 30, 15},
    {25, 7, 26},
    {9, 1, 47},
    {4, 4, 73},
    {0, 7, 100},
    {12, 44, 138},
    {24, 82, 177},
    {57, 125, 209},
    {134, 181, 229},
    {211, 236, 248},
    {241, 233, 191},
    {248, 201, 95},
    {255, 170, 0},
    {204, 128, 0},
    {153, 87, 0},
    {106, 52, 3},
    {16, 16, 16},
};

void init(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("usage: ./mandelbrot_pth c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./ompi_pth -2.5 1.5 -2.0 2.0 11500 32\n");
        printf("    Seahorse Valley:      ./ompi_pth -0.8 -0.7 0.05 0.15 11500 64\n");
        printf("    Elephant Valley:      ./ompi_pth 0.175 0.375 -0.1 0.1 11500 128\n");
        printf("    Triple Spiral Valley: ./ompi_pth -0.188 -0.012 0.554 0.754 11500 256\n");
        exit(0);
    }
    else
    {
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);

        i_x_max = image_size;
        i_y_max = image_size;

        pixel_width = (c_x_max - c_x_min) / i_x_max;
        pixel_height = (c_y_max - c_y_min) / i_y_max;
    }
}

void update_rgb_buffer(int iteration, int x, int y)
{

    int color;

    if (iteration == iteration_max)
    {
        flat[((i_y_max * y) + x) * 3 + 0 - flat_offset] = colors[gradient_size][0];
        flat[((i_y_max * y) + x) * 3 + 1 - flat_offset] = colors[gradient_size][1];
        flat[((i_y_max * y) + x) * 3 + 2 - flat_offset] = colors[gradient_size][2];
    }
    else
    {
        color = iteration % gradient_size;
        flat[((i_y_max * y) + x) * 3 + 0 - flat_offset] = colors[color][0];
        flat[((i_y_max * y) + x) * 3 + 1 - flat_offset] = colors[color][1];
        flat[((i_y_max * y) + x) * 3 + 2 - flat_offset] = colors[color][2];
    }
}

void write_to_file()
{
    FILE *file;
    char *filename = "output.ppm";
    char *comment = "# ";

    int max_color_component_value = 255;

    file = fopen(filename, "wb");

    fprintf(file, "P6\n %s\n %d\n %d\n %d\n", comment,
            i_x_max, i_y_max, max_color_component_value);

    fwrite(flat, sizeof(unsigned char), flat_size, file);

    fclose(file);
}

struct thread_data
{
    int start;
    int end;
};

void calculate(int start, int end)
{
    int i_y;
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    double c_x;
    double c_y;

    int iteration;
    for (i_y = start; i_y < end; i_y++)
    {
        c_y = c_y_min + i_y * pixel_height;

        if (fabs(c_y) < pixel_height / 2)
        {
            c_y = 0.0;
        }

        for (int i_x = 0; i_x < i_x_max; i_x++)
        {
            c_x = c_x_min + i_x * pixel_width;

            z_x = 0.0;
            z_y = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for (iteration = 0;
                 iteration < iteration_max &&
                 ((z_x_squared + z_y_squared) < escape_radius_squared);
                 iteration++)
            {
                z_y = 2 * z_x * z_y + c_y;
                z_x = z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            }

            update_rgb_buffer(iteration, i_x, i_y);
        }
    }
}

void compute_mandelbrot(int argc, char *argv[])
{
    int dest, i, source, leftover, nprocs, myid, offset, tmp;

    //init mpi
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    //start of mpi
    image_slice = image_size / nprocs;
    leftover = image_size % nprocs;

    if (myid == MASTER)
    {
        flat_size = image_size * image_size * 3;
        flat = malloc(sizeof(unsigned char) * flat_size);
        flat_offset = 0;
        offset = 0;

        for (dest = 1; dest < nprocs; dest++)
        {
            MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            printf("Sent %d elements to task %d offset= %d\n", image_slice, dest, offset);
            offset = offset + image_slice;
        }

        //work
        calculate(offset, offset + image_slice + leftover);

        offset = 0;
        for (i = 1; i < nprocs; i++)
        {
            source = i;
            MPI_Recv(&flat[offset*image_size*3], image_slice*image_size*3, MPI_UNSIGNED_CHAR, source, 4, MPI_COMM_WORLD, &status);
            offset += image_slice;
        }

        write_to_file();
    }

    else
    {
        /* Receive my portion of array from the master task */
        source = MASTER;
        MPI_Recv(&tmp, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);

        flat_size = image_slice * image_size * 3;
        flat = malloc(sizeof(unsigned char) * flat_size);
        flat_offset = tmp * image_size * 3;

        calculate(tmp, tmp + image_slice);

        /* Send my results back to the master task */
        dest = MASTER;
        MPI_Send(flat, flat_size, MPI_UNSIGNED_CHAR, dest, 4, MPI_COMM_WORLD);
    } /* end of non-master */

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}

int main(int argc, char *argv[])
{

    init(argc, argv);

    compute_mandelbrot(argc, argv);

    return 0;
}
