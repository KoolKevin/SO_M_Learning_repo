#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_POSTI 3    
#define MAX_BICI 1  
#define MAX_MONOPATTINI 1      

#define NUMERO_VISITATORI 10
#define MAX_DURATA_VISITA 3

typedef struct{
    int posti_liberi;
    int bici_libere;
    int monopattini_liberi;

    // semaforo condizione su cui sospendere i visitatori per cui non c'è posto
    // oppure per cui non è disponibili il mezzo da loro desiderato
    sem_t S;    
    int sospesi;
    pthread_mutex_t m;
} parco;

enum VeicoloRichiesto {
    BICI,
    MONOPATTINO
};

// variabili globali
parco p;

void inizializzaParco(parco* p) {
    p->posti_liberi = MAX_POSTI;
    p->bici_libere = MAX_BICI;
    p->monopattini_liberi = MAX_MONOPATTINI;
    
    sem_init(&(p->S), 0, 0);
    p->sospesi = 0;
    pthread_mutex_init(&(p->m), NULL); 
}

void stampaParco(parco p) {
    printf("PARCO:\n");
    printf("\tposti: %d\n", p.posti_liberi);
    printf("\tbici: %d\n", p.bici_libere);
    printf("\tmonopattini: %d\n", p.monopattini_liberi);
}

void entrata(int veicolo_richiesto) {  
    switch(veicolo_richiesto) {
        case BICI:
            pthread_mutex_lock(&(p.m));

            printf("Il visitatore %ld richiede una bici!\n", pthread_self());

            while (p.posti_liberi == 0 || p.bici_libere == 0) { 
                p.sospesi++;
                printf("Il visitatore %ld deve aspettare prima di entrare!\n", pthread_self());
                printf("\n");
                pthread_mutex_unlock(&(p.m));

                sem_wait(&p.S);

                pthread_mutex_lock(&(p.m));
                p.sospesi--;
                printf("Il visitatore %ld può ritentare a prendere una bici!\n", pthread_self());
            }

            p.posti_liberi--;
            p.bici_libere--;
            stampaParco(p);

            pthread_mutex_unlock(&(p.m));
            break;

        case MONOPATTINO:   //ripeto la stessa cosa brutalmente. Bruttissimo ma non ho voglia
            pthread_mutex_lock(&(p.m));

            printf("Il visitatore %ld richiede un monopattino!\n", pthread_self());

            while (p.posti_liberi == 0 || p.monopattini_liberi == 0) { 
                p.sospesi++;
                printf("Il visitatore %ld deve aspettare prima di entrare!\n", pthread_self());
                printf("\n");
                pthread_mutex_unlock(&(p.m));

                sem_wait(&p.S);

                pthread_mutex_lock(&(p.m));
                p.sospesi--;
                printf("Il visitatore %ld può ritentare a prendere un monopattino!\n", pthread_self());
            }

            p.posti_liberi--;
            p.monopattini_liberi--;
            stampaParco(p);
            
            pthread_mutex_unlock(&(p.m));
            break;
    }
    
}

void uscita(int veicolo_richiesto) {
    switch(veicolo_richiesto) {
        case BICI:
            pthread_mutex_lock(&(p.m));
            printf("Il visitatore %ld esce e libera una bici!\n", pthread_self());

            p.posti_liberi++;
            p.bici_libere++;
            stampaParco(p);
            printf("\n");

            for (int i=0; i<p.sospesi; i++) {
                sem_post(&p.S);
            }

            pthread_mutex_unlock(&(p.m));
            break;
        case MONOPATTINO:
            pthread_mutex_lock(&(p.m));
            printf("Il visitatore %ld esce e libera un monopattino!\n", pthread_self());

            p.posti_liberi++;
            p.monopattini_liberi++;
            stampaParco(p);
            printf("\n");

            for (int i=0; i<p.sospesi; i++) {
                sem_post(&p.S);
            }

            pthread_mutex_unlock(&(p.m));
            break;
    }
}

void* visita(void* arg) {
    int veicolo = (int)arg;

    entrata(veicolo);
    
    int durata_visita = 1 + rand() % (MAX_DURATA_VISITA-1);
    printf("La visita di %ld durerà: %d ore\n", pthread_self(), durata_visita);
    printf("\n");
    sleep(durata_visita);

    uscita(veicolo);

    pthread_exit(NULL);
}


int main() { 
    pthread_t threads[NUMERO_VISITATORI];
    int rj;

    srand(time(0)); 
    inizializzaParco(&p);
    stampaParco(p);
    printf("\n");
    

    for(int i=0; i<NUMERO_VISITATORI; i++) {
        // i visitatori con indice pari vogliono le bici gli altri vogliono il monopattino
        if (pthread_create(&threads[i], NULL, visita, (void *)(i%2)) < 0) {
            fprintf (stderr, "create error for thread 1\n");
            exit(1);
        }
    }
    
    for(int i=0; i<NUMERO_VISITATORI; i++) {
        rj = pthread_join(threads[i], NULL);

        if (rj)
            printf("ERRORE join thread %ld codice %d\n", threads[i], rj);
    }
}