#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "omp.h"

#define DEBUG 1

void stampa_matrice(const double* mat, int dim) {
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            printf("\t%0.2f", mat[i*dim + j]);
        }     

        printf("\n");   
    }

    printf("\n");
}

double gemm(const int* mat_a, const int* mat_b, int* mat_c, int dim) {
    double start = omp_get_wtime();

    // i primi due cicli considerano ogni elemento della matrice
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            // il singolo elemento della matrice risultante viene calcolato considerando un intera riga/colonna
            for(int k=0; k<dim; k++) {
                mat_c[i*dim + j] += mat_a[i*dim + k] * mat_b[k*dim + j];
            }  
        }     
    }

    double end = omp_get_wtime();

    return end-start;
}

double parallel_gemm(const int* mat_a, const int* mat_b, int* mat_c, int dim) {
    double start = omp_get_wtime();

    #pragma omp parallel for schedule (static, 1)
    // i primi due cicli considerano ogni elemento della matrice
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            #ifdef DEBUG
            int thread_id = omp_get_thread_num();
            printf("\tThread %d sta calcolando mat_c[%d][%d]\n", thread_id, i, j);
            #endif
            // il singolo elemento della matrice risultante viene calcolato considerando un intera riga/colonna
            for(int k=0; k<dim; k++) {
                mat_c[i*dim + j] += mat_a[i*dim + k] * mat_b[k*dim + j];
            }  
        }     
    }

    double end = omp_get_wtime();

    return end-start;
}

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("usage: ./conv <dim_matrix> <num_thread>\n");
        exit(1);
    }

    int dim_matrix = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    // printf("Numero massimo di thread eseguibili in parallelo: %d\n", omp_get_max_threads());
    // omp_set_num_threads(num_threads);

    int size_input = dim_matrix*dim_matrix*sizeof(double);
    // approssimo per eccesso (es: input 5x5 -> risultato 3x3)
    int dim_risultato = (dim_matrix+1) / 2;
    int size_risultato = dim_risultato*dim_risultato*sizeof(double);
    
    double* mat_input = malloc(size_input);
    double* mat_risultato = malloc(size_risultato);

    srand(time(NULL));
    for(int i=0; i<dim_matrix; i++) {
        for(int j=0; j<dim_matrix; j++) {
            // rand() varia tra [0, RAND_MAX]
            // qua sotto sto quindi generando numeri casuali tra 0 e 1
            mat_input[i*dim_matrix + j] = (double)rand() / RAND_MAX;
        }        
    }

    #ifdef DEBUG
    printf("--- MATRICE INPUT ---\n");
    stampa_matrice(mat_input, dim_matrix);
    printf("--- MATRICE RISULTATO ---\n");
    stampa_matrice(mat_risultato, dim_risultato);
    #endif

    // double elapsed_sequential = gemm(mat_a, mat_b, mat_c, dim_matrix);
    // double elapsed_parallel = parallel_gemm(mat_a, mat_b, mat_c, dim_matrix);

    #ifdef DEBUG
    printf("--- MATRICE RISULTATO ---\n");
    stampa_matrice(mat_risultato, dim_risultato);
    #endif

    // printf("Elapsed sequential:\t %f ms\n", elapsed_sequential*1000);
    // printf("Elapsed parallel:\t %f ms;\tSpeedup: %0.2f\n", elapsed_parallel*1000, elapsed_sequential/elapsed_parallel);

    free(mat_input);
    free(mat_risultato);
}