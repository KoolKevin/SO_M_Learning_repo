MPI (Message passing interface) è uno standard di fatto, che stabilisce un protocollo per la comunicazione tra processi in sistemi paralleli.

Nasce per sistemi con architetture a **memoria distribuita**, che quindi utilizzano un **modello a scambio di messaggi**. Tuttavia è una libreria flessibile utilizzabile anche per sistemi a memoria comune.

**MPI adotta il modello SPMD**:
- ogni processo esegue lo stesso programma su un nodo diverso
- per differenziarne il comportamento si usa il **conditional branching**.

**Gestione di processi statica e implicita**: il grado di parallelismo viene definito a tempo di caricamento
- *mpirun –n 2*; vengono lanciate 2 istanze del programma su due nodi distinti




### MPI_Init e MPI_Finalize 
main() {
     ... // parte sequenziale

    MPI_Init(...);
    <codice con chiamate alla libreria MPI> // parte parallela
    MPI_Finalize();
    
    ... // parte sequenziale
}

- MPI_Init e MPI_Finalize delimitano la parte del programma che verrà eseguita su più nodi in parallelo.
- Fuori dal blocco MPI_Init/MPI_Finalize non possono essere chiamate funzioni MPI.


### Communicator
Quando lancio un programma con *mpirun* su n nodi, viene creato implicitamente un communicator che contiene questi n processi.

Il communicator è un’**astrazione che definisce un dominio di comunicazione**, ovvero un insieme di processi che possono comunicare tra loro 
- **NB**: due processi possono scambiarsi messaggi se e solo se appartengono allo stesso communicator.

...

### Primitive di comunicazione
MPI offre varie primitive di comunicazione che distinguiamo in:
- singole/collettive
- sincrone/asincrone

...

**MPI_Send** può essere sia sincrona che asincrona, **dipendentemente dall’implementazione** (bruh).

L'unica garanzia è che la primitiva è **buffer-safe**! Quando chiami MPI_Send(buffer, ...):

    Buffer del sender: Il buffer contiene i dati che vuoi inviare.
    Comunicazione: Il sistema MPI deve garantire che questi dati siano accessibili al destinatario.
    Sicurezza del buffer:
        MPI può bloccare il processo chiamante fino a quando:
            Il destinatario ha ricevuto completamente i dati (comunicazione sincrona).
            I dati sono stati copiati in un buffer interno di MPI per l'invio (comunicazione asincrona).

In entrambi i casi, una volta che MPI_Send ritorna, puoi essere certo che il contenuto del tuo buffer non sarà più necessario per il sistema MPI, e puoi:

    Sovrascrivere i dati nel buffer.
    Rilasciare il buffer (se allocato dinamicamente).

... varinati della send ...

... receive ...

### Esempio integrale con metodo dei trapezi
...

**Comunicazione asimmetrica** slave->master:
- il nodo master raccoglie i risultati parziali degli slave e li somma per ottenere il risultato finale  
- La soluzione è centralizzata: al crescere del numero dei nodi, il master potrebbe rappresentare un collo di bottiglia, in quanto **deve eseguire tante _MPI_Recv_ quanti sono i nodi**.

Per mitigare il problema, **si potrebbe distribuire il carico di comunicazione tra più nodi**, utilizzando, invece che comunicazioni punto-punto tra ogni slave ed il master, degli **schemi di comunicazione gerarchici** che coinvolgano tutti i nodi.
- **Ad esempio**: schema di comunicazione ad albero
    - ogni nodo (non foglia) recupera il risultato parziale dei suoi due figli, fa una somma parziale e comunica il risultato al padre; così fino alla radice dell'albero
    - In questo modo il master viene alleggerito e il costo della comunicazione è distribuito sui vari nodi


### Distribuzione del carico tra i nodi | primitive di comunicazione collettive
La soluzione proposta sopra è valida ma aggiungo carico al programmatore. MPI offre già delle soluzioni prefabbricate al problema!

Le primitive di comunicazione collettive. Sono funzioni per la **comunicazione asimmetrica** implementate in modo tale da **distribuire il carico di comunicazione e di calcolo tra i nodo coinvolti**.

Ce ne sono tante, in seguito le principali:
- MPI_Reduce()      -> molti-a-uno
    - tutti i processi eseguono la MPI_Reduce! 
        - si distinguono i nodi che inviano semplicemente il loro dato (include anche il collettore) ...
        - ... dal collettore che si occupa anche di aggregare i dati nel risultato finale, specificando il rank del collettore
        - il **collettore** riceve e aggrega i dati in total_int
        - tutti gli altri processi inviano il proprio risultato (local_int).
    - Semantica di **ricezione bloccante**
- MPI_Bcast()       -> uno-a-molti
    - Simile a sopra, tutti i processi eseguono MPI_Bcast e si specifica il rango del processo emitter per capire quale variabile inviare
        - Il processo «emitter» invia il dato
        - tutti gli altri lo ricevono 
    - recv bloccante
    - **NB**: questa operazione è spesso necessaria in quanto è tipico che un singolo processo recuperi i dati iniziali del problema (e.g. stdin, oppure da un file sul suo nodo, ecc...). MPI_Bcast è quindi necessaria quando:
        - I dati iniziali non sono determinati staticamente nel codice, ma dipendono, ad esempio, da un input recuperato dal processo con rank 0
        - Solo il processo con rank 0 ha accesso a questi dati ed è quindi responsabile di distribuirli agli altri processi.

L’esigenza di distribuire dati e raccogliere risultati verso/da processi paralleli è comune per moltissime applicazioni HPC. Reduce e broadcast però operano solo con dati scalari, **per distribuire/raccogliere vettori**:
- MPI_Scatter()     -> uno-a-molti
    - Semantica ricezione bloccante
    - il numero di *send* dipende dal size del communicator
- MPI_Gather()      -> molti-a-uno
    - Semantica ricezione bloccante
    - il numero di *receive* dipende dal size del communicator
- MPI_AllGather()   -> molti-a-molti
    - ogni processo è collettore (oltre che sender)


**NOTA**: in generale tutte le chiamante bloccanti hanno anche una **versione asincrona non bloccante**. Per testare la disponibilità del buffer dopo una chiamata asincrona si usa *MPI_Test()*, oppure si aspetta con *MPI_Wait()* (sospensiva).

### Sincronizzazione con barriera
- int MPI_Barrier(MPI_Comm comm)

Blocca ogni processo nel communicator comm fino a quando tutti non avranno chiamato la MPI_Barrier


### Misurazione del tempo in MPI
- double MPI_Wtime(void);

È una funzione che restituisce il valore corrente del tempo locale (Walltime).

Per la misurazione dei tempi di esecuzione di programmi, si calcola la differenza tra due valori prodotti da chiamate successive a MPI_Wtime. Tale differenza esprime (**in secondi**) il tempo trascorso tra le 2 chiamate.

**NB**: Nella misurazione di tempi globali, ovvero tempi che ci dicono dopo quanto **tutti i processi** hanno terminato un'operazione, è **necessario sincronizzare con una barriera** prima e dopo le misurazioni

double start, end;

MPI_Init(&argc, &argv);
...
**MPI_Barrier(MPI_COMM_WORLD);**
**start = MPI_Wtime();**


/* ... calcola ... */


**MPI_Barrier(MPI_COMM_WORLD);**
**end = MPI_Wtime();**

if (rank == 0)
    printf("Tempo di esecuzione = %f\n", end-start);
...
MPI_Finalize();

In questa maniera misuriamo quanto tempo il programma parallelo (inteso come insieme di processi) ha speso in una determinata sezione del codice.
- processi diversi possono terminare ad istanti diversi
- sincronizzando possiamo tenere conto dei tempi che i processi veloci spenderebbero ad aspettare i processi lenti una volta giunti a dei punti di comunicazione/interazione.

**Es con reduce finale**:

L'ultima reduce è buona prassi ma in teoria, anche senza tutti i tempi dovrebbero essere simili in quanto abbiamo sincronizzato.

**MPI_Barrier(MPI_COMM_WORLD);**
**inizio=MPI_Wtime();**

h = (b - a)/n;
local_n = n/comm_sz;
local_a = a + my_rank * local_n * h;
local_b = local_a + local_n * h;
local_int = Trap(local_a, local_b, local_n, h);

// questa chiamata è bloccante, sincronizza già di suo
**MPI_Reduce(&local_int, &total_int,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);** 
**fine=MPI_Wtime();**

local_elaps= fine-inizio;
printf("tempo impiegato da proc %d: %f secondi\n", my_rank, local_elaps);

**MPI_Reduce(&local_elaps, &global_elaps,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);**

if (my_rank == 0) {
    printf("With n = %d trapezoids, our estimate\n", n);
    printf("of the integral from %f to %f = %.15e\n", a, b, total_int);
    printf("tempo impiegato: %f secondi\n", global_elaps);
}