#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_POSTI 5    
#define MAX_DIM_GRUPPO 5
#define MAX_AUTO 2      // da 1 a 5 persone di un gruppo per auto

#define NUMERO_GRUPPI 10 
#define MAX_DURATA_VISITA 3

typedef struct{
    int posti_liberi;
    int auto_libere;

    // semafori privati su cui sospendere i gruppi di visitatori se:
    //  - posti_liberi < dimensione_gruppo
    //  - auto_libere == 0
    //  - c'è un gruppo più piccolo in attesa
    //      - sem_privati[n] === semaforo per la classe di processi con dimensione del gruppo = n+1
    //      - stessa cosa per sospesi[n]
    sem_t sem_privati[MAX_DIM_GRUPPO];
    int gruppi_sospesi[MAX_DIM_GRUPPO];  
    pthread_mutex_t sem_mutex;
} parco;

typedef struct{
    int id;
    int num_persone;
} gruppo;

// variabili globali
parco p;

void inizializzaParco(parco* p) {
    p->posti_liberi = MAX_POSTI;
    p->auto_libere = MAX_AUTO;
    
    for(int i=0; i<MAX_DIM_GRUPPO; i++) {
        sem_init(&(p->sem_privati[i]), 0, 0);
        p->gruppi_sospesi[i] = 0;
    }

    pthread_mutex_init(&(p->sem_mutex), NULL); 
}

void stampaParco(parco p) {
    printf("PARCO:\n");
    printf("\tposti liberi: %d\n", p.posti_liberi);
    printf("\tauto libere: %d\n", p.auto_libere);
}

void inizializzaGruppi(gruppo** gruppi) {
    *gruppi = (gruppo*)malloc(sizeof(gruppo)*NUMERO_GRUPPI);   

    for(int i=0; i<NUMERO_GRUPPI; i++) {
        (*gruppi)[i].id = i;
        (*gruppi)[i].num_persone = 1 + rand() % (MAX_DIM_GRUPPO);
    }
}

void stampaGruppi(gruppo* gruppi, int num_gruppi) {
     printf("GRUPPI:\n");
    for(int i=0; i<num_gruppi; i++) {
        printf("\tid: %d\n", gruppi[i].id);
        printf("\tnumero persone: %d\n", gruppi[i].num_persone);   
        printf("\n");
    }
}   

void entrata(gruppo g) {  
    pthread_mutex_lock(&(p.sem_mutex));

    printf("GRUPPO %d VUOLE ENTRARE\n", g.id);  

    while (p.posti_liberi < g.num_persone || p.auto_libere == 0) { 
        p.gruppi_sospesi[g.num_persone-1]++;
        printf("gruppo %d ha priorità %d, deve aspettare prima di entrare\n", g.id, g.num_persone-1);
        printf("\n");
        pthread_mutex_unlock(&(p.sem_mutex));

        sem_wait(&p.sem_privati[g.num_persone-1]);

        pthread_mutex_lock(&(p.sem_mutex));
        printf("gruppo %d ha priorità %d, può ritentare ad entrare\n", g.id, g.num_persone-1);
    }

    p.posti_liberi -= g.num_persone;
    p.auto_libere--;
    stampaParco(p);

    pthread_mutex_unlock(&(p.sem_mutex));
}

void uscita(gruppo g) {
    pthread_mutex_lock(&(p.sem_mutex));

    printf("GRUPPO %d ESCE E LIBERA %d POSTI\n", g.id, g.num_persone);  
    p.posti_liberi += g.num_persone;
    p.auto_libere++;
    stampaParco(p);
    printf("\n");

    //risveglio tutti in ordine di priorità finchè c'è posto 
    // TODO: aggiusta perchè qua stai sequnzializzando
    int fine = 0;

    for (int i=0; i<MAX_DIM_GRUPPO; i++) {
        while(p.gruppi_sospesi[i] > 0) {
            p.gruppi_sospesi[i]--;
            sem_post(&p.sem_privati[i]);
            fine = 1;
        }  

        if(fine)
            break; 
    }

    pthread_mutex_unlock(&(p.sem_mutex));
}

void* visita(void* arg) {
    gruppo* gruppo_in_visita = (gruppo*)arg;

    entrata(*gruppo_in_visita);
    
    int durata_visita = 1 + rand() % (MAX_DURATA_VISITA);
    printf("La visita del gruppo %d durerà: %d ore\n", gruppo_in_visita->id, durata_visita);
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