#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10    //num persone
#define K 10    //num film

typedef struct{
    char film[K][40];
    int voti[K];
    int num_pareri;

    pthread_mutex_t mutex;
} sondaggio;

sondaggio s;

void stampaSondaggio(sondaggio s) {
    for(int i=0; i<K; i++) {
        printf("FILM: %s ; VOTO: %d\n", film[k], )
    }
}

void *thread1_process (void * arg) {
    int k=1;

    pthread_mutex_lock(&s.mutex); 
    
    for(int i=0; i<K; i++) {
        voti[i] += i

        printf("accessi di T1: %d\n", accessi1);    
    }

    pthread_mutex_unlock(&s.mutex); /*epilogo */

    pthread_exit (0);
}

main() { 
    pthread_t th1, th2;
    /* INIZIALIZZA STRUTTURA DATI E SEMAFORO */

    if (pthread_create(&th1, NULL, thread1_process, NULL) < 0) {
        fprintf (stderr, "create error for thread 1\n");
        exit(1);
    }

    pthread_join (th1, NULL);
    pthread_join (th2, NULL);
}