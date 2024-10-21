## PTHREAD
È una libreria che offre funzioni e tipi per la gestione dei threads, in  conformita` con lo standard POSIX. È disponibile in tutti i SO conformi allo standard 👉‍ Portabilita

L’interfaccia della libreria è la stessa in tutti i SO che la offrono, ma  l’implementazione dei thread e dei meccanismi per la loro gestione può  differire da sistema  a sistema. __LinuxThreads__ è l’implementazione di pthread nel sistema operativo GNU/linux.

Caratteristiche pthread:
- l’esecuzione di un programma determina la creazione di un __thread iniziale che esegue il codice del main__.
- Il thread iniziale può creare altri thread: si crea una gerarchia di __thread che condividono lo stesso spazio di indirizzi__.
- I thread vengono creati all’interno di un processo per eseguire __una funzione__.

LinuxThreads:
- Il thread è realizzato a livello kernel
    - è l’unità di schedulazione, ovvero, la contesa sulla CPU avviene a livello di thread e non di processo/task
    - la principale differenza tra kernel UNIX e kernel Linux è che quest'ultimo supporta i thread a livello di kernel, per questo motivo sono implementati li.
- Gestione dei segnali non conforme a POSIX:
    - Non c’è la possibilita di inviare un segnale a un task.
    - SIGUSR1 e SIGUSR2 vengono usati per l’implementazione dei threads e quindi non sono piu` disponibili.
    - Tuttavia la libreria stessa fornisce altri strumenti di sincronizzazione

... seguono vari tipi e funzioni per operare sui thread ...

Un thread (che esegue start_routine) puo` terminare in due modi:
- terminando la funzione start_routine
    - return
- chiamando __pthread_exit(void *retval)__
    - in questo caso può restituire un valore di ritorno ad altri threads
__NB__: exit(), chiamato da qualsiasi thread, termina il processo, quindi tutti i thread! 

...

Un thread puo` sospendersi in attesa della terminazione di un altro thread  con:    

    int pthread_join(pthread_t th, void **thread_return);

Dove:
- th: e` il pid del particolare thread da attendere
- thread_return: se thread_return non è NULL,  in *thread_return  viene memorizzato il valore di ritorno del thread (v. parametro pthread_exit)

Il valore restituto dalla pthread_join indica l'esito della chiamata: se diverso da zero, significa che la pthread_join e` fallita (ad es. non vi sono thread figli)

__NB__: Normalmente, per thread joinable, e` necessario che il thread “padre” esegua la __pthread_join__ per ogni thread figlio che termina la sua esecuzione, altrimenti __rimangono allocate le aree di memoria__ ad esso assegnate.

### MODELLO MASTER/WORKER
Il modello si presta alla soluzione di problemi secondo lo  schema master/worker.

- thread master crea i thread e assegna loro il compito da eseguire e i dati
    - control plane
- thread worker, eseguono il lavoro con cui vengono istruiti                  
    - data plane

### Compilazione con libreria pthread
    
    gcc -D_REENTRANT -o prog prog.c  -l pthread

L'opzione "D_REENTRANT" è importante per una esecuzione “thread-safe”.

