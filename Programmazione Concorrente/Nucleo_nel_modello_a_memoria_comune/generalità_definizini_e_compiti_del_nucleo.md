### Realizzazione dei semafori
In sistemi operativi multiprogrammati, il semaforo viene realizzato dal kernel, che, sfruttando i meccanismi di gestione dei processi (sospensione e riattivazione) __elimina la possibilità di attesa attiva__.

Descrittore di un semaforo:

    typedef struct{
        int contatore;
        coda queue;
    } semaforo;

E` possibile definire la P e la V nel modo seguente, garantendo comunque la validità delle proprietà del semaforo:
- Una p su un semaforo con contatore a 0, sospende il processo nella coda queue, altrimenti contatore viene decrementato.
- Una v su un semaforo la cui coda queue non è vuota, estrae un processo dalla coda, altrimenti incrementa il contatore

NB: L’implementazione di p e v è parte del nucleo della macchina concorrente e __dipende dal tipo di architettura hardware__ (monoprocessore,
multiprocessore, ecc.) __e da come il nucleo rappresenta e gestisce i processi concorrenti__.

### Il nucleo di un sistema multiprogrammato (modello a memoria comune)

    Def: Si chiama nucleo (kernel) il modulo (o insieme di funzioni) realizzato in software, hardware o firmware che supporta il concetto di processo e realizza gli strumenti necessari per la gestione dei processi.

OSS: In un sistema multiprogrammato (o «a processi») vengono offerte tante unità di elaborazione astratte (macchine virtuali) quanti sono i processi.

    NB: Il nucleo è il solo modulo che è conscio dell'esistenza delle interruzioni.

- Ogni processo che richiede un’operazione ad un dispositivo utilizza un’opportuna primitiva del nucleo che provvede a sospenderlo in attesa del completamento dell'azione richiesta.
- Quando l'azione è completata, un segnale di interruzione inviato dal dispositivo alla CPU viene catturato e gestito dal nucleo, che provvede a risvegliare il processo sospeso.
- La gestione delle interruzioni è quindi invisibile ai processi ed ha come unico effetto rilevabile di rallentare la loro esecuzione sulle rispettive macchine virtuali.
    - Quando viene attivata una interruzione essa viene gestita dal kernel e non viene mai propagata in user space.

Obiettivo di questo capitolo: realizzazione dei processi e della loro sincronizzazione

### Stati di un processo
Distinguiamo tra processo __attivo__:
- Un processo con assegnata o revocata la CPU ma con niente che blocca la sua esecuzione

e processo __bloccato__:
- Un processo, senza CPU, in attesa di un evento per proseguire.

Le transizioni tra i due stati sono implementate dai meccanismi di sincronizzazione realizzati dal nucleo.
Es: semaforo:
- p per sospensione
- v per risveglio.

### Contesto di un processo
- __Contesto di un processo__: è l'insieme delle informazioni contenute nei registri del processore, quando esso opera sotto il controllo del processo.
- __Salvataggio di contesto__: Quando un processo perde il controllo del processore, il contenuto dei registri del processore (contesto) viene salvato in una struttura dati associata al processo, chiamata descrittore (PCB).
- __Ripristino del contesto__: Quando un processo viene schedulato, i valori salvati nel suo descrittore vengono caricati nei registri del processore.

### Compiti fondamentali del nucleo:
Il compito fondamentale del nucleo di un sistema a processi è gestire le transizioni di stato dei processi. In particolare:
- Gestire il __salvataggio ed il ripristino dei contesti__ dei processi
    - per ogni avvicendamento tra 2 processi P1 e P2 nell’uso della CPU, va salvato il contesto del processo deschedulato P1 (registri -> descrittore) e ripristinato il contesto del processo schedulato P2 (descrittore -> registri).
- Scegliere a quale tra i processi pronti assegnare l'unità di elaborazione (__scheduling della CPU__):
    - Quando un processo abbandona il controllo dell'unità di elaborazione, il nucleo deve scegliere tra tutti i processi pronti quello da mettere in esecuzione. La scelta è dettata dalla politica di scheduling adottata (es: FIFO, SJF, Priorità ecc.).
- __Gestire le interruzioni__ dei dispositivi esterni
    - traducendole eventualmente in attivazione di processi da bloccato a pronto.
- Realizzare i meccanismi di sincronizzazione dei processi
    - gestendo il passaggio dei processi dallo stato di esecuzione allo stato bloccato e da bloccato a pronto (es. primitive p e v).

### Strutture dati del nucleo

#### Descrittore del processo
contiene un sacco di informazioni, tra le quali:
- contesto: fotografia dello stato del processo (registri).
- __riferimenti a varie code__: a seconda dello stato del processo, il suo descrittore può essere inserito in apposite code (es. coda dei processi bloccati su un dispositivo, ready queue, ecc). Ogni descrittore contiene, pertanto, il riferimento all’elemento successivo nella stessa coda.

#### Coda dei processi pronti
Esistono una o più (caso di scheduling con priorità) code di processi pronti. Quando un processo e` riattivato per effetto di una v, viene inserito al fondo della coda corrispondente alla sua priorità.

#### Coda dei descrittori liberi
Coda nella quale sono concatenati i descrittori disponibili per la creazione di nuovi processi e nella quale sono re-inseriti i descrittori dei processi terminati.

#### Processo in esecuzione
Il nucleo necessita di conoscere quale processo è in esecuzione. Questa informazione, rappresentata dall'indice del descrittore del processo, viene contenuta in una particolare variabile del nucleo (spesso, un registro del processore)

### Funzioni del nucleo
Le funzioni del nucleo realizzano le operazioni di transizione di stato per i singoli processi. __Ogni transizione prevede il prelievo da una coda del descrittore del processo coinvolto, ed il suo inserimento in un'altra coda__. Si utilizzano a questo scopo due procedure: Inserimento e Prelievo di un descrittore da una coda. Se la coda è vuota: valore -1 (NIL).
