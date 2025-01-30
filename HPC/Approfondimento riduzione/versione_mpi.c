#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define DEBUG 1

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

double calcola_media_intorno_ris_parziale(const double* righe, int dim_riga, int num_elem, int riga, int colonna) {
    double somma = 0.0;

    /*
        considero l'intorno 3x3 dell'elemento corrente
    */
    for(int i=-1; i<=1; i++) {
        for(int j=-1; j<=1; j++) {
            int riga_intorno = riga + i;
            int colonna_intorno = colonna + j;

            // controllo se sto considerando l'intorno di un elemento su un bordo della matrice
            if( (riga_intorno<0 || riga_intorno*dim_riga>=num_elem) || (colonna_intorno<0 || colonna_intorno>=dim_riga) ) {
                somma += 0.0;
            }
            else {
                somma += righe[riga_intorno*dim_riga + colonna_intorno];
            }
        }
    }

    return somma/9;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("usage: ./versione_mpi <dim_matrix>\n");
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
        printf("\n");
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

    // Ogni nodo ha visibilità di un sottoinsieme di righe della matrice di input
    int num_righe_ris_per_processo = dim_result / num_proc;
    int size_my_righe = num_righe_ris_per_processo*dim_matrix*3*sizeof(double);
    double* my_righe = malloc(size_my_righe);
    // ogni nodo calcola un sottoinsieme delle righe della matrice risultato
    double* my_righe_ris = malloc(num_righe_ris_per_processo*dim_result*sizeof(double));

    #ifdef DEBUG
    printf("ogni processo elabora %d righe della matrice risultato\n", num_righe_ris_per_processo);
    printf("corrispondenti a %d elementi della matrice di input \n", num_righe_ris_per_processo*3*dim_matrix);
    MPI_Barrier(MPI_COMM_WORLD);
    #endif

    /*
        In seguito calcolo:
        - il numero di dati da mandare ad ogni processo
            - send_counts[num_proc]
        - gli offset rispetto all'inizio della matrice da cui far partire
            gli invii dei dati
            - input offsets[num_proc] 
    */
    int* send_counts = malloc(num_proc * sizeof(int));
    int* input_offsets = malloc(num_proc * sizeof(int));

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
    
    // il nodo di rank 0 definisce, inizializza e distribuisce la matrice di input
    double* mat_input;
    if (my_rank == 0) {
        #ifdef DEBUG
        printf("\n");
        for(int i=0; i<num_proc; i++) {
            printf("proc: %d\n", i);
            printf("\tsend_counts: %d\n", send_counts[i]);
            printf("\tinput_offsets: %d\n", input_offsets[i]);
        }
        printf("\n");
        #endif

        int size_input = dim_matrix*dim_matrix*sizeof(double);
        mat_input = malloc(size_input);

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

        // int MPI_Scatterv(
        //     const void *sendbuf,   // Buffer dei dati da inviare
        //     const int *sendcounts, // Numero di elementi da inviare a ciascun processo
        //     const int *displs,     // Offset nel buffer di invio per ogni processo
        //     MPI_Datatype sendtype, // Tipo di dato degli elementi inviati
        //     void *recvbuf,         // Buffer ricezione (su ogni processo)
        //     int recvcount,         // Numero di elementi ricevuti dal processo
        //     MPI_Datatype recvtype, // Tipo di dato degli elementi ricevuti
        //     int root,              // Rank del processo sender
        //     MPI_Comm comm          // Comunicatore MPI
        // );
        MPI_Scatterv(mat_input, send_counts, input_offsets, MPI_DOUBLE, my_righe, send_counts[my_rank], MPI_DOUBLE, 0, MPI_COMM_WORLD); // mittente
    }
    else {
        MPI_Scatterv(NULL, send_counts, input_offsets, MPI_DOUBLE, my_righe, send_counts[my_rank], MPI_DOUBLE, 0, MPI_COMM_WORLD); // destinatari
    }








    /*
        Ogni processo:
            - scorre tutti gli elementi ricevuti dalla scatterv
            - calcola la media dell'interno dei punti giusti
            - costruisce il proprio pezzo della matrice risultato
    */
    int riga_ris_parziale = 0;
    int colonna_ris_parziale = 0;
    for(int j=0; j<send_counts[my_rank]; j++) {
        int riga_globale    = (j+input_offsets[my_rank]) / dim_matrix;
        int colonna_globale = (j+input_offsets[my_rank]) % dim_matrix;

        // calcolo la media dell'intorno dei soli elementi con entrambi gli indici pari
        if(riga_globale%2 == 0 && colonna_globale%2 == 0) {
            double media_intorno = calcola_media_intorno_ris_parziale(my_righe, dim_matrix, send_counts[my_rank], riga_ris_parziale, colonna_ris_parziale);
            my_righe_ris[riga_ris_parziale*dim_result + colonna_ris_parziale] = media_intorno;

            #ifdef DEBUG
            printf("\tProcesso %d, ha calcolato my_righe[%d][%d] = %0.2f\n", my_rank, riga_ris_parziale, colonna_ris_parziale, media_intorno);
            #endif

            colonna_ris_parziale++;
            if(colonna_ris_parziale == dim_result) {
                riga_ris_parziale++;
                colonna_ris_parziale = 0;
            }
        }
    }     

    MPI_Barrier(MPI_COMM_WORLD);



  

        
    //collettore 
    if (my_rank==0) {   
        int size_mat_result = dim_result*dim_result*sizeof(double);
        double* mat_result = malloc(size_mat_result);

        // int MPI_Gatherv(
        //     const void *sendbuf,   // Buffer locale di invio (ogni processo invia i propri dati)
        //     int sendcount,         // Numero di elementi inviati dal processo corrente
        //     MPI_Datatype sendtype, // Tipo di dati degli elementi inviati
        //     void *recvbuf,         // Buffer di ricezione (solo il root lo usa)
        //     const int *recvcounts, // Array: numero di elementi ricevuti da ciascun processo
        //     const int *displs,     // Array: offset nel buffer di ricezione per ogni processo
        //     MPI_Datatype recvtype, // Tipo di dati degli elementi ricevuti
        //     int root,              // Rank del processo root
        //     MPI_Comm comm          // Comunicatore MPI
        // );
        MPI_Gatherv(my_righe_ris, send_counts[my_rank], MPI_DOUBLE, mat_result, send_counts, input_offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);  // ricevo

        #ifdef DEBUG
        printf("\n--- MATRICE RISULTATO ---\n");
        stampa_matrice(mat_result, dim_result);
        #endif 

        free(mat_result);
        // ora che ho eventualmente fatto anche il controllo sulla correttezza del risultato libero
        free(mat_input);
    }
    else {
        MPI_Gatherv(my_righe_ris, send_counts[my_rank], MPI_DOUBLE, NULL, send_counts, input_offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);    // invio
    }
    
    free(my_righe);
    free(my_righe_ris);
    free(send_counts);
    free(input_offsets);

    MPI_Finalize();
    
    
    return EXIT_SUCCESS;
}

