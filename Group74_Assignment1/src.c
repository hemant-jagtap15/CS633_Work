/*
 * CS633 Assignment 1
 * MPI Point-to-Point Communication using MPI_Send and MPI_Recv
 *
 * Usage:
 *   mpirun -np P ./src M D1 D2 T seed
 *
 * Each process exchanges buffers with ranks at distances D1 and D2,
 * performs computations, and reports global maximum values with runtime.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define TAG_D1_SEND 10
#define TAG_D1_RET  11
#define TAG_D2_SEND 20
#define TAG_D2_RET  21

#define TAG_MAX_D1 100
#define TAG_MAX_D2 101

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    /* Get rank and total number of processes */
    int rank, P;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    /* Input parameter check */
    if (argc != 6)
    {
        if (rank == 0)
            printf("Usage: ./src M D1 D2 T seed\n");

        MPI_Finalize();
        return 0;
    }

    /* Read input arguments */
    int M    = atoi(argv[1]);
    int D1   = atoi(argv[2]);
    int D2   = atoi(argv[3]);
    int T    = atoi(argv[4]);
    int seed = atoi(argv[5]);

    /* Allocate communication buffers */
    double *buffer_D1 = malloc(M * sizeof(double));
    double *buffer_D2 = malloc(M * sizeof(double));

    double *recvbuf = malloc(M * sizeof(double));
    double *retbuf  = malloc(M * sizeof(double));

    /* Check allocation success */
    if (!buffer_D1 || !buffer_D2 || !recvbuf || !retbuf)
    {
        printf("Rank %d: Memory allocation failed!\n", rank);
        MPI_Finalize();
        return 0;
    }

    /* Initialize local buffers with random values */
    srand(seed);

    for (int i = 0; i < M; i++)
    {
        double val = (double)rand() * (rank + 1) / 10000.0;
        buffer_D1[i] = val;
        buffer_D2[i] = val;
    }

    /* Check if this rank is a valid sender for D1 and D2 */
    int send_to_D1 = (rank + D1 <= P - 1);
    int send_to_D2 = (rank + D2 <= P - 1);

    /* Start execution timer */
    double start = MPI_Wtime();

    /* Store maximum values from received buffers in final iteration */
    double last_recv_max_D1 = -DBL_MAX;
    double last_recv_max_D2 = -DBL_MAX;

    /* Main iteration loop */
    for (int iter = 0; iter < T; iter++)
    {
        MPI_Status status;

        int src_D1 = rank - D1;
        int src_D2 = rank - D2;

        /* ---------------- D1 Receiver ---------------- */
        if (src_D1 >= 0)
        {
            /* Receive buffer, compute square, send back */
            MPI_Recv(recvbuf, M, MPI_DOUBLE,
                     src_D1, TAG_D1_SEND,
                     MPI_COMM_WORLD, &status);

            for (int i = 0; i < M; i++)
                recvbuf[i] = recvbuf[i] * recvbuf[i];

            MPI_Send(recvbuf, M, MPI_DOUBLE,
                     src_D1, TAG_D1_RET,
                     MPI_COMM_WORLD);
        }

        /* ---------------- D1 Sender ---------------- */
        if (send_to_D1)
        {
            /* Send buffer and receive squared result */
            MPI_Send(buffer_D1, M, MPI_DOUBLE,
                     rank + D1, TAG_D1_SEND,
                     MPI_COMM_WORLD);

            MPI_Recv(retbuf, M, MPI_DOUBLE,
                     rank + D1, TAG_D1_RET,
                     MPI_COMM_WORLD, &status);

            /* Compute max directly from received buffer in final iteration */
            if (iter == T - 1)
            {
                for (int i = 0; i < M; i++)
                    if (retbuf[i] > last_recv_max_D1)
                        last_recv_max_D1 = retbuf[i];
            }

            /* Update local buffer using modulo operation */
            for (int i = 0; i < M; i++)
            {
                unsigned long long v =
                    (unsigned long long)retbuf[i];

                buffer_D1[i] = (double)(v % 100000ULL);
            }
        }

        /* ---------------- D2 Receiver ---------------- */
        if (src_D2 >= 0)
        {
            /* Receive buffer, compute log, send back */
            MPI_Recv(recvbuf, M, MPI_DOUBLE,
                     src_D2, TAG_D2_SEND,
                     MPI_COMM_WORLD, &status);

            for (int i = 0; i < M; i++)
                recvbuf[i] = log(recvbuf[i]);

            MPI_Send(recvbuf, M, MPI_DOUBLE,
                     src_D2, TAG_D2_RET,
                     MPI_COMM_WORLD);
        }

        /* ---------------- D2 Sender ---------------- */
        if (send_to_D2)
        {
            /* Send buffer and receive log result */
            MPI_Send(buffer_D2, M, MPI_DOUBLE,
                     rank + D2, TAG_D2_SEND,
                     MPI_COMM_WORLD);

            MPI_Recv(retbuf, M, MPI_DOUBLE,
                     rank + D2, TAG_D2_RET,
                     MPI_COMM_WORLD, &status);

            /* Compute max directly from received buffer in final iteration */
            if (iter == T - 1)
            {
                for (int i = 0; i < M; i++)
                    if (retbuf[i] > last_recv_max_D2)
                        last_recv_max_D2 = retbuf[i];
            }

            /* Update local buffer by scaling */
            for (int i = 0; i < M; i++)
                buffer_D2[i] = retbuf[i] * 100000.0;
        }
    }

    /* Final local maxima from received buffers */
    double local_max_D1 = -DBL_MAX;
    double local_max_D2 = -DBL_MAX;

    if (send_to_D1)
        local_max_D1 = last_recv_max_D1;

    if (send_to_D2)
        local_max_D2 = last_recv_max_D2;

    /* Rank 0 collects global maximum values */
    double global_max_D1 = -DBL_MAX;
    double global_max_D2 = -DBL_MAX;

    if (rank == 0)
    {
        if (send_to_D1) global_max_D1 = local_max_D1;
        if (send_to_D2) global_max_D2 = local_max_D2;

        for (int r = 1; r < P; r++)
        {
            int validD1 = (r + D1 <= P - 1);
            int validD2 = (r + D2 <= P - 1);

            double temp;

            if (validD1)
            {
                MPI_Recv(&temp, 1, MPI_DOUBLE,
                         r, TAG_MAX_D1,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (temp > global_max_D1)
                    global_max_D1 = temp;
            }

            if (validD2)
            {
                MPI_Recv(&temp, 1, MPI_DOUBLE,
                         r, TAG_MAX_D2,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (temp > global_max_D2)
                    global_max_D2 = temp;
            }
        }
    }
    else
    {
        /* Send local maxima to rank 0 */
        if (send_to_D1)
            MPI_Send(&local_max_D1, 1, MPI_DOUBLE,
                     0, TAG_MAX_D1, MPI_COMM_WORLD);

        if (send_to_D2)
            MPI_Send(&local_max_D2, 1, MPI_DOUBLE,
                     0, TAG_MAX_D2, MPI_COMM_WORLD);
    }

    /* End execution timer */
    double end = MPI_Wtime();

    /* Rank 0 prints final output */
    if (rank == 0)
        printf("%lf %lf %lf\n",
               global_max_D1,
               global_max_D2,
               end - start);

    /* Free allocated memory */
    free(buffer_D1);
    free(buffer_D2);
    free(recvbuf);
    free(retbuf);

    MPI_Finalize();
    return 0;
}
