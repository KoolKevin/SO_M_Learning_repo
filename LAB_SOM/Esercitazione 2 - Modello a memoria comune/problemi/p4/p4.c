#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#define N 10    //num persone
#define K 10    //num film

#define MAX_DOWNLOAD 3 
#define MAX_DURATA_DOWNLOAD 3

typedef struct{
    char film[K][40];
    int voti[K];
    int medie_utenti[N];
    int num_pareri;

    sem_t barriera;
    int completati;
    pthread_mutex_t mutex;
} sondaggio;

typedef struct{
    int download_disponibili;

    // semafori privati su cui sospendere i partecipanti al sondaggio se:
    //  - download_disponibili == 0
    //  - c'è un altro spettatore sospeso con una media più alta
    sem_t sem_privati[N];
    int sospesi[N];  
    pthread_mutex_t sem_mutex;
} gestore_download;

// variabili globali
sondaggio s;
gestore_download g;
char* WINNER_FILM;

void inizializzaSondaggio(sondaggio* s) {
    for(int i=0; i<K; i++) {
        snprintf(s->film[i], 40, "Film %d", i+1);
        s->voti[i] = 0;
    }
    //medie_utenti non ha bisogno di inizializzazione
    s->num_pareri   = 0;

    sem_init (&(s->barriera), 0, 0);
    s->completati = 0;
    pthread_mutex_init(&(s->mutex), NULL); 
}

void stampaSondaggio(sondaggio s) {
    for(int i=0; i<K; i++) {
        printf("FILM: %s;\t VOTO: %d;\n", s.film[i], s.voti[i]);
    }

    printf("\n");
}

void inizializzaGestoreDownload(gestore_download* g) {
    g->download_disponibili = MAX_DOWNLOAD;
    
    for(int i=0; i<N; i++) {
        sem_init(&(g->sem_privati[i]), 0, 0);
        g->sospesi[i] = 0;
    }

    pthread_mutex_init(&(g->sem_mutex), NULL); 
}

int checkSospesoPiuPrioritario(int spettatore) {
    int result = 0;
    int media_spettatore = s.medie_utenti[spettatore];

    for(int i=0; i<N; i++) {
        if(g.sospesi[i] != 0 && s.medie_utenti[i] > media_spettatore) {
            result = i;
        }
    }

    if(result)
        printf("\tlo spettatore %d con priorità %d deve aspettare lo spettatore %d con priorità %d\n", spettatore, media_spettatore, result, s.medie_utenti[result]);

    return result;
}

int inizio_download(int spettatore) {  
    pthread_mutex_lock(&(g.sem_mutex));

    printf("LO SPETTATORE %d VUOLE SCARICARE IL FILM\n", spettatore);  

    while(g.download_disponibili == 0 || checkSospesoPiuPrioritario(spettatore)) { 
        g.sospesi[spettatore]++;
        printf("spettatore %d ha priorità %d, deve aspettare prima di scaricare\n", spettatore, s.medie_utenti[spettatore]);
        printf("\n");
        pthread_mutex_unlock(&(g.sem_mutex));

        sem_wait(&g.sem_privati[spettatore]);

        pthread_mutex_lock(&(g.sem_mutex));
        printf("spettatore %d ha priorità %d, può ritentare a scaricare\n", spettatore, s.medie_utenti[spettatore]);
    }

    g.download_disponibili--;
    printf("donwload disponibili: %d\n", g.download_disponibili);

    int durata_download = 1 + rand() % (MAX_DURATA_DOWNLOAD);
    printf("\tLo spettatore %d ci metterà %d minuti a scaricare il film vincitore %s\n\n", spettatore, durata_download, WINNER_FILM);

    pthread_mutex_unlock(&(g.sem_mutex));

    return durata_download;
}

void fine_download(int spettatore) {
    pthread_mutex_lock(&(g.sem_mutex));

    printf("LO SPETTATORE %d HA FINITO DI SCARICARE IL FILM\n", spettatore);  
    g.download_disponibili++;
    printf("donwload disponibili: %d\n", g.download_disponibili);
    printf("\n");

    //risveglio tutti solamente quelli con priorità più alta riusciranno ad uscire dal while
    int fine = 0;

    for (int i=0; i<N; i++) {   
        if(g.sospesi[i] != 0) {
            /* PROBLEMA: qua modifico i sospesi che in realtà mi servono nel ciclo sopra per capire se c'è qualcuno di più prioritario */
            g.sospesi[i]--;     
            sem_post(&g.sem_privati[i]);
        }
    }

    pthread_mutex_unlock(&(g.sem_mutex));
}

void* vota(void* arg) {
    int spettatore = (int)arg;
    //accesso alla struttura dati condivisa
    pthread_mutex_lock(&(s.mutex)); /* prologo */
    printf("Spettatore numero %d sta votando\n", spettatore);
    s.num_pareri++;

    int somma_voti_spettatore = 0;
    for(int i=0; i<K; i++) {
        int voto = 1+rand()%10;
        s.voti[i] += voto;
        somma_voti_spettatore += voto;
        printf("\tFilm: %s;\t voto_medio: %0.2f;\n", s.film[i],  (float)s.voti[i]/s.num_pareri);
    }

    s.medie_utenti[spettatore] = roundf( (float)somma_voti_spettatore / K );
    
    printf("\tmedia voti spettatore %d = %d\n", spettatore, s.medie_utenti[spettatore]);
    printf("\tnum pareri: %d\n", s.num_pareri);
    pthread_mutex_unlock(&(s.mutex)); /*epilogo */

    //barriera di sincronizzazione
    pthread_mutex_lock(&(s.mutex));
    s.completati++;
    if (s.completati == N) {
        //l'ultimo a terminare calcola anche il film vincitore
        float max = 0;

        for(int i=0; i<K; i++) {
            float voto_medio = (float)s.voti[i] / s.num_pareri;

            if( voto_medio > max ) {
                max = voto_medio;
                WINNER_FILM = s.film[i];
            }
        }
        printf("Lo spettatore: %d ha calcolato il film vincitore: %s!\n\n", spettatore, WINNER_FILM);

        sleep(1);
        //sblocco la barriera
        sem_post(&s.barriera);
    } 
    pthread_mutex_unlock(&(s.mutex));
    sem_wait(&s.barriera);
    sem_post(&s.barriera);    // risveglio a tornello

    //scarico e vedo il film vincitore 
    int durata_download = inizio_download(spettatore);

    sleep(durata_download);

    fine_download(spettatore);

    printf("Lo spettatore: %d sta vedendo il film vincitore: %s\n", spettatore, WINNER_FILM);

    pthread_exit(NULL);
}

int main() { 
    pthread_t threads[N];
    int rj;

    srand(time(0)); 
    inizializzaSondaggio(&s);
    stampaSondaggio(s);
    inizializzaGestoreDownload(&g);

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

    printf("\nFilm vincitore: %s, scaricato e in visione da tutti!\n", WINNER_FILM);
}