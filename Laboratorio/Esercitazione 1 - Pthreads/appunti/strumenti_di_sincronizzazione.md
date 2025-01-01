### SEMAFORI DI MUTUA ESCLUSIONE
Operazioni  fondamentali:
- inizializzazione:             pthread_mutex_init
- Locking (v. operazione p):    pthread_mutex_lock
- Unlocking(v. operazione v):   pthread_mutex_unlock

...
si può inizializzare un semaforo anche con una macro: pthread_mutex_t mux= PTHREAD_MUTEX_INIZIALIZER 

### SEMAFORI GENERALI
<pthread.h> non contiene semafori generali, bisogna aggiungere un pezzo ulteriore dello standard posix


- Il tipo di dato associato al semaforo è sem_t.
- Semafori: libreria <semaphore.h>
    - sem_init: inizializzazione di un semaforo
    - sem_wait: implementazione di P
    - sem_post: implementazione di V

__NB__: semafori devono essere condivisi, di conseguenza prima di lanciare i thread il semaforo va inizializzato.
   