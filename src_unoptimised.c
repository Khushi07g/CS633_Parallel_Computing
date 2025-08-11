#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int px, py, pz;
int nx, ny, nz, nc;
int myrank, nprocs;
double ****local_data;

void Send(int x1, int x2, int y1, int y2, int z1, int z2, int to) {
    int size = (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1) * nc;
    double *send_buf = (double *)malloc(sizeof(double) * size);

    int ptr = 0;
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            for (int z = z1; z <= z2; z++) {
                for (int t = 0; t < nc; t++) {
                    send_buf[ptr++] = local_data[x][y][z][t];
                }
            }
        }
    }

    MPI_Send(send_buf, size, MPI_DOUBLE, to, to, MPI_COMM_WORLD);
    free(send_buf);
}

void Recv(int x1, int x2, int y1, int y2, int z1, int z2, int from) {
    int size = (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1) * nc;
    double *recv_buf = (double *)malloc(sizeof(double) * size);

    MPI_Status status;
    MPI_Recv(recv_buf, size, MPI_DOUBLE, from, myrank, MPI_COMM_WORLD, &status);

    int ptr = 0;
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            for (int z = z1; z <= z2; z++) {
                for (int t = 0; t < nc; t++) {
                    local_data[x][y][z][t] = recv_buf[ptr++];
                }
            }
        }
    }

    free(recv_buf);
}

int main(int argc, char *argv[]) {
    char input_file[128], output_file[128];

    MPI_File fh;
    MPI_Status status;

    double *global_data;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    double tot_start = MPI_Wtime();
    double io_start = MPI_Wtime();

    // INPUT ARGUMENTS
    strcpy(input_file, argv[1]);
    px = atoi(argv[2]), py = atoi(argv[3]), pz = atoi(argv[4]);
    nx = atoi(argv[5]), ny = atoi(argv[6]), nz = atoi(argv[7]);
    nc = atoi(argv[8]);
    strcpy(output_file, argv[9]);

    assert(nprocs == (px * py * pz));

    // size of subdomain
    int mx = (nx + px - 1) / px;
    int my = (ny + py - 1) / py;
    int mz = (nz + pz - 1) / pz;

    // coordinate and size of subdomain
    int x_coord = myrank % px, y_coord = (myrank / px) % py,
        z_coord = myrank / (px * py);
    int size_x = ((x_coord == px - 1) ? (nx - (px - 1) * mx) : mx);
    int size_y = ((y_coord == py - 1) ? (ny - (py - 1) * my) : my);
    int size_z = ((z_coord == pz - 1) ? (nz - (pz - 1) * mz) : mz);

    // total size of each subdomain
    int sizes[nprocs];
    for (int rank = 0; rank < nprocs; rank++) {
        int x_coord = rank % px, y_coord = (rank / px) % py,
            z_coord = rank / (px * py);
        int size_x = ((x_coord == px - 1) ? (nx - (px - 1) * mx) : mx);
        int size_y = ((y_coord == py - 1) ? (ny - (py - 1) * my) : my);
        int size_z = ((z_coord == pz - 1) ? (nz - (pz - 1) * mz) : mz);
        sizes[rank] = size_x * size_y * size_z * nc;
    }

    if (!myrank) {
        // Reading from file
        global_data = (double *)malloc(sizeof(double) * nx * ny * nz * nc);
        int offsets[nprocs];
        int psum = 0;
        for (int i = 0; i < nprocs; i++) {
            offsets[i] = psum;
            psum += sizes[i];
        }

        FILE *file = fopen(input_file, "r");
        if (!file) {
            perror("Failed to open file");
            return 1;
        }

        char line[nc * 20];
        int x = 0, y = 0, z = 0;
        while (fgets(line, sizeof(line), file) && z < nz) {
            char *token = strtok(line, " ");
            int proc = (x / mx) + (y / my) * px + (z / mz) * px * py;

            int t = 0;
            while (token && t < nc) {
                global_data[offsets[proc]++] = atof(token);  // Convert to float
                token = strtok(NULL, " ");
                t++;
            }
            x++;
            if (x == nx)
                y++, x = 0;
            if (y == ny)
                z++, y = 0;
        }

        fclose(file);
    }

    // SCATTER

    int displs[nprocs];
    int psum = 0;
    for (int i = 0; i < nprocs; i++) {
        displs[i] = psum;
        psum += sizes[i];
    }

    double *recv_buf = (double *)malloc(size_x * size_y * size_z * nc * sizeof(double));
    MPI_Scatterv(global_data, sizes, displs, MPI_DOUBLE, recv_buf, sizes[myrank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    local_data = (double ****)malloc((size_x + 2) * sizeof(double ***));
    for (int x = 0; x < size_x + 2; x++) {
        local_data[x] = (double ***)malloc((size_y + 2) * sizeof(double **));
        for (int y = 0; y < size_y + 2; y++) {
            local_data[x][y] = (double **)malloc((size_z + 2) * sizeof(double *));
            for (int z = 0; z < size_z + 2; z++) {
                local_data[x][y][z] = (double *)malloc(nc * sizeof(double));
            }
        }
    }

    for (int x = 0; x < size_x; x++) {
        for (int y = 0; y < size_y; y++) {
            for (int z = 0; z < size_z; z++) {
                for (int t = 0; t < nc; t++) {
                    local_data[x + 1][y + 1][z + 1][t] = recv_buf[t + nc * x + nc * size_x * y + nc * size_x * size_y * z];
                }
            }
        }
    }

    if (!myrank)
        free(global_data);
    free(recv_buf);

    // EXCHANGE

    // x-direction
    if (x_coord % 2 == 0) {
        if (x_coord < px - 1) {
            Send(size_x, size_x, 1, size_y, 1, size_z, myrank + 1);
            Recv(size_x + 1, size_x + 1, 1, size_y, 1, size_z, myrank + 1);
        }
    } else {
        if (x_coord > 0) {
            Recv(0, 0, 1, size_y, 1, size_z, myrank - 1);
            Send(1, 1, 1, size_y, 1, size_z, myrank - 1);
        }
    }

    if (x_coord % 2 == 1) {
        if (x_coord < px - 1) {
            Send(size_x, size_x, 1, size_y, 1, size_z, myrank + 1);
            Recv(size_x + 1, size_x + 1, 1, size_y, 1, size_z, myrank + 1);
        }
    } else {
        if (x_coord > 0) {
            Recv(0, 0, 1, size_y, 1, size_z, myrank - 1);
            Send(1, 1, 1, size_y, 1, size_z, myrank - 1);
        }
    }

    // y-direction
    if (y_coord % 2 == 0) {
        if (y_coord < py - 1) {
            Send(1, size_x, size_y, size_y, 1, size_z, myrank + px);
            Recv(1, size_x, size_y + 1, size_y + 1, 1, size_z, myrank + px);
        }
    } else {
        if (y_coord > 0) {
            Recv(1, size_x, 0, 0, 1, size_z, myrank - px);
            Send(1, size_x, 1, 1, 1, size_z, myrank - px);
        }
    }

    if (y_coord % 2 == 1) {
        if (y_coord < py - 1) {
            Send(1, size_x, size_y, size_y, 1, size_z, myrank + px);
            Recv(1, size_x, size_y + 1, size_y + 1, 1, size_z, myrank + px);
        }
    } else {
        if (y_coord > 0) {
            Recv(1, size_x, 0, 0, 1, size_z, myrank - px);
            Send(1, size_x, 1, 1, 1, size_z, myrank - px);
        }
    }

    // z-direction
    if (z_coord % 2 == 0) {
        if (z_coord < pz - 1) {
            Send(1, size_x, 1, size_y, size_z, size_z, myrank + px * py);
            Recv(1, size_x, 1, size_y, size_z + 1, size_z + 1, myrank + px * py);
        }
    } else {
        if (z_coord > 0) {
            Recv(1, size_x, 1, size_y, 0, 0, myrank - px * py);
            Send(1, size_x, 1, size_y, 1, 1, myrank - px * py);
        }
    }

    if (z_coord % 2 == 1) {
        if (z_coord < pz - 1) {
            Send(1, size_x, 1, size_y, size_z, size_z, myrank + px * py);
            Recv(1, size_x, 1, size_y, size_z + 1, size_z + 1, myrank + px * py);
        }
    } else {
        if (z_coord > 0) {
            Recv(1, size_x, 1, size_y, 0, 0, myrank - px * py);
            Send(1, size_x, 1, size_y, 1, 1, myrank - px * py);
        }
    }

    double io_end = MPI_Wtime();
    double io_time = io_end - io_start;
    double comp_start = MPI_Wtime();

    // local min and global min
    int local_min_count_p[nc], local_max_count_p[nc];
    double global_min_p[nc], global_max_p[nc];
    for (int t = 0; t < nc; t++) {
        local_min_count_p[t] = 0;
        local_max_count_p[t] = 0;
        global_min_p[t] = -1;
        global_max_p[t] = -1;
    }

    for (int x = 1; x <= size_x; x++) {
        for (int y = 1; y <= size_y; y++) {
            for (int z = 1; z <= size_z; z++) {
                for (int t = 0; t < nc; t++) {
                    // local min
                    int cnt = 0;
                    if ((x_coord == 0 && x == 1) || local_data[x][y][z][t] <= local_data[x - 1][y][z][t])
                        cnt++;
                    if ((x_coord == px - 1 && x == size_x) || local_data[x][y][z][t] <= local_data[x + 1][y][z][t])
                        cnt++;
                    if ((y_coord == 0 && y == 1) || local_data[x][y][z][t] <= local_data[x][y - 1][z][t])
                        cnt++;
                    if ((y_coord == py - 1 && y == size_y) || local_data[x][y][z][t] <= local_data[x][y + 1][z][t])
                        cnt++;
                    if ((z_coord == 0 && z == 1) || local_data[x][y][z][t] <= local_data[x][y][z - 1][t])
                        cnt++;
                    if ((z_coord == pz - 1 && z == size_z) || local_data[x][y][z][t] <= local_data[x][y][z + 1][t])
                        cnt++;

                    if (cnt == 6)
                        local_min_count_p[t]++;

                    // local max
                    cnt = 0;
                    if ((x_coord == 0 && x == 1) || local_data[x][y][z][t] >= local_data[x - 1][y][z][t])
                        cnt++;
                    if ((x_coord == px - 1 && x == size_x) || local_data[x][y][z][t] >= local_data[x + 1][y][z][t])
                        cnt++;
                    if ((y_coord == 0 && y == 1) || local_data[x][y][z][t] >= local_data[x][y - 1][z][t])
                        cnt++;
                    if ((y_coord == py - 1 && y == size_y) || local_data[x][y][z][t] >= local_data[x][y + 1][z][t])
                        cnt++;
                    if ((z_coord == 0 && z == 1) || local_data[x][y][z][t] >= local_data[x][y][z - 1][t])
                        cnt++;
                    if ((z_coord == pz - 1 && z == size_z) || local_data[x][y][z][t] >= local_data[x][y][z + 1][t])
                        cnt++;

                    if (cnt == 6) {
                        local_max_count_p[t]++;
                    }

                    // global min
                    if (global_min_p[t] == -1 || global_min_p[t] > local_data[x][y][z][t])
                        global_min_p[t] = local_data[x][y][z][t];

                    // global max
                    if (global_max_p[t] == -1 || global_max_p[t] < local_data[x][y][z][t])
                        global_max_p[t] = local_data[x][y][z][t];
                }
            }
        }
    }

    int local_min_count[nc], local_max_count[nc];
    double global_min[nc], global_max[nc];

    MPI_Reduce(local_min_count_p, local_min_count, nc, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(local_max_count_p, local_max_count, nc, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(global_min_p, global_min, nc, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(global_max_p, global_max, nc, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double comp_end = MPI_Wtime();
    double comp_time = comp_end - comp_start;

    FILE *out;
    if (!myrank) {
        out = fopen(output_file, "w");
        for (int t = 0; t < nc; t++) {
            fprintf(out, "(%d, %d)", local_min_count[t], local_max_count[t]);
            if (t != nc - 1)
                fprintf(out, ", ");
        }
        fprintf(out, "\n");
        for (int t = 0; t < nc; t++) {
            fprintf(out, "(%lf, %lf)", global_min[t], global_max[t]);
            if (t != nc - 1)
                fprintf(out, ", ");
        }
        fprintf(out, "\n");
    }

    double tot_end = MPI_Wtime();
    double tot_time = tot_end - tot_start;

    double max_io_time, max_comp_time, max_tot_time;
    MPI_Reduce(&io_time, &max_io_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comp_time, &max_comp_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&tot_time, &max_tot_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (!myrank) {
        fprintf(out, "%lf, ", max_io_time);
        fprintf(out, "%lf, ", max_comp_time);
        fprintf(out, "%lf\n", max_tot_time);
    }

    MPI_Finalize();
    return 0;
}
