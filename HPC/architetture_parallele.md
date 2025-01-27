
...

Da un certo punto di vista, il SO di un sistama HPC è più semplice rispetto ad uno di un computer tradizionale: 
- scheduling più semplice, un processo per ogni nodo. 
- non c'è più bisogno di concorrenza e multiprogrammazione dato che ho a disposizione un elevato numero di nodi, sufficienti per tutti i processi che voglio lanciare.

...

### Parallelismo low-level | ILP

**Premessa**: eseguire una singola istruzione in realtà richiede una serie di sottoperazioni ancora più elementari eseguibili parallelamente in pipeline.   
- pipelining: fasi diverse di **istruzioni diverse** possono essere eseguite in parallelo.

Efficace fino a quando non ci sono dipendenze tra le istruzioni del programma da eseguire. 

### Parallelismo low-level | Hardware multithreading
Detto anche Thread Level Parallelism (TLP). In sostanza si tengono allocati più thread ad un singolo core/processore contemporaneamente.

Si divide in livelli; consideriamo il caso di 2 thread (2 livelli)

Hardware multithreading (TLP): permette a più thread (ad esempio, 2 thread) di condividere la stessa CPU (core), utilizzando una tecnica di **sovrapposizione**.

**Occhio**: non c'è lo scheduler qua, siamo a basso livello.

Qua abbiamo due batterie di registri (2 livelli) nella singola CPU, ognuna associata ad uno dei due thread assegnati alla CPU in quel momento.

L'hardware considererà queste due batterie una per volta, di fatto eseguendo un thread alla volta con un cambio di contesto hardware e più leggero.

**OSS**: questo tipo di cambio di contesto è molto più leggero rispetto a quello classico, siccome non bisogna andare a recuperare i registri del processo a cui si sta switchando. Sono già in una delle batterie di registri.

...

con un multithreading a grana grossa: le istruzioni presenti nella pipeline, a fronte di un cache miss, non sono più valide (non è predicibile quando arriva un cache miss).

...

### HPC Systems 

...

usano sia memoria condivisa che distribuita (modello ibrido). 

...