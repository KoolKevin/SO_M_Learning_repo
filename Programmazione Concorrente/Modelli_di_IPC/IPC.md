### MODELLI DI INTERAZIONE TRA PROCESSI (CONCORRENTI)
    • Modello a memoria comune (ambiente globale, shared memory)
    • Modello a scambio di messaggi (ambiente locale, distributed memory)

due modelli di interazione utilizzati per gestire la comunicazione e la sincronizzazione tra processi o thread che eseguono operazioni simultaneamente.

**NB**: Equivalenza Computazionale

Entrambi i modelli sono equivalenti in termini di potenza espressiva:

    Un sistema basato su memoria condivisa può essere simulato con lo scambio di messaggi, e viceversa.
        Ad esempio, un'area di memoria condivisa può essere rappresentata da un processo che funge da "gestore" e accetta richieste di lettura e scrittura via messaggi.
        Allo stesso modo, la comunicazione tramite messaggi può essere implementata usando strutture dati condivise come code o buffer circolari.

Conclusione

Anche se entrambi i modelli sono teoricamente equivalenti e possono risolvere gli stessi problemi di concorrenza, la scelta pratica dipende dal contesto applicativo:

    Memoria comune: Preferibile per sistemi con thread nello stesso indirizzo di memoria (es. applicazioni multi-threading).
    Scambio di messaggi: Più adatto per sistemi distribuiti o processi indipendenti.

**ULTERIORE_OSS**: Il modello a scambio di messaggi non necessità che i due processi interagenti risiedano sulla stessa macchina in quanto hanno spazi di indirizzamento separati, e sempre per questo motivo si pongono in generale meno problemi riguardanti corse critiche. 

Tuttavia dato che il canale di comunicazione risiede nello spazio di indirizzamento del kernel si ha necessità di usare delle syscall (send() e receive()) per l'effettiva comunicazione! Questo ha un overhead rispetto al modello a memoria comune in cui si ha bisogno di una syscall solamente quando si crea lo spazio di memoria comune. A quel punto i due processi vedono quello spazio come se fosse nel loro spazio di indirizzamento e quindi posso accederci senza costo!
