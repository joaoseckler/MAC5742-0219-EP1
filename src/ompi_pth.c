#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
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
unsigned char **image_buffer;
unsigned char *flat;

int i_x_max;
int i_y_max;
int image_buffer_size;
int image_slice;

int num_threads;
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

void allocate_image_buffer()
{
    int rgb_size = 3;
    image_buffer = (unsigned char **)malloc(sizeof(unsigned char *) * image_buffer_size);

    for (int i = 0; i < image_buffer_size; i++)
    {
        image_buffer[i] = (unsigned char *)malloc(sizeof(unsigned char) * rgb_size);
    }
}

void init(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("usage: ./mandelbrot_pth c_x_min c_x_max c_y_min c_y_max image_size num_threads\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_pth -2.5 1.5 -2.0 2.0 11500 32\n");
        printf("    Seahorse Valley:      ./mandelbrot_pth -0.8 -0.7 0.05 0.15 11500 64\n");
        printf("    Elephant Valley:      ./mandelbrot_pth 0.175 0.375 -0.1 0.1 11500 128\n");
        printf("    Triple Spiral Valley: ./mandelbrot_pth -0.188 -0.012 0.554 0.754 11500 256\n");
        exit(0);
    }
    else
    {
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);
        if (argc >= 7)
            sscanf(argv[6], "%d", &num_threads);
        else
            num_threads = 32;

        i_x_max = image_size;
        i_y_max = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width = (c_x_max - c_x_min) / i_x_max;
        pixel_height = (c_y_max - c_y_min) / i_y_max;
    }
}

void update_rgb_buffer(int iteration, int x, int y)
{

    int color;

    if (iteration == iteration_max)
    {
        flat[((i_y_max * y) + x) * 3 + 0] = colors[gradient_size][0];
        flat[((i_y_max * y) + x) * 3 + 1] = colors[gradient_size][1];
        flat[((i_y_max * y) + x) * 3 + 2] = colors[gradient_size][2];
    }
    else
    {
        color = iteration % gradient_size;
        flat[((i_y_max * y) + x) * 3 + 0] = colors[color][0];
        flat[((i_y_max * y) + x) * 3 + 1] = colors[color][1];
        flat[((i_y_max * y) + x) * 3 + 2] = colors[color][2];
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

    for (int i = 0; i < image_buffer_size; i++)
    {
        fwrite(image_buffer[i], 1, 3, file);
    }

    fclose(file);
}

struct thread_data
{
    int start;
    int end;
};

void *thread_routine(void *arg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *)arg;
    int i_y, start, end;
    start = my_data->start;
    end = my_data->end;
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
    pthread_exit(0);
}

void update()
{
    int j = 0;
    for (int i = 0; i < image_buffer_size * 3; i += 3)
    {
        image_buffer[j][0] = flat[i];
        image_buffer[j][1] = flat[i + 1];
        image_buffer[j][2] = flat[i + 2];
        j++;
    }
}

void compute_mandelbrot(int argc, char *argv[])
{

    int rc, dest, i, j = 0, tag1,
                     tag2, source, leftover, nprocs, myid, offset, tmp;

    //init mpi
    flat = (unsigned char *)malloc(sizeof(unsigned char) * image_buffer_size * 3);
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    //start of mpi
    int flat_slice = image_buffer_size * 3 / nprocs;
    image_slice = image_size / nprocs;
    leftover = image_size % nprocs;
    tag1 = 2;
    tag2 = 1;
    pthread_t threads[num_threads];
    struct thread_data thread_data_array[num_threads];

    if (myid == MASTER)
    {

        offset = image_slice + leftover;
        for (dest = 1; dest < nprocs; dest++)
        {
            MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
            printf("Sent %d elements to task %d offset= %d\n", image_slice, dest, offset);
            offset = offset + image_slice;
        }

        //work
        offset = 0;
        for (i = 0; i < image_slice % num_threads; i++)
        {
            thread_data_array[i].start = offset;
            offset += image_slice / num_threads + 1;
            thread_data_array[i].end = offset;
            pthread_create(&threads[i], NULL, thread_routine, (void *)&thread_data_array[i]);
        }

        for (; i < num_threads; i++)
        {
            thread_data_array[i].start = offset;
            offset += image_slice / num_threads;
            thread_data_array[i].end = offset;
            pthread_create(&threads[i], NULL, thread_routine, (void *)&thread_data_array[i]);
        }

        for (i = 0; i < num_threads; i++)
        {
            pthread_join(threads[i], NULL);
        }

        for (i = 1; i < nprocs; i++)
        {
            source = i;
            MPI_Recv(&flat[flat_slice * i], flat_slice, MPI_UNSIGNED_CHAR, source, 4, MPI_COMM_WORLD, &status);
        }

        update();
        write_to_file();
    }

    else
    {
        /* Receive my portion of array from the master task */
        source = MASTER;
        MPI_Recv(&tmp, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);

        /* Do my part of the work */
        //work
        offset = tmp;
        for (i = 0; i < image_slice % num_threads; i++)
        {
            thread_data_array[i].start = offset;
            offset += image_slice / num_threads + 1;
            thread_data_array[i].end = offset;
            pthread_create(&threads[i], NULL, thread_routine, (void *)&thread_data_array[i]);
        }

        for (; i < num_threads; i++)
        {
            thread_data_array[i].start = offset;
            offset += image_slice / num_threads;
            thread_data_array[i].end = offset;
            pthread_create(&threads[i], NULL, thread_routine, (void *)&thread_data_array[i]);
        }

        for (i = 0; i < num_threads; i++)
        {
            pthread_join(threads[i], NULL);
        }

        /* Send my results back to the master task */
        dest = MASTER;
        MPI_Send(&flat[flat_slice * myid], flat_slice, MPI_UNSIGNED_CHAR, dest, 4, MPI_COMM_WORLD);
    } /* end of non-master */

    MPI_Finalize();
    //MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char *argv[])
{

    init(argc, argv);

    allocate_image_buffer();

    compute_mandelbrot(argc, argv);

    return 0;
}