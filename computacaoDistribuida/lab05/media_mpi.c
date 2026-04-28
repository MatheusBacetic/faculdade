#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int N;
    float *vetor;
    float soma_local = 0.0;
    float media_local = 0.0;
    float soma_global = 0.0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            printf("Uso correto: mpirun -np <processos> ./media_mpi <N>\n");
        }
        MPI_Finalize();
        return 1;
    }

    N = atoi(argv[1]);

    if (N <= 0) {
        if (rank == 0) {
            printf("Erro: N deve ser maior que zero.\n");
        }
        MPI_Finalize();
        return 1;
    }

    vetor = (float *) malloc(N * sizeof(float));

    if (vetor == NULL) {
        printf("[Processo %d] Erro ao alocar memória.\n", rank);
        MPI_Finalize();
        return 1;
    }

    srand(time(NULL) + rank);

    for (int i = 0; i < N; i++) {
        vetor[i] = rand() / (float) RAND_MAX;
        soma_local += vetor[i];
    }

    media_local = soma_local / N;

    printf("[Processo %d] Soma local: %.3f, Média local: %.4f\n",
           rank, soma_local, media_local);

    MPI_Reduce(
        &soma_local,
        &soma_global,
        1,
        MPI_FLOAT,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0) {
        float media_global = soma_global / (N * size);

        printf("\n[Soma global] %.3f\n", soma_global);
        printf("[Média global] %.4f\n", media_global);
    }

    free(vetor);

    MPI_Finalize();

    return 0;
}