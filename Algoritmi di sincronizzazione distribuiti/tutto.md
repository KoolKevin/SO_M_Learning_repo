### Premessa
In questo capitolo consideriamo con "sistema distribuito" sia: sistemi composti da un singolo nodo in cui non semplicemente non c'è condivisione di memoria, ma anche sistemi propriamente distribuiti su più nodi distinti. 

## Algoritmi di sincronizzazione
Il modello a scambio di messaggi è la naturale astrazione di un sistema distribuito, nel quale processi distinti eseguono su nodi fisicamente separati, collegati tra di loro attraverso una rete.

Come nel modello a memoria comune, anche nel modello a scambio di messaggi è importante poter disporre di algoritmi di sincronizzazione tra i processi concorrenti, che consentano di risolvere alcune problematiche comuni attraverso un opportuno coordinamento i vari processi.

Ad esempio:
- timing: sincronizzazione dei clock e tempo logico
- mutua esclusione distribuita
- elezione di coordinatori in gruppi di processi

Inoltre, è desiderabile che gli algoritmi distribuiti godano delle proprietà di scalabilità e di tolleranza ai guasti.




## Algoritmi per la gestione del tempo
In un sistema distribuito, ogni nodo è dotato di un proprio orologio. Se gli orologi locali di due nodi non sono sincronizzati, è possibile che se
un evento e2 accade nel nodo N2 dopo un altro evento e1 nel nodo N1, ad e2 sia associato un istante temporale precedente quello di e1.

    e2 > e1
    
    senza sincronizzazione tuttavia è possibile che:

    istante(e2) < istante(e1) 

Questo può risultare problematico in un sistema distribuito, ma non è stato spiegato perchè e in che casi.

## Soluzioni | Orologi logici
In applicazioni distribuite può essere necessario avere un unico riferimento temporale, condiviso da tutti i partecipanti.

Si può realizzare con:
- un orologio fisico universale: algoritmo che garantisce che tutti i nodi abbiano la stessa ora. Il problema non si pone più 
    - NON TRATTATO

- un orologio **logico**, che permetta di associare ad ogni evento un istante logico (timestamp) la cui relazione con i timestamp di altri eventi sia coerente con l’ordine in cui essi si verificano.

**FONDAMENTALE**: In un’applicazione distribuita, gli eventi sono legati da **vincoli di precedenza** che danno origine ad una relazione d’ordine parziale.

### Relazione di precedenza tra eventi (Happened-Before, ->):
se a e b sono eventi in uno stesso processo ed a si verifica prima di b, allora: **a->b**

Data una coppia di eventi (a,b) sono possibili 3 casi:
1. a->b, cioè a avviene prima di b
2. b->a, cioè b avviene prima di a
3. a e b non sono legati dalla relazione HB 
    - a e b sono **concorrenti** 

### Obiettivo di un orologio logico:
Si vuole definire un modo per misurare il concetto di tempo tale per cui, ad ogni evento *a* possiamo associare un timestamp *C(a)* sul quale tutti i processi siano d’accordo.

I timestamp devono soddisfare la seguente proprietà:
    
    se a -> b allora C(a) < C(b)

Quindi:
1. Se all’interno di un processo, *a* precede *b*, allora *C(a)* < *C(b)*
2. Se *a* è l’evento di invio (in un processo Ps) e *b* l’evento di ricezione (in un processo Pr) dello stesso messaggio *m*, allora *C(a)* < *C(b)*.
    - La relazione di precedenza deve quindi valere anche in nodi diversi

### Algoritmo di lamport
Ogni processo Pi mantiene localmente un **contatore del tempo logico Ci**, che viene gestito nel modo seguente:

1. Ogni nuovo evento all’interno di Pi provoca un incremento del valore di Ci: Ci=Ci+1
2. Ogni volta che Pi vuole inviare un messaggio m, il contatore Ci viene incrementato: Ci=Ci+1 e successivamente il messaggio viene inviato, insieme al timestamp Ci: ts(m)=Ci.
3. Quando un processo Pj riceve un messaggio m, Pj assegna al proprio contatore Cj un valore uguale a: Cj=max{Cj, ts(m)},  e successivamente lo incrementa di 1: Cj=Cj+1

**OSS**: In questo modo, su nodi diversi, i timestamp sono ordinati solo tra gli eventi legati da una relazione di precedenza, Ma questo è il caso che ci interessa.

Nei sistemi distribuiti l’algoritmo di Lamport viene generalmente eseguito da uno strato software (middleware) che interfaccia i processi alla rete: nella comunicazione tra processi diversi, essi vedono solamente il tempo logico degli altri.

**In breve**: alla ricezione di un messaggio modifico il mio orologio impostando come malore max{ts_in_entrata; my_ts}













## Mutua esclusione distribuita
Obiettivo: garantire che due o più processi non possano eseguire contemporaneamente certe attività. Ad esempio: accesso a risorse “condivise” : file, stampanti, ecc.

Soluzioni:
- **centralizzata**: la risorsa è gestita da un** processo dedicato (coordinatore)**, al quale tutti i processi si rivolgono per accedervi.
- **decentralizzata**: non è previsto un coordinatore, quindi i processi in competizione si sincronizzano tra loro tramite opportuni algoritmi, la cui logica è distribuita tra tutti i processi.

**NB**: Una soluzione decentralizzata è, in generale, più scalabile di soluzioni centralizzate, nelle quali il processo gestore della risorsa rappresenta un “collo di bottiglia”.

In generale, possiamo suddividere le soluzioni in due categorie:
- **Permission-based**: ogni processo che vuole eseguire la sua sezione critica, richiede un permesso ad uno o più altri processi.
- **Token-based**: un oggetto (“testimone”, o token) viene passato tra i vari processi in competizione. Il processo che possiede il token può:
    - eseguire la sua sezione critica, oppure
    - passare il token a un altro processo, se non intenzionato ad entrare nella sezione critica.

**NB**: Algoritmi token-based sono sempre decentralizzati, mentre algoritmi permissionbased possono essere sia centralizzati che decentralizzati.


### Prima soluzione | centralizzata permission-based 
L’algoritmo si basa su permessi (permission-based):
- la risorsa viene gestita da un processo coordinatore al quale ogni processo che vuole eseguire la sua sezione critica si rivolge per ottenere il permesso.
- per eseguire la propria sezione critica ogni processo Pi:
    1. Richiesta: Pi invia una richiesta di autorizzazione al coordinatore; quando verrà accolta, il processo Pi otterrà il permesso.
    2. <esegue la sezione critica>
    3. Rilascio: Pi comunica al coordinatore il termine della sezione critica
- Il coordinatore concede un permesso alla volta: ogni Richiesta ricevuta da un processo Pi mentre l’autorizzazione è concessa al processo Pj viene messa in attesa.
    - Il coordinatore mantiene una coda delle Richieste in attesa

**Punti interessanti**:
- soluzione poco scalabile: unico processo che fà da coordinatore è un collo di bottiglia (ed anche un single point of failure)
- se il coordinatore si guasta ci sarebbe bisogno di una election per designare un nuovo coordinatore.
- Un richiedente non può distinguere se un coordinatore è guasto oppure se ci sta solo mettendo tanto tempo a concederli il permesso di accesso alla sezione critica.
    - Soluzione: Appesantire la comunicazione con degli ack e usare un timeout sugli ack su quest'ultimi



### Algoritmo Ricart-Agrawala | decentralizzato permission-based
Il sistema è costituito da un insieme di processi in competizion. Ad ogni processo sono associate 2 attività concorrenti (es. thread):
- main: il thread che esegue la sezione critica
- receiver: il thread dedicato alla ricezione delle richieste di autorizzazione (il main si sospende in attesa delle autorizzazioni vere e proprie)

**Struttura del main**
Quando un processo vuole entrare nella sezione critica:
1. invia (n-1) richieste di autorizzazione ai receiver degli altri (n-1) nodi : Request(Pid, timestamp)
2. attende le (n-1) autorizzazioni
3. esegue la sezione critica
4. invia OK a tutte le richieste in attesa attesa arrivate nel durante l'attesa delle autorizzazioni sul receiver

**Struttura del receiver**:
Quando riceve una richiesta, il receiver può trovarsi in uno di tre possibili stati:
1. RELEASED. Il processo non è interessato ad entrare nella sezione critica: risponde OK.
2. WANTED. Il processo è in procinto di entrare nella sezione critica (attende autorizzazione): confronta il timestamp Tr della richiesta ricevuta con quello della richiesta inviata (Ts):
    - se Tr < Ts risponde OK (vai prima tu)
    - altrimenti non risponde e mette la richiesta ricevuta in coda (vado prima io)
3. HELD. Il processo sta eseguendo la sezione critica: la richiesta viene messa in coda...

**NB**: Ricart-Agrawala necessità di un orologio logico per determinare chi è che bisogna autorizzare per primo


...

**Punti interessanti**:
- Più scalabile rispetto a soluzione con processo coordinatore:
    - Qua la scalabilità non sta nel numero di messaggi che ogni processo deve smaltire ma nel costo computazionale nel servire ogni richiesta; in questo algoritmo ogni processo deve decidere solo se dare o meno l'ok piuttosto che mantenere una coda (non è vero).

- Non c'è più un singlo point of failure. Una volta capito che uno degli N nodi è fallito, il ripristino è molto semplice! Basta escludere il nodo guasto, niente elezione.
    - come ci sia accorge che il nodo è guasto? stesso problema di prima, magari ci sta mettendo solo tanto tempo a rispondere perchè sta eseguendo la sua sezione critica. 
    - come prima: ack sulla richiesta di autorizzazione, se scade un timeout il nodo viene considerato guasto ed escluso 

- maggiore costo di comunicazione: 2*(N-1) messaggi per ogni sezione critica


### Algoritmo Token-ring
I processi sono organizzati, logicamente, secondo una topologia ad anello.
- Ogni processo conosce i suoi vicini nell’anello.
- Un messaggio (detto token), circola attraverso l’anello, nel verso relativo all’ordine dei processi nella topologia.

Il token rappresenta il permesso unico di eseguire sezioni critiche. Il token viene inizialmente inviato da P0 a P1:
- P1 può essere nello stato WANTED: in questo caso trattiene il token ed esegue la sezione critica; al termine passa il token al processo successivo P2.
- P1 può essere nello stato RELEASED: in questo caso passa il token al processo successivo P2.
- P2 si comporta allo stesso modo di P1, come tutti i processi successivi nell’anello.


**Punti interessanti**:
- scalabile: 
    - la logica della mutua esclusione è distribuita sui vari processi e quindi non si appensantisce aggiungendo nodi (che i nodi siano 10000 o 10 ogni nodo ci mette lo stesso tempo a gestire il token)
    - al crescere del numero di processi, il tempo di attesa per riuscire ad eseguire una sezione critica diventa più lungo (il token deve fare un giro più lungo)
- Il numero di messaggi può essere illimitato se tutti i processi sono released; il ciclo si rompe quando un processo entra in una sezione critica, a cui verranno (forse ingiustamente) affibiati tutti i messaggi precedenti di scambio del token.
- Ragioniamo anche sulla fairness in questo caso; qua le richieste non vengono servite solamente in base all'ordine di formulazione, ma anche in base alla posizione dei processi richiedenti nella topologia! Token ring NON è fair. 

















## Algoritmi di elezione
In alcuni algoritmi è previsto che un processo rivesta un ruolo speciale nella sincronizzazione tra tutti i nodi. Tale processo viene detto coordinatore. (es. il coordinatore nell’algoritmo di mutua esclusione centralizzato).

Come viene individuato questo processo? La designazione del coordinatore può essere:
- statica: il coordinatore viene deciso in modo arbitrario dal programmatore/amministratore prima dell’esecuzione.
- **dinamica**: il coordinatore viene designato a runtime dai processi del sistema con un **algoritmo di elezione**.
    - Ad esempio: Se il coordinatore di un gruppo di processi subisce un crash, per ripristinare l’operatività del sistema, è necessario individuare a runtime un altro processo del gruppo a cui attribuire il ruolo di nuovo coordinatore.

### Algoritmo bully 
Quando un processo Pk rileva che il coordinatore non è più attivo (ack + timeout), organizza un’elezione:
1. Pk invia un messaggio “ELEZIONE” a tutti i processi con ID più alto: Pk+1,Pk+2,...,PN−1.
2. Se nessun processo risponde, Pk vince l’elezione e diventa il nuovo coordinatore; comunica quindi a tutti gli altri processi il nuovo ruolo tramite il messaggio “COORDINATOR”
    - all'ultimo giro di elezione nessun altro risponde
3. Se un processo Pj (j>k) risponde, Pj prende il controllo e Pk rinuncia ed esce dall’elezione.
    - prendere il controllo significa indirre a propria volta una elezione
    - Ogni processo attivo risponde ai messaggio di ELEZIONE con un messaggio "OK"

**NB**: All'ultimo giro di elezione, se è stata inviato un messaggio "ELEZIONE" ad un nodo che non risponde (vecchio coordinatore guasto), dopo un timeout esso viene considerato guasto.


### Algoritmo ad anello
Attenzione qua la topologia ad anello è di nuovo logica, ma sopratutto vale solo per il funzionamento dell'algoritmo di elezione. Per il resto, i vari processi si conoscono l'un l'altro e possono comunicare direttamente

La posizione (ID) che ogni processo occupa all’interno dell’anello rappresenta la sua priorità.

Il processo attivo con la massima priorità viene eletto coordinatore.

Quando un qualunque processo Pi si rende conto che il coordinatore non risponde più, inizia un’elezione:
1. Pi invia un messaggio ELEZIONE contenente il suo ID al suo successore Pi+1. Se il successore Pi+1 è in crash (non risponde con un ack), il messaggio viene spedito al successore di Pi.
2. Quando un processo Pj riceve un messaggio ELEZIONE:
- se il messaggio non contiene l’ID di Pj, aggiunge il suo ID al messaggio e lo spedisce al successivo ecc..
- se il messaggio contiene l’ID di Pj, significa che è stato compiuto un giro completo dell’anello: Pj designa come coordinatore il processo corrispondente all’ID più alto nel messaggio ricevuto e invia al successivo processo un messaggio COORDINATOR contenente l’ID del processo designato come nuovo coordinatore.
3. Quando un processo riceve un messaggio COORDINATOR, ne prende atto e inoltra lo stesso messaggio al successivo.