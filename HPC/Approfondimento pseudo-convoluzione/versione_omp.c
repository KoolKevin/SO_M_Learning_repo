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

double calcola_media_intorno(const double* mat_input, int dim_input, int riga, int colonna) {
    double somma = 0.0;

    /*
        considero l'intorno 3x3 dell'elemento corrente

        NB: non ho corse critiche dato che gli unici elementi della matrice che vengono
        acceduti da più thread contemporaneamente sono quelli appartenenti agli intorni; 
        quest'ultimi vengono acceduti in sola lettura! 
    */
    for(int i=-1; i<=1; i++) {
        for(int j=-1; j<=1; j++) {
            int riga_intorno = riga + i;
            int colonna_intorno = colonna + j;

            // controllo se sto considerando l'intorno di un elemento su un bordo della matrice
            if( (riga_intorno<0 || riga_intorno>=dim_input) || (colonna_intorno<0 || colonna_intorno>=dim_input) ) {
                somma += 0.0;
            }
            else {
                somma += mat_input[riga_intorno*dim_input + colonna_intorno];
            }
        }
    }

    return somma/9;
}


/*
    NB: problema embarassingly parallel!
    - Le medie degli intorni sono calcolabili in maniera indipendente l'una dall'altra
    - Non c'è bisogno di interazione tra i processi concorrenti  
*/
double pseudo_convoluzione_parallela(const double* mat_input, double* mat_output, int dim_input, int dim_output) {
    double start = omp_get_wtime();

    #pragma omp parallel for schedule (static, 1)
    for(int i=0; i<dim_input; i+=2) {
        for(int j=0; j<dim_input; j+=2) {
            double media_intorno = calcola_media_intorno(mat_input, dim_input, i, j);   // variabile locale al singolo thread (definita internamente)
            mat_output[(i/2)*dim_output + (j/2)] = media_intorno;
        }     
    }   //sincronizzazione implicita alla fine del blocco parallelo (modello cobegin-coend)

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
    
    printf("Numero di core (logici) disponibili: %d\n", omp_get_max_threads());
    omp_set_num_threads(num_threads);

    int size_input = dim_matrix*dim_matrix*sizeof(double);
    // approssimo per eccesso (es: input 5x5 -> risultato 3x3)
    int dim_risultato = (dim_matrix+1) / 2;
    int size_risultato = dim_risultato*dim_risultato*sizeof(double);
    
    double* mat_input = malloc(size_input);
    double* mat_risultato = malloc(size_risultato);
    for (int i = 0; i < size_risultato/sizeof(double); i++) {
        mat_risultato[i] = 0.0;
    }

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
    #endif

    double elapsed_parallel = pseudo_convoluzione_parallela(mat_input, mat_risultato, dim_matrix, dim_risultato);
    #ifdef DEBUG
    printf("--- MATRICE RISULTATO PARALLELO ---\n");
    stampa_matrice(mat_risultato, dim_risultato);
    #endif

    printf("Elapsed parallel:\t %f ms;\n", elapsed_parallel*1000);

    free(mat_input);
    free(mat_risultato);
}