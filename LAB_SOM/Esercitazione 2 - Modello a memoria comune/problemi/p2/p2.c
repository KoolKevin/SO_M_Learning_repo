#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_POSTI 6    
#define MAX_DIM_GRUPPO 1
#define MAX_AUTO 2      // da 1 a 5 persone di un gruppo per auto

#define NUMERO_GRUPPI 10 
#define MAX_DURATA_VISITA 3

typedef struct{
    int posti_liberi;
    int auto_libere;

    // semaforo condizione su cui sospendere i gruppi di visitatori se:
    //  - posti_liberi < dimensione_gruppo
    //  - auto_libere == 0
    //  - c'è un gruppo che è arrivato prima
    sem_t sem_condizione;    
    int prossimo_gruppo;  
    int gruppi_sospesi[NUMERO_GRUPPI];
    pthread_mutex_t sem_mutex;
} parco;

typedef struct{
    int num_persone;
    int posizione_in_coda;
} gruppo;

// variabili globali
parco p;

void inizializzaParco(parco* p) {
    p->posti_liberi = MAX_POSTI;
    p->auto_libere = MAX_AUTO;
    
    sem_init(&(p->sem_condizione), 0, 0);
    p->prossimo_gruppo = 0;
    for(int i=0; i<NUMERO_GRUPPI; i++)
        p->gruppi_sospesi[i] = 0;
    pthread_mutex_init(&(p->sem_mutex), NULL); 
}

void stampaParco(parco p) {
    printf("PARCO:\n");
    printf("\tposti liberi: %d\n", p.posti_liberi);
    printf("\tauto libere: %d\n", p.auto_libere);
    printf("\tprossimo gruppo che deve entrare: %d\n", p.prossimo_gruppo);
}

void inizializzaGruppi(gruppo** gruppi) {
    *gruppi = (gruppo*)malloc(sizeof(gruppo)*NUMERO_GRUPPI);   

    for(int i=0; i<NUMERO_GRUPPI; i++) {
        (*gruppi)[i].num_persone = 1 + rand() % (MAX_DIM_GRUPPO);
        (*gruppi)[i].posizione_in_coda = i;
    }
}

void stampaGruppi(gruppo* gruppi, int num_gruppi) {
     printf("GRUPPI:\n");
    for(int i=0; i<num_gruppi; i++) {
        printf("\tnumero persone: %d\n", gruppi[i].num_persone); 
        printf("\tposizione_in_coda: %d\n", gruppi[i].posizione_in_coda);  
        printf("\n");
    }
}   

void entrata(gruppo g) {  
    pthread_mutex_lock(&(p.sem_mutex));

    printf("GRUPPO %d VUOLE ENTRARE\n", g.posizione_in_coda);  

    while (p.posti_liberi < g.num_persone || p.auto_libere == 0 || g.posizione_in_coda != p.prossimo_gruppo) { 
        p.gruppi_sospesi[g.posizione_in_coda]++;
        printf("gruppo %d deve aspettare prima di entrare\n", g.posizione_in_coda);
        printf("\n");
        pthread_mutex_unlock(&(p.sem_mutex));

        sem_wait(&p.sem_condizione);

        pthread_mutex_lock(&(p.sem_mutex));
        p.gruppi_sospesi[g.posizione_in_coda]--;
        printf("gruppo %d può ritentare ad entrare\n", g.posizione_in_coda);
    }

    p.posti_liberi -= g.num_persone;
    p.auto_libere--;
    p.prossimo_gruppo++;
    stampaParco(p);

    pthread_mutex_unlock(&(p.sem_mutex));
}

void uscita(gruppo g) {
    pthread_mutex_lock(&(p.sem_mutex));
    printf("GRUPPO %d ESCE E LIBERA %d POSTI\n", g.posizione_in_coda, g.num_persone);  

    p.posti_liberi += g.num_persone;
    p.auto_libere++;
    stampaParco(p);
    printf("\n");

    for (int i=0; i<NUMERO_GRUPPI; i++) {
        if(p.gruppi_sospesi[i] != 0)
            sem_post(&p.sem_condizione);
    }

    pthread_mutex_unlock(&(p.sem_mutex));
}

void* visita(void* arg) {
    gruppo* gruppo_in_visita = (gruppo*)arg;

    entrata(*gruppo_in_visita);
    
    int durata_visita = 1 + rand() % (MAX_DURATA_VISITA);
    printf("La visita del gruppo %d durerà: %d ore\n", gruppo_in_visita->posizione_in_coda, durata_visita);
    printf("\n");
    sleep(durata_visita);

    uscita(*gruppo_in_visita);

    pthread_exit(NULL);
}

int main() { 
    pthread_t threads[NUMERO_GRUPPI];
    int esito_join;

    gruppo* gruppi;

    srand(time(0)); 

    printf("-------- STATO INIZIALE --------\n\n");
    inizializzaParco(&p);
    stampaParco(p);
    inizializzaGruppi(&gruppi);
    stampaGruppi(gruppi, NUMERO_GRUPPI);

    printf("\n-------- INIZIO VISITE --------\n\n");
    for(int i=0; i<NUMERO_GRUPPI; i++) {
        if (pthread_create(&threads[i], NULL, visita, (void *)(gruppi+i)) < 0) {
            fprintf (stderr, "create error for thread 1\n");
            exit(1);
        }
    }
    
    for(int i=0; i<NUMERO_GRUPPI; i++) {
        esito_join = pthread_join(threads[i], NULL);

        if(esito_join)
            printf("ERRORE join thread %ld codice %d\n", threads[i], esito_join);
    }

    free(gruppi);
}