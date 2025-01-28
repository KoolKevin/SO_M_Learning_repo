
...

Da un certo punto di vista, il SO di un sistama HPC è più semplice rispetto ad uno di un computer tradizionale: 
- scheduling più semplice, un processo per ogni nodo. 
- non c'è più bisogno di concorrenza e multiprogrammazione dato che ho a disposizione un elevato numero di nodi, sufficienti per tutti i processi che voglio lanciare.

## Von Neumann Bottleneck e soluzioni

...

### Caching
...

### Parallelismo low-level | ILP

**Premessa**: eseguire una singola istruzione in realtà richiede una serie di sottoperazioni ancora più elementari eseguibili parallelamente in pipeline.   
- pipelining: fasi diverse di **istruzioni diverse** possono essere eseguite in parallelo.

Efficace fino a quando non ci sono dipendenze tra le istruzioni del programma da eseguire. 

### Parallelismo low-level | Hardware multithreading
Detto anche Thread Level Parallelism (TLP). In sostanza si tengono allocati più thread ad un singolo core/processore contemporaneamente.

Si divide in livelli; consideriamo il caso di 2 thread (2 livelli)

Hardware multithreading (TLP): permette a più thread (ad esempio, 2 thread) di condividere la stessa CPU (core), utilizzando una tecnica di **sovrapposizione**.

**Occhio**: non c'è uno software schedulere, siamo a basso livello.

Qua abbiamo due batterie di registri (2 livelli) nella singola CPU, ognuna associata ad uno dei due thread assegnati alla CPU in quel momento.

L'hardware considererà queste due batterie una per volta, di fatto eseguendo un thread alla volta con un cambio di contesto hardware e più leggero.

**OSS**: questo tipo di cambio di contesto è molto più leggero rispetto a quello classico, siccome non bisogna andare a recuperare i registri del processo a cui si sta switchando. Sono già in una delle batterie di registri.

con un multithreading a grana grossa: le istruzioni presenti nella pipeline, a fronte di un cache miss, non sono più valide (non è predicibile quando arriva un cache miss).


## Architetture per HPC
ILP e Hardware multithreading hanno permesso un miglioramento delle prestazioni dei processori, tuttavia tali **meccanismi sono trasparenti per i programmatori**. 
- modello Von Neumann Esteso

Nei sistemi HPC, invece, **il parallelismo disponibile per l’esecuzione dei programmi è visibile al programmatore**, che deve progettare il software in modo da sfruttare al meglio tutte le risorse computazionali a disposizione. 
- architetture non Von Neumann

Distinguiamo tra sistemi:
- SIMD
    - sincronicità nel funzionamento: Ad ogni istante le unità di calcolo presenti a livello HW eseguono la stessa istruzione su dati diversi.
    - Più processori ed 1 sola control unit.
- MIMD
    - Asincronicità delle attività nei diversi nodi: Ogni CPU esegue una sequenza di istruzioni diversa dagli altri.
        - Necessità di interazione tra nodi per convergere ad un risultato finale
    - più processori, ognuno con la sua propria control unit.

Concentriamoci sui sistemi MIMD e categorizziamoli in:
- **shared memory systems**
    - nodi multiprocessor e/o multicore
    - UMA, NUMA
- **distributed memory systems**
    - nodi connessi in rete
    - cluster, grid, supercomputers
- **modelli ibridi**
    - cluster di nodi a memoria distribuita collegati da reti ad alte prestazioni
    - ogni nodo è un multiprocessore: insieme di processori multicore.
    - La maggior parte dei sistemi HPC di oggi adotta questo modello



## Sviluppo di software parallelo 
Un programma parallelo in esecuzione è un **insieme di processi**, ognuno **in esecuzione su un nodo fisico distinto**, che all’occorrenza possono **interagire**.

Nei sistemi HPC chi lancia l’esecuzione di un programma parallelo può contare sulla disponibilità esclusiva di un insieme di nodi di elaborazione
da usare per l’esecuzione. In sistemi di questa categoria **il parallelismo si ottiene distribuendo task diversi a processi diversi (task parallelism)**
- ogni processo è assegnato a una CPU a sua completa disposizione.

**Come assegnare i task ai nodi?** 
Nello sviluppo di SW parallelo è molto utilizzato il **paradigma SPMD** («Single Program Multiple Data»):
- **ogni core esegue lo stesso programma**
- per differenziare il codice eseguito dai diversi nodi, si sfrutta il **branching condizionale** 
- Il codice viene personalizzato in base all’indice (rank) del nodo sul quale il singolo processo esegue


### Modelli di interazione
A seconda delle **caratteristiche architetturali del sistema utilizzato**, nello sviluppo di software parallelo è possibile fare riferimento a **2 modelli di interazione**:

- Se i nodi dell’architettura non condividono memoria (es. Cluster HPC) lo sviluppo dei programmi paralleli si fonda sul modello a **scambio di messaggi**. 
    - In questo ambito lo standard è rappresentato dalle librerie **MPI**.

- Se, invece, è prevista la condivisione di memoria tra tutti i nodi (es. sistemi multicore/multiprocessor) il modello di interazione tra processi è a **memoria comune**.
    - In questo caso, è possibile utilizzare **OpenMP**.
