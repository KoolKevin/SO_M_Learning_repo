/* la FASE2 nel thread 1 va eseguita dopo la FASE1 nel thread 2*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

sem_t my_sem;
int V=0;

void *thread1_process (void * arg) { 
    printf("Thread 1: partito!...\n");

    /* inizio Fase 2: */
    sem_wait(&my_sem); //operazione "p"
    printf("FASE2: Thread 1:  V=%d\n", V);
    
    pthread_exit(0);
}

void *thread2_process (void * arg) {
    int i;
    V=99;

    printf ("Thread 2: partito!...\n");
    
    /* inizio fase 1: */  
    printf ("FASE1: Thread 2:  V=%d\n", V);
    sleep (1);

    /* ... termine Fase 1: sblocco il thread 1*/
    sem_post (&my_sem); //operazione "v"

    pthread_exit (0);
}

int main () {
    pthread_t th1, th2;
    void *ret;

    sem_init (&my_sem, 0, 0); /* semaforo a 0 */
    
    if (pthread_create (&th1, NULL, thread1_process, NULL) < 0) { 
        fprintf (stderr, "pthread_create error for thread 1\n");
        exit (1);
    }

    if (pthread_create(&th2,NULL, thread2_process, NULL) < 0) {
        fprintf (stderr, "pthread_create error for thread \n");
        exit (1);
    }

    pthread_join (th1, &ret);
    pthread_join (th2, &ret);
}