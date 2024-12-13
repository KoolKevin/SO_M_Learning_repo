MPI (Message passing interface) è uno standard di fatto, che stabilisce un protocollo per la comunicazione tra processi in sistemi paralleli.

Nasce per sistemi con architetture a memoria distribuita, e quindi che utilizzano un modello a scambio di messaggi. Tuttavia è una libreria flessibile, nel senso che funziona anche per modello a memoria comune.

...

**Gestione di processi statica e implicita**: il grado di parallelismo viene definito a tempo di caricamento

...

MPI adotta il modello SPMD: **ogni processo esegue lo stesso programma su un nodo diverso**; per differenziarne il comportamento si usa il **conditional branching**.

### Communicator
Quando lancio un programma con *mpirun* su n nodi, viene creato implicitamente un communicator che contiene questi n processi.

Il communicator è un’astrazione che definisce un dominio di comunicazione, ovvero un insieme di processi che possono comunicare tra loro 
- due processi possono scambiarsi messaggi se e solo se appartengono allo stesso communicator.

...

### Primitive di comunicazione
MPI offre varie primitive di comunicazione che distinguiamo in:
- singole/collettive
- sincrone/asincrone

...

MPI_Send può essere sia sincrona che asincrona, **dipendentemente dall’implementazione** (bruh).

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

... esempio ...

### Distribuzione del carico tra i nodi | primitive di comunicazione collettive
Le primitive di comunicazione collettive. Sono funzioni per la **comunicazione asimmetrica** implementate in modo tale da distribuire il carico di comunicazione e di calcolo tra i nodo coinvolti.

ce ne sono tante, in seguito le principali:
- MPI_Reduce()      -> molti-a-uno
- MPI_Bcast()       -> uno-a-molti
- MPI_Scatter()     -> uno-a-molti
- MPI_Gather()      -> molti-a-uno
- MPI_AllGather()   -> molti-a-molti

...

**Bcast**
Un'operazione come MPI_Bcast sarebbe necessaria se:
- I dati iniziali non fossero determinati staticamente nel codice, ma dipendessero, ad esempio, da un input fornito dal processo con rank 0.
- Solo il processo con rank 0 avesse accesso a questi dati e fosse responsabile di distribuirli agli altri processi.