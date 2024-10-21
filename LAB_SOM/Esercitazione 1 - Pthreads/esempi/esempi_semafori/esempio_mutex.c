#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX 10

pthread_mutex_t M; /* def.mutex condiviso tra threads */
int DATA=0; /* variabile condivisa */

void *thread1_process (void * arg) {
    int k=1;
    static int accessi1 = 0; /* num. di accessi del thread 1 alla sez crit. */

    while(k) {
        pthread_mutex_lock(&M); /* prologo */

        accessi1++;
        DATA++;
        k=(DATA >= MAX ? 0 : 1);
        printf("accessi di T1: %d\n", accessi1);

        pthread_mutex_unlock(&M); /* epilogo */
    }

    pthread_exit(0);
}

void *thread2_process (void * arg) { 
    int k=1;
    static int accessi2 = 0; /* num. di accessi del thread 2 alla sez crit. */

    while(k) {
        pthread_mutex_lock(&M); /* prologo */

        accessi2++;
        DATA++;
        k = (DATA >= MAX ? 0 : 1);
        printf("accessi di T2: %d\n", accessi2);

        pthread_mutex_unlock(&M); /* epilogo */
    }

    pthread_exit(0);
}

int main() { 
    pthread_t th1, th2;
    /* il mutex e` inizialmente libero: */
    pthread_mutex_init(&M, NULL);

    if (pthread_create(&th1, NULL, thread1_process, NULL) < 0) {
        fprintf(stderr, "create error for thread 1\n");
        exit(1);
    }

    if (pthread_create(&th2, NULL,thread2_process,NULL) < 0) {
        fprintf(stderr, "create error for thread 2\n");
        exit(1);
    }

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
}