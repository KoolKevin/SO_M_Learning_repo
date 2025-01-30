#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "omp.h"


// #define DEBUG 1

#define RESET   "\x1b[0m"    // Resetta i colori
#define RED     "\x1b[31m"   // Rosso

void stampa_matrice_evidenziata(const double* mat, int dim) {
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            if (i%2 == 0 && j%2 == 0)
                printf("\t%s%0.2f%s", RED, mat[i*dim + j], RESET);
            else
                printf("\t%0.2f", mat[i*dim + j]);
        }     

        printf("\n");   
    }

    printf("\n");
}

void stampa_matrice(const double* mat, int dim) {
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            printf("\t%0.2f", mat[i*dim + j]);
        }     

        printf("\n");   
    }

    printf("\n");
}

void check_matrici_uguali(const double* mat_a, const double* mat_b, int dim) {
    double epsilon = 1E-8;

    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            if( fabs(mat_a[i*dim + j] - mat_b[i*dim + j]) > epsilon) {
                printf("le matrici differiscono al campo [%d, %d]\n", i, j);
                printf("\t%f != %f\n", mat_a[i*dim + j], mat_b[i*dim + j]);
                
                return;
            } 
        }     
    }

    printf("le matrici sono uguali\n");
}

double calcola_media_intorno(const double* mat_input, int dim_input, int riga, int colonna) {
    double somma = 0.0;

    /*
        considero l'intorno 3x3 dell'elemento corrente
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

double pseudo_convoluzione(const double* mat_input, double* mat_output, int dim_input, int dim_output) {
    double start = omp_get_wtime();

    // scorro gli elementi PARI della matrice di input
    // es: (0; 0), (0; 2), ..., (2; 0), (2; 2), ...
    for(int i=0; i<dim_input; i+=2) {
        for(int j=0; j<dim_input; j+=2) {
            // calcolo la media dell'intorno del punto corrente
            double media_intorno = calcola_media_intorno(mat_input, dim_input, i, j);
            mat_output[(i/2)*dim_output + (j/2)] = media_intorno;
        }     
    }

    double end = omp_get_wtime();

    return end-start;
}


int main(int argc, char** argv) {
    if(argc < 3) {
        printf("usage: ./conv <dim_matrix> <proc>\n");
        exit(1);
    }

    int dim_matrix = atoi(argv[1]);
    int num_proc = atoi(argv[2]);
    // approssimo per eccesso (es: input 5x5 -> risultato 3x3)
    int dim_risultato = (dim_matrix+1) / 2;
    
    #ifdef DEBUG
    printf("Dim matrice in input: %dx%d\n", dim_matrix, dim_matrix);
    printf("Dim matrice in output: %dx%d\n", dim_risultato, dim_risultato);
    printf("Num di processi paralleli: %d\n", num_proc);
    #endif

    if(dim_risultato % num_proc != 0) {
        printf("il numero di processi (%d) non è un divisore della dimensione della matrice da calcolare: %d.\n", num_proc, dim_risultato);
        exit(1);
    }
    
    if(dim_risultato > dim_risultato) {
        printf("il numero di processi (%d) è maggiore della dimensione della matrice da calcolare (%d).\n", num_proc, dim_risultato);
        exit(1);
    }

    



    int size_input = dim_matrix*dim_matrix*sizeof(double);
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
    stampa_matrice_evidenziata(mat_input, dim_matrix);
    #endif

    double elapsed_sequential = pseudo_convoluzione(mat_input, mat_risultato, dim_matrix, dim_risultato);
    #ifdef DEBUG
    printf("--- MATRICE RISULTATO SEQUENZIALE ---\n");
    stampa_matrice(mat_risultato, dim_risultato);
    #endif


















    // array contenente il numero di elementi da mandare ai vari processi 
    int* send_counts = malloc(num_proc * sizeof(int));
    // array contente i vari offset da cui partire a mandare il numero di elementi contenuti in send_counts
    int* input_offsets = malloc(num_proc * sizeof(int));

    int num_righe_ris_per_processo = dim_risultato / num_proc;
    printf("ogni processo elabora %d righe della matrice risultato\n", num_righe_ris_per_processo);

    for(int i=0; i<num_proc; i++) {
        // ogni riga della matrice risultato si ottiene
        //  - considerando elementi di 3 righe della matrice di input
        //  - considerando solo una volta le righe in comune (num_righe_ris_per_processo-1)
        send_counts[i] = num_righe_ris_per_processo*3*dim_matrix - (num_righe_ris_per_processo-1)*dim_matrix;
        
        // ogni processo (diverso dal primo) parte da una riga indietro rispetto all'elemento da calcolare
        if(i==0)
            input_offsets[i] = 0;
        else
            input_offsets[i] = input_offsets[i-1] + (send_counts[i-1]-dim_matrix);

        // Se sono il primo processo il primo blocco di righe della matrice di input è "spesso" soltanto due;
        // tolgo la dimensione di una riga al numero di elementi da mandare
        if(i==0) {
            send_counts[i] -= dim_matrix;
        }
        // Se sono l'ultimo processo e la matrice ha dimensione dispari l'ultimo blocchi di riga della matrice di
        // input è "spesso" soltanto due; 
        if(i == num_proc-1 && dim_matrix%2 == 1) {
            send_counts[i] -= dim_matrix;
        }
    }
    
    for(int i=0; i<num_proc; i++) {
        printf("proc: %d\n", i);
        printf("\tsend_counts: %d\n", send_counts[i]);
        printf("\tinput_offsets: %d\n", input_offsets[i]);
    }

    double* mat_risultato_scatterv = malloc(size_risultato);
    for (int i = 0; i < size_risultato/sizeof(double); i++) {
        mat_risultato_scatterv[i] = 0.0;
    }

    for(int i=0; i<num_proc; i++) {
        for(int j=0; j<send_counts[i]; j++) {
            int riga = (j+input_offsets[i]) / dim_matrix;
            int colonna = (j+input_offsets[i]) % dim_matrix;

            if(riga%2 == 0 && colonna%2 == 0) {
                double media_intorno = calcola_media_intorno(mat_input, dim_matrix, riga, colonna);
                mat_risultato_scatterv[(riga/2)*dim_risultato + (colonna/2)] = media_intorno;
            }
        }     
    }

    #ifdef DEBUG
    printf("--- MATRICE RISULTATO SCATTERV ---\n");
    stampa_matrice(mat_risultato_scatterv, dim_risultato);
    #endif 

    check_matrici_uguali(mat_risultato, mat_risultato_scatterv, dim_risultato);

    free(mat_input);
    free(mat_risultato);
    free(mat_risultato_scatterv);
}