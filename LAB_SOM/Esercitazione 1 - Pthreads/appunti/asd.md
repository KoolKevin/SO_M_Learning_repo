pthread è uno standard posix

Linuxthreads è l’implementazione di pthread nel sistema operativo GNU/linux.

Caratteristiche pthread:
- l’esecuzione di un programma determina la creazione di un __thread iniziale che esegue il codice del main__.
- Il thread iniziale può creare altri thread: si crea una gerarchia di thread che
condividono lo stesso spazio di indirizzi.
- I thread vengono creati all’interno di un processo per eseguire una funzione.

LinuxThreads:
- Il thread e` realizzato a livello kernel (e` l’unita` di schedulazione)
    - la principale differenza tra kernel UNIX e kernel Linux è che quest'ultimo supporta i thread a livello di kernel, per questo motivo sono implementati li.
- Gestione dei segnali non conforme a POSIX:
    - Non c’e` la possibilita` di inviare un segnale a un task.
    - SIGUSR1 e SIGUSR2 vengono usati per l’implementazione dei threads e quindi non sono piu` disponibili.
    - tuttavia la libreria stessa fornisce altri strumenti di sincronizzazione

Un thread (che esegue start_routine) puo` terminare in due modi:
- terminando la funzione start_routine
- chiamando pthread_exit()
__NB__: exit(), chiamato da qualsiasi thread, termina il processo, quindi tutti i thread! 

Normalmente, per thread joinable, e` necessario che il thread “padre” esegua la __pthread_join__ per ogni thread figlio che termina la sua esecuzione, altrimenti __rimangono allocate le aree di memoria__ ad esso assegnate.

### MODELLO MASTER/WORKER
thread master crea i thread e assegna loro il compito da eseguire e i dati
    -> control plane
thread worker, eseguono il lavoro con cui vengono istruiti                  
    -> data plane

### SEMAFORI DI MUTUA ESCLUSIONE
si può inizializzare un semaforo anche con una macro

### SEMAFORI GENERALI
non basta pthread.h per semafori generali ma bisogna aggiungere un pezzo ulteriore dello standard posix

Memoria condivisa: uso dei semafori (POSIX.1003.1b)

    Il tipo di dato associato al semaforo è sem_t.

- Semafori: libreria <semaphore.h>
    - sem_init: inizializzazione di un semaforo
    - sem_wait: implementazione di P
    - sem_post: implementazione di V

__NB__: semafori devono essere condivisi, di conseguenza prima di lanciare i thread il semaforo va inizializzato.
   
