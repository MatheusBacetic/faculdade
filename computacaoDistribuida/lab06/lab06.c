#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define DATA_SIZE 100
#define NUM_PROCESSOS 5

void imprimir_vetor(int vetor[], int tamanho) {
    printf("[");
    for (int i = 0; i < tamanho; i++) {
        printf("%d", vetor[i]);

        if (i < tamanho - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main(int argc, char *argv[]) {
    int rank, size;
    int elementos_por_processo;

    int vetor_original[DATA_SIZE];
    int vetor_transformado[DATA_SIZE];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != NUM_PROCESSOS) {
        if (rank == 0) {
            printf("Erro: este programa deve ser executado com exatamente 5 processos.\n");
            printf("Use: mpirun -np 5 ./lab06\n");
        }

        MPI_Finalize();
        return 1;
    }

    elementos_por_processo = DATA_SIZE / NUM_PROCESSOS;

    int vetor_local[elementos_por_processo];

    if (rank == 0) {
        for (int i = 0; i < DATA_SIZE; i++) {
            vetor_original[i] = i + 1;
        }

        printf("[Processo 0] Vetor original:\n");
        imprimir_vetor(vetor_original, DATA_SIZE);
    }

    MPI_Scatter(
        vetor_original,
        elementos_por_processo,
        MPI_INT,
        vetor_local,
        elementos_por_processo,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    for (int i = 0; i < elementos_por_processo; i++) {
        vetor_local[i] = vetor_local[i] * vetor_local[i];
    }

    MPI_Gather(
        vetor_local,
        elementos_por_processo,
        MPI_INT,
        vetor_transformado,
        elementos_por_processo,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0) {
        printf("\n[Processo 0] Vetor transformado:\n");
        imprimir_vetor(vetor_transformado, DATA_SIZE);
    }

    MPI_Finalize();

    return 0;
}