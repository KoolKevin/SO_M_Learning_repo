#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define N 20    // dimensione vettore
#define K 4     // K = dimensione porzione del vettore in cui verrà effettuata una ricerca da parte di un singolo thread

int V[N];       // DATO CONDIVISO

// codice worker
void* Calcolo(void* t) {
    int first, result=0;
    
    first = (int)t;     

    for (int i=first; i<first+K; i++)
        if (V[i]>result)
            result=V[i];

    printf("\tWorker:\tha calcolato il massimo locale: %d, operando su [%d; %d[\n", result, first, first+K); 

    // come fa result a persistere? pthread_exit fa una copia?
    pthread_exit((void*) result);
}

int main (int argc, char *argv[]) {
    int rc, t, first, status, max=0;

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
	    first=t*K; // passo ad ogni thread l'indice del primo elemento da elaborare

        rc = pthread_create(&thread[t], NULL, Calcolo, (void *)first);
        if (rc != 0) {
            printf("ERRORE CREAZIONE: %d\n", rc);
            exit(-1);   
        }
    }

    for(t=0; t<M; t++) {
        // se provi a togliere il cast a void * il compilatore si lamenta dicendo che vuole void **; 
        // ma se non è void** anche col cast, sta succedendo???
        rc = pthread_join(thread[t], (void *)&status);

        //raccolgo in status il valore calcolato dal figlio
        if (rc)
            printf("ERRORE join thread %d codice %d\n", t, rc);
        else {
            printf("Finito thread %d con ris. %d\n", t, status);

            if (status>max)
                max=status;
        }
    }

   printf("main-risultato finale: %d\n", max);
}
