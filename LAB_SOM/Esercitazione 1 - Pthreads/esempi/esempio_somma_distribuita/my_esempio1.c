#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define N 20    // dimensione vettore
#define K 4     // K = dimensione porzione del vettore in cui verrà effettuata una ricerca da parte di un singolo thread

int V[N];       // DATO CONDIVISO

/*
    Questa funzione ha tante responsabilità di gestione della memoria:
        - libera la memoria passata come argomento t
        - alloca la memoria per il risultato result
*/
void* Calcolo(void* t) {
    int first = *( (int *)t );  // prima cast e poi dereferenziazione
    free(t);

    //result deve essere un puntatore e persistere dopo la terminazione della funzione e quindi lo alloco dinamicamente
    int *result = (int *)malloc(sizeof(int));
    *result = 0;

    for (int i = first; i < first+K; i++)
        if (V[i] > *result)
            *result = V[i];

    printf("\tWorker:\tha calcolato il massimo locale: %d, operando su [%d; %d[\n", *result, first, first+K); 

    pthread_exit((void*) result);
}

int main (int argc, char *argv[]) {
    int rc, t, max=0;
    int *ris_parziale;

    int M=N/K;	// M = numero thread
    pthread_t thread[M];
    srand(time(0)); 

    printf("inizializzazione vettore V:\n");
	for(int i = 0; i < N; i++) {
	   V[i] = 1+rand() % 200;
	   printf("v[%d] = %d\n", i, V[i]);
   	}	
    printf("\n");
    
    // lancio degli M thread
    for(t=0; t<M; t++) {
        printf("Main:\tcreazione thread n. %d\n", t);
        int *first = (int *)malloc(sizeof(int));
	    *first = t*K; // passo ad ogni thread l'indice del primo elemento da elaborare
        //dentro calcolo si dealloca first
        rc = pthread_create(&thread[t], NULL, Calcolo, (void *)first);

        /*
            NB: se facessi così:

            int first = t*K; // passo ad ogni thread l'indice del primo elemento da elaborare
            rc = pthread_create(&thread[t], NULL, Calcolo, (void *)&first);

            thread diversi otterrebbero lo stesso riferimento a first e quindi si sovrascriverebbero a vicendo.
            La soluzione che ho trovato è allocare first fuori per poi deallocarla dentro la funzione.
                -> casino :(
        */
        if (rc != 0) {
            printf("ERRORE CREAZIONE: %d\n", rc);
            exit(-1);   
        }
    }

    for(t=0; t<M; t++) {
        rc = pthread_join(thread[t], (void **)&ris_parziale);

        //raccolgo in ris_parziale il valore calcolato dal figlio
        if (rc)
            printf("ERRORE join thread %d codice %d\n", t, rc);
        else {
            printf("Finito thread %d con risultato parziale %d\n", t, *ris_parziale);

            if (*ris_parziale > max)
                max = *ris_parziale;
            
            //dealloco la memoria che ho allocato per far persistere il risultato parziale
            free(ris_parziale);
        }
    }

   printf("main-risultato finale: %d\n", max);
}
