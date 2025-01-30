#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// #define DEBUG 1

void stampa_matrice(const double* mat, int dim) {
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            printf("\t%0.2f", mat[i*dim + j]);
        }     

        printf("\n");   
    }

    printf("\n");
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("usage: ./conv <dim_matrix>\n");
        exit(1);
    }

    int dim_matrix = atoi(argv[1]);
    int dim_result = (dim_matrix+1)/2;

    MPI_Init(&argc, &argv);
    
    int num_proc, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    #ifdef DEBUG
    if (my_rank == 0) {
        printf("Dim matrice in input: %dx%d\n", dim_matrix, dim_matrix);
        printf("Dim matrice in output: %dx%d\n", dim_result, dim_result);
        printf("Num di processi paralleli: %d\n", num_proc);
    }
    #endif

    /*
        IPOTESI SEMPLIFICATIVA:
        La dimensione della matrice in output è multipla del numero di nodi; in questa
        maniera posso trasferire direttamente dei "blocchi" di righe ai vari nodi.

        Se non facessi questa ipotesi, per distribuire i dati ad ogni nodo, dovrei:
        - costruirmi  una matrice di appoggio in cui memorizzo i vari blocchi 3x3 linearmente in memoria. 
        - invocare una scatter su questa matrice di appoggio e non su quella di input (memorizzata in
          maniera row-major e con quindi i dati dei blocchi 3x3 sparsi in memoria con un offset pari alla
          dimensione della riga)

        In conclusione:
            - num_proc deve essere un divisore di dim_result
            - ogni processo calcola blocchi di righe della matrice risultato
    */
    if(dim_result % num_proc != 0) {
        printf("il numero di processi (%d) non è un divisore della dimensione della matrice da calcolare: %d.\n", num_proc, dim_result);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    
    if(num_proc > dim_result) {
        printf("il numero di processi (%d) è maggiore della dimensione della matrice da calcolare (%d).\n", num_proc, dim_result);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // la matrice B deve essere visibile a tutti
    int mat_B[DIM][DIM];
    // sottoporzioni delle matrici assegnate ai vari nodi
    int  my_A[DIM*(DIM/num_proc)], my_C[DIM*(DIM/num_proc)];
    
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

        // DIM/num_proc = numero di righe assegnate ad ogni processo
        // Ogni riga ha poi DIM elementi
        MPI_Scatter(mat_A, DIM*(DIM/num_proc), MPI_INT, &my_A, DIM*(DIM/num_proc), MPI_INT, 0, MPI_COMM_WORLD); // mittente
    }
    else {
        MPI_Scatter(NULL, DIM*(DIM/num_proc), MPI_INT, &my_A, DIM*(DIM/num_proc), MPI_INT, 0, MPI_COMM_WORLD);  // destinatari
    }

    // printf("\tsono :%d my_rank; ho ricevuto la riga che inizia per %d\n", my_rank, my_A[0]);

    // tutti eseguono il brodcast della matrice operando B
    MPI_Bcast(mat_B, DIM*DIM, MPI_INT, 0, MPI_COMM_WORLD);
    
    // ogni nodo calcola la sua porzione
    for(int i=0; i < DIM/num_proc; i++) {       // questo scorre le righe della sotto-matrice my_A
        for(int k=0; k < DIM; k++) {                // questo scorre tutte le colonne della matriche B
            my_C[i*DIM + k] = 0; 

            for(int j=0; j < DIM; j++) {            // questo scorre gli elementi all'interno di una riga/colonna
                my_C[i*DIM + k] += my_A[i*DIM + j] * mat_B[j][k];   
                // printf("\t\tsono: %d; sto calcolando A[%d][%d] * B[%d][%d] = %d * %d => C[%d][%d]\n", my_rank, i, j, j, k,  my_A[i*DIM + j], mat_B[j][k], i, k);
            }
        }
    }

    printf("\tsono :%d my_rank; ho calcolato delle righe che iniziano per %d\n", my_rank, my_C[0]);
        
    //collettore 
    if (my_rank==0) {   
        int mat_C[DIM][DIM];
        MPI_Gather(&my_C, DIM*(DIM/num_proc), MPI_INT, mat_C, DIM*(DIM/num_proc), MPI_INT, 0, MPI_COMM_WORLD);  // ricevo

        printf("\nRisultato C=A*B:\n");
        for(int i=0; i<DIM; i++) {
            printf("\t[ ");

            for(int j=0; j<DIM; j++) {
                printf("%d\t", mat_C[i][j]);
            }

            printf(" ]\n");
        }
    }
    else {
        MPI_Gather(&my_C, DIM*(DIM/num_proc), MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);    // invio
    }
    
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}

