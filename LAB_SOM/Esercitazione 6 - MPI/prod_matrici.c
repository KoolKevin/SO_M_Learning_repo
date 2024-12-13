#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define DIM 3

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int process_size, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &process_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if(process_size > DIM) {
        printf("il numero di processi: %d è maggiore della dimensione dei dati: %d.\n", process_size, DIM);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    // ipotesi semplificativa: accetto solo un numero di nodi tale da dividere
    // per righe intere la matriche DIM*DIM ->  process_size deve essere un divisore di DIM
    if(DIM % process_size != 0) {
        printf("il numero di processi: %d non è un divisore della dimensione dei dati: %d.\n", process_size, DIM);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    
    // la matrice B deve essere visibile a tutti
    int mat_B[DIM][DIM];
    // sottoporzioni delle matrici assegnate ai vari nodi
    int  my_A[DIM*DIM], my_C[DIM*DIM];
    
    if (my_rank == 0) {
        int mat_A[DIM][DIM];
        
        //inizializzazione e distribuzione della matrice operando A
        srand((unsigned int)time(NULL)); 
        for(int i=0; i<DIM; i++) {
            for(int j=0; j<DIM; j++) {
                mat_A[i][j] = rand()%10 + 1;
                mat_B[i][j] = rand()%10 + 1;
            }
        }
        printf("\nOperando A:\n");
        for(int i=0; i<DIM; i++) {
            printf("\t[ ");
            for(int j=0; j<DIM; j++) {
                printf("%d\t", mat_A[i][j]);
            }
            printf(" ]\n");
        }

        printf("\nOperando B:\n");
        for(int i=0; i<DIM; i++) {
            printf("\t[ ");
            for(int j=0; j<DIM; j++) {
                printf("%d\t", mat_B[i][j]);
            }
            printf(" ]\n");
        }

        MPI_Scatter(mat_A, DIM*DIM/process_size, MPI_INT, &my_A, DIM*DIM/process_size, MPI_INT, 0, MPI_COMM_WORLD);   
    }
    else {
        MPI_Scatter(NULL, DIM*DIM/process_size, MPI_INT, &my_A, DIM*DIM/process_size, MPI_INT, 0, MPI_COMM_WORLD);
    }
    // tutti eseguono il brodcast della matrice operando B
    MPI_Bcast(mat_B, DIM*DIM, MPI_INT, 0, MPI_COMM_WORLD);
    
    //TODO: qua gli indici sono scazzati, aggiusta
    // ogni nodo calcola la sua porzione
    for(int i=0; i < DIM*DIM/process_size; i++) {
        for(int j=0; j<DIM; j++) {
            // l'indice di riga per la matrice B può eccedere la dimensione
            // della riga singola e quindi devo aggiungere un modulo
            my_C[i] += my_A[i] * mat_B[i%DIM][j];   
        }
    }
        
    //collettore 
    if (my_rank==0) {   
        int mat_C[DIM][DIM];
        MPI_Gather(&my_C, DIM*DIM/process_size, MPI_INT, mat_C, DIM*DIM/process_size, MPI_INT, 0, MPI_COMM_WORLD);

        printf("\nRisultato C=A*B:\n");
        for(int i=0; i<DIM; i++) {
            printf("\t[ ");
            for(int j=0; j<DIM; j++) {
                printf("%d\t", mat_C[i][j]);
            }
            printf(" ]\n");
        }
    }
    else
        MPI_Gather(&my_C, DIM*DIM/process_size, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}

