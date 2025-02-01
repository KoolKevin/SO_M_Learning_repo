#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

// #define DEBUG 1
#define DEBUG_TEMPI 1
// #define CHECK_CORRETTEZZA 1

#define RESET   "\x1b[0m"    // Resetta i colori
#define RED     "\x1b[31m"   // Rosso
#define GREEN   "\x1b[32m"   // Verde

#ifdef CHECK_CORRETTEZZA
void check_matrici_uguali(const double* mat_a, const double* mat_b, int dim) {
    double epsilon = 1E-8;

    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            if( fabs(mat_a[i*dim + j] - mat_b[i*dim + j]) > epsilon) {
                printf("%sle matrici differiscono al campo [%d, %d]\n", i, j, RED);
                printf("\t%f != %f%s\n", mat_a[i*dim + j], mat_b[i*dim + j], RESET);
                
                return;
            } 
        }     
    }

    printf("\n%sLE MATRICI SONO UGUALI%s\n\n", GREEN, RESET);
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

void pseudo_convoluzione(const double* mat_input, double* mat_output, int dim_input, int dim_output) {
    // scorro gli elementi PARI della matrice di input
    // es: (0; 0), (0; 2), ..., (2; 0), (2; 2), ...
    for(int i=0; i<dim_input; i+=2) {
        for(int j=0; j<dim_input; j+=2) {
            // calcolo la media dell'intorno del punto corrente
            double media_intorno = calcola_media_intorno(mat_input, dim_input, i, j);
            mat_output[(i/2)*dim_output + (j/2)] = media_intorno;
        }     
    }
}
#endif



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
    #ifdef DEBUG
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    #endif
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
                #ifdef DEBUG
                printf("\tProcesso %d; blocco[%d][%d] => +0\n", my_rank, i, j);
                #endif
                somma += 0.0;
            }
            else {
                #ifdef DEBUG
                printf("\tProcesso %d; blocco[%d][%d] => + %f\n", my_rank, i, j, righe[riga_intorno*dim_riga + colonna_intorno]);
                #endif
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
    int dim_result = (dim_matrix+1)/2;  // approssimato per eccesso

    int num_proc, my_rank;
    double start, end, local_elapsed, global_elapsed;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int num_righe_ris_per_processo = dim_result / num_proc;

    if (my_rank == 0) {
        printf("\n");
        printf("Dim matrice in input: %dx%d\n", dim_matrix, dim_matrix);
        printf("Dim matrice in output: %dx%d\n", dim_result, dim_result);
        printf("Num di processi paralleli: %d\n", num_proc);
        printf("Num di righe del risultato calcolate da ogni singolo processo: %d\n", num_righe_ris_per_processo);
    }

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

        NB: considero questo tempo nella misurazione del
        tempo di calcolo della semi-riduzione  
    */
    MPI_Barrier(MPI_COMM_WORLD);
    start=MPI_Wtime();

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

    MPI_Barrier(MPI_COMM_WORLD);
    end=MPI_Wtime();
    local_elapsed = end-start;

    #ifdef DEBUG_TEMPI
    printf("[processo %d]: %f ms per preparare l'invio dei dati\n", my_rank, local_elapsed*1000);
    #endif

    // il nodo di rank 0 definisce, inizializza e distribuisce la matrice di input
    // (NB: non considero l'inizializzazine della matrice nel tempo di calcolo analogmanete
    //  a come ho fatto con OMP)
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

        start=MPI_Wtime();
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

        #ifdef DEBUG
        printf("Processo %d: \n", my_rank);
        for(int i=0; i<send_counts[my_rank]; i++) {
            printf("\tricevo %f\n", my_righe[i]);
        }
        printf("\n");
        #endif
    }
    else {
        start=MPI_Wtime();
        MPI_Scatterv(NULL, send_counts, input_offsets, MPI_DOUBLE, my_righe, send_counts[my_rank], MPI_DOUBLE, 0, MPI_COMM_WORLD); // destinatari

        #ifdef DEBUG
        printf("Processo %d: \n", my_rank);
        for(int i=0; i<send_counts[my_rank]; i++) {
            printf("\tricevo %f\n", my_righe[i]);
        }
        printf("\n");
        #endif
    }








    /*
        Ogni processo:
            - scorre tutti gli elementi ricevuti dalla scatterv
            - calcola la media dell'intorno dei punti giusti
            - costruisce il proprio pezzo della matrice risultato
    */
    int riga_ris_parziale = 0;
    int colonna_ris_parziale = 0;
    for(int j=0; j<send_counts[my_rank]; j++) {
        int riga_globale    = (j+input_offsets[my_rank]) / dim_matrix;
        int colonna_globale = (j+input_offsets[my_rank]) % dim_matrix;
        int riga_locale    = j / dim_matrix;
        int colonna_locale = j % dim_matrix;

        // calcolo la media dell'intorno dei soli elementi con entrambi gli indici pari
        if(riga_globale%2 == 0 && colonna_globale%2 == 0) {
            double media_intorno = calcola_media_intorno_ris_parziale(my_righe, dim_matrix, send_counts[my_rank], riga_locale, colonna_locale);
            my_righe_ris[riga_ris_parziale*dim_result + colonna_ris_parziale] = media_intorno;

            #ifdef DEBUG
            printf("\tProcesso %d, ha calcolato mat_input[%d][%d] = %0.2f\n", my_rank, riga_globale, colonna_globale, media_intorno);
            #endif

            colonna_ris_parziale++;
            if(colonna_ris_parziale == dim_result) {
                riga_ris_parziale++;
                colonna_ris_parziale = 0;
            }
        }
    }     

    #ifdef DEBUG
    MPI_Barrier(MPI_COMM_WORLD);    // per ordinare le stampe
    #endif


        
    //collettore 
    if (my_rank==0) {   
        int size_result = dim_result*dim_result*sizeof(double);
        double* mat_result = malloc(size_result);

        MPI_Gather(my_righe_ris, num_righe_ris_per_processo*dim_result, MPI_DOUBLE,
                   mat_result, num_righe_ris_per_processo*dim_result, MPI_DOUBLE, 0, MPI_COMM_WORLD);  // ricevo


        #ifdef DEBUG
        printf("Processo %d: \n", my_rank);
        for(int i=0; i<dim_result*num_righe_ris_per_processo; i++) {
            printf("\tinvia %f\n", my_righe_ris[i]);
        }
        printf("\n");
        
        printf("\n--- MATRICE RISULTATO ---\n");
        stampa_matrice(mat_result, dim_result);
        #endif

        #ifdef CHECK_CORRETTEZZA
        double* mat_result_sequenziale = malloc(size_result);
        pseudo_convoluzione(mat_input, mat_result_sequenziale, dim_matrix, dim_result);

        #ifdef DEBUG
        printf("--- MATRICE RISULTATO SEQUENZIALE ---\n");
        stampa_matrice(mat_result_sequenziale, dim_result);
        #endif
        
        check_matrici_uguali(mat_result, mat_result_sequenziale, dim_result);

        free(mat_result_sequenziale);
        #endif 

        free(mat_result);
        free(mat_input);
    }
    else {
        MPI_Gather(my_righe_ris, num_righe_ris_per_processo*dim_result, MPI_DOUBLE,
                   NULL, num_righe_ris_per_processo*dim_result, MPI_DOUBLE, 0, MPI_COMM_WORLD);  // ricevo

        #ifdef DEBUG
        printf("Processo %d: \n", my_rank);
        for(int i=0; i<dim_result*num_righe_ris_per_processo; i++) {
            printf("\tinvia %f\n", my_righe_ris[i]);
        }
        printf("\n");
        #endif
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end=MPI_Wtime();
    local_elapsed += end-start;

    #ifdef DEBUG_TEMPI
    printf("\t[processo %d]: %f ms per concludere\n", my_rank, local_elapsed*1000);
    #endif

    PI_Barrier(MPI_COMM_WORLD); // per stampa
    
    // NB: riduco al risultato minimo! Siccome i processi non master (rank != 0)
    // aspettano l'inizializzazione della matrice fatta dal processo master.
    // Questo tempo NON lo voglio considerare.
    MPI_Reduce(&local_elapsed, &global_elapsed, 1 , MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    if (my_rank == 0) { 
        printf("\n\n\n%sTempo impiegato: %f ms%s\n", GREEN, global_elapsed*1000, RESET);
        printf("----------------------- FINE -----------------------\n\n\n");
    }
    
    free(my_righe);
    free(my_righe_ris);
    free(send_counts);
    free(input_offsets);

    MPI_Finalize();
    
    
    return EXIT_SUCCESS;
}

