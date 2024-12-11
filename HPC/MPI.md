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

### Funzioni di comunicazione
MPI offre varie primitive di comunicazione che distinguiamo in:
- singole/collettive
- sincrone/asincrone

...

MPI_Send può essere sia sincrona che asincrona, **dipendentemente dall’implementazione** (bruh).

capisci meglio che cosa significa buffer-safe