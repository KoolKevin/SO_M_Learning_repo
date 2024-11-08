#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define N 10    //num persone
#define K 10    //num film

typedef struct{
    char film[K][40];
    int voti[K];
    int num_pareri;

    pthread_mutex_t mutex;
} sondaggio;

// variabili globali
sondaggio s;
char* MAX_FILM;

sem_t barriera;
int completati = 0;

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
    int spettatore = (int)arg;
    //accesso alla struttura dati condivisa
    pthread_mutex_lock(&(s.mutex)); /* prologo */
    printf("Spettatore numero %d sta votando\n", spettatore);
    s.num_pareri++;

    for(int i=0; i<K; i++) {
        s.voti[i] += 1+rand()%10;
        printf("\tFilm: %s;\t voto_medio: %0.2f;\n", s.film[i],  (float)s.voti[i]/s.num_pareri);
    }
    
    printf("\tnum pareri: %d\n", s.num_pareri);
    pthread_mutex_unlock(&(s.mutex)); /*epilogo */

    //barriera di sincronizzazione
    pthread_mutex_lock(&(s.mutex));
    completati++;
    if (completati == N) {
        //l'ultimo a terminare calcola anche il film vincitore
        float max = 0;

        for(int i=0; i<K; i++) {
            float voto_medio = (float)s.voti[i] / s.num_pareri;

            if( voto_medio > max ) {
                max = voto_medio;
                MAX_FILM = s.film[i];
            }
        }
        printf("Lo spettatore: %d ha calcolato il film vincitore: %s!\n\n", spettatore, MAX_FILM);

        sleep(1);
        //sblocco la barriera
        sem_post(&barriera);
    } 
    pthread_mutex_unlock(&(s.mutex));
    sem_wait(&barriera);
    sem_post(&barriera);

    //scarico e vedo il film vincitore (variabile globale perchè non voglio ripetere il calcolo 10 volte)
    printf("Lo spettatore: %d sta scaricando il film vincitore: %s\n", spettatore, MAX_FILM);

    pthread_exit(NULL);
}

int main() { 
    pthread_t threads[N];
    int rj;

    srand(time(0)); 
    inizializzaSondaggio(&s);
    stampaSondaggio(s);
    sem_init (&barriera, 0, 0);

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

    printf("\nFilm vincitore: %s, scaricato e in visione da tutti!\n", MAX_FILM);
}