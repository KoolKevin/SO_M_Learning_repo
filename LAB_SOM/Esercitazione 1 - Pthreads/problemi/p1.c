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

void inizializzaSondaggio(sondaggio* s) {
    for(int i=0; i<K; i++) {
        snprintf(s->film[i], 40, "Film %d", i+1);
        s->voti[i] = 0;
    }

    s->num_pareri   = 0;
    pthread_mutex_init(&(s->mutex), NULL); 
}

void stampaSondaggio(sondaggio s) {
    for(int i=0; i<K; i++) {
        printf("FILM: %s;\t VOTO: %d;\n", s.film[i], s.voti[i]);
    }

    printf("\n");
}

void* vota(void* arg) {
    //accesso alla struttura dati condivisa
    pthread_mutex_lock(&(s.mutex)); /* prologo */
    
    printf("Spettatore numero %d sta votando\n", (int)arg);
    for(int i=0; i<K; i++) {
        s.voti[i] += 1+rand()%10;
    }
    
    stampaSondaggio(s);
    s.num_pareri++;
    printf("\t\tnum pareri: %d\n", s.num_pareri);

    pthread_mutex_unlock(&(s.mutex)); /*epilogo */

    pthread_exit(NULL);
}

int main() { 
    pthread_t threads[N];
    int rj;
    srand(time(0)); 
    
    /* INIZIALIZZA STRUTTURA DATI E SEMAFORO */
    inizializzaSondaggio(&s);
    stampaSondaggio(s);

    for(int i=0; i<N; i++) {
        if (pthread_create(&threads[i], NULL, vota, (void *)i) < 0) {
            fprintf (stderr, "create error for thread 1\n");
            exit(1);
        }
    }
    
    for(int i=0; i<N; i++) {
        rj = pthread_join(threads[i], NULL);

        if (rj)
            printf("ERRORE join thread %ld codice %d\n", threads[i], rj);
    }

    //thread hanno terminato
    int max = 0;
    char* maxFilm;

    for(int i=0; i<K; i++) {
        int voto_medio = s.voti[i] / s.num_pareri;
        printf("FILM: %s;\t VOTO MEDIO: %d;\n", s.film[i], voto_medio);

        if( voto_medio > max ) {
            max = voto_medio;
            maxFilm = s.film[i];
        }
    }

    printf("\nFilm vincitore: %s!\n", maxFilm);
}