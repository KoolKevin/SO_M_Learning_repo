## Realizzazione dei semafori nel nucleo | modello a memoria comune
#### Caso monoprocessore
- p():
    - se n > 1 -> n--
    - se n == 1 
        - inserisco processo corrente nella coda dei processi sospesi del semaforo
        - invoco lo scheduler per effettuare un cambio di contesto verso un processo pronto
- v():
    - se n > 0 -> n++
    - se n == 0
        - recupero il primo processo che trovo nella coda del semaforo e lo inserisco nella coda dei processi pronti di sistema(con la priorità corretta)
        - in caso di priorità del risvegliato maggiore rispetto a quella del processo corrente: inserisco il corrente nella coda dei pronti e faccio un cambio di contesto verso il risvegliato

#### Caso multiprocessore | modello SMP
i semafori, come tutte le altre strutture dati del kernel, ora sono protette da lock

**Preemption**: L'esecuzione della V può portare al cambio di contesto tra il processo risvegliato (Pr) e uno tra i molteplici (uno per core) processi in esecuzione (Pi), se la priorità di Pr è più alta della priorità di Pi. In questo caso il nucleo deve provvedere a revocare l'unità di elaborazione al processo Pi con priorità più bassa ed assegnarla al processo Pr riattivato dalla V.
- Occorre che il nucleo mantenga l'informazione del processo in esecuzione a più bassa priorità e del nodo fisico su cui opera.
- Inoltre è necessario un meccanismo di segnalazione tra le unità di elaborazione.

Passaggi:
1. Pi chiama V(sem): Il nucleo, attualmente eseguito da Ui con la funzione V(sem), invia un segnale di interruzione a Uk.
2. Uk gestisce l’interruzione utilizzando le funzioni del nucleo: inserisce Pk nella coda dei processi pronti e mette in esecuzione il processo Pj.


#### Caso multiprocessore | modello SMP
Solo le interazioni tra processi appartenenti a nodi virtuali diversi utilizzano la memoria comune.

Distinzione tra:
- Semafori privati di un nodo U: cioè utilizzati da processi appartenenti al nodo virtuale U. Poichè hanno visibilità confinata al nodo U, vengono **realizzati come nel caso monoprocessore** (non c'è concorrenza?).
- **Semafori condivisi** tra nodi: cioè utilizzati da processi appartenenti a nodi virtuali differenti.

La memoria comune deve contenere tutte le informazioni relative ai semafori condivisi.


**Rappresentante del processo**: siccome un processo ora esegue su un nodo con memoria privata, ho bisogno di identificare su quale nodo sta eseguendo il processo e qual'è il suo descrittore nella memoria privata di questo nodo. Il rappresentate: Insieme minimo di informazioni sufficienti per
identificare sia il nodo fisico su cui il processo opera, sia il descrittore contenuto nella memoria privata del processore

Per ogni semaforo condiviso S vengono mantenute varie code:
- Su ogni nodo Ui: una coda locale a Ui contenente i descrittori dei processi locali sospesi su S; la coda risiede nella memoria privata del nodo e viene gestita esclusivamente dal nucleo del nodo.
- Una coda globale dei rappresentanti di tutti i processi sospesi su S, accessibile da ogni nucleo.

La somma dei descrittori di tutte le code locali forma la coda globale (si discriminano i descrittori con l'id del nodo)

**Esecuzione di una P sospensiva su un semaforo S condiviso (ValS=0)**:
il nucleo del nodo Ui sul quale opera il processo Pi che ha chiamato la P(s), provvede a:
- inserire il rappresentante di Pi nella coda globale dei rappresentanti associata a S.
- inserire il descrittore di Pi nella coda locale a Ui associata a S.

**Esecuzione di una V su un semaforo S (con rappresentanti in coda)**:
Assumendo che la V(S) sia chiamata dal processo Pj appartenente al nodo Uj:
1. Viene verificata le coda dei rappresentanti di S: il primo elemento è il rappresentante del processo Pi che appartiene al nodo Ui , che viene eliminato dalla coda.
2. il nucleo di Uj provvede a comunicare tramite interrupt (e previa scrittura in un buffer comune) l'identità del processo Pi risvegliato al nucleo di Ui;
3. Il nucleo di Ui provvede a riattivare il processo Pi, estraendo il suo descrittore dalla coda locale.








## Modello a scambio di messaggi
Con riferimento alla modalità di sincronizzazione tra i processi comunicanti, possiamo individuare 3 tipi di canale:
1. Comunicazione asincrona
    - Semantica: il processo mittente continua la sua esecuzione dopo l’invio del messaggio. In altre parole, L’invio di un messaggio non è un punto di sincronizzazione per mittente e destinatario
    - La ricezione del messaggio può avvenire in un istante successivo all’invio (quando pare al mittente). Questo permette di fare cose come: controllare se ci sono messaggi nel buffer( vedi _len()_ di go)
    - Da un punto di vista realizzativo, sarebbe necessario un buffer di capacità illimitata. Nella pratica, si sospende il processo mittente se il buffer è pieno C
2. Comunicazione sincrona (rendez-vous semplice)
    - Il primo dei due processi comunicanti che esegue l'invio (mittente) o la ricezione (destinatario) si sospende in attesa che l'altro sia pronto ad eseguire l'operazione corrispondente. L’invio di un messaggio è un punto di sincronizzazione!
    - Non è necessaria l'introduzione di un buffer
3. Comunicazione con sincronizzazione estesa (rendez-vous esteso)
    - Assunzione: ogni messaggio inviato rappresenta una richiesta al destinatario dell’esecuzione di una certa azione.
    - Semantica: Il processo mittente rimane in attesa fino a che il ricevente non ha terminato di svolgere l'azione richiesta. 
        - la sincronizzazione è estesa fino al completamento dell'azione richiesta, non solo al ricevimento del messaggio
    - Modello cliente-servitore. Analogia semantica con la chiamata di procedura.

**primitive di comunicazione**
- receive: in generale sempre bloccante
- send: 
    - caso asincrono -> bloccante solo se il canale è pieno
    - caso sincrono  -> bloccante fino alla corrispondente receive
    - caso esteso    -> bloccante fino al completamento dell'operazione richiesta


### Comandi con guardia
- A che cosa servono?
    - la receive è bloccante ed un processo server, in generale, espone molteplici servizi attraverso altrettante porte. Ma se il server si blocca su una porta, **non riesce a servire le richieste che arrivano sulle altre**.
    - con receive non bloccante tutto funziona, ma quando non ci sono messaggi il server fa dell'attesa attiva.
- Meccanismo di ricezione ideale:
    - consente al processo server di verificare contemporaneamente la disponibilità di messaggi su più canali;
    - abilita la ricezione di un messaggio da un qualunque canale contenente messaggi;
    - quando tutti i canali sono vuoti, **sospende** il processo in attesa che arrivi un messaggio, qualunque sia il canale su cui arriva,.

**Comando con guardia ripetitivo**:
do { 
    [ ] <guardia_1> -> <istruzione_1>;
    …
    [ ] <guardia_n> -> <istruzione_n>;
}

- racchiude un numero arbitrario di comandi con guardia semplici
Vengono valutate le guardie di tutti i rami; 3 casi:
1. se una o più guardie sono valide viene scelto, in maniera non deterministica, uno dei rami con guardia valida e la relativa guardia viene eseguita (viene cioè eseguita la receive); viene quindi eseguita l'istruzione relativa al ramo scelto, e poi si passa al ciclo successivo (ripetizione);

2. se tutte le guardie non fallite sono ritardate, il processo in esecuzione si sospende in attesa che arrivi un messaggio che abilita la transizione di una guardia da ritardata a valida; a quel punto procede come nel caso precedente (si passa all’iterazione successiva);

3. se tutte le guardie sono fallite, l'esecuzione del comando termina.










### Realizzazione delle porte e dei meccanismi di comunicazione
Sia nel caso di architetture mono e multielaboratore, una porta è sostanzialmente una coda (di messaggi) associata ad un processo. L'insieme delle porte di un processo sono salvate infatti nel suo descrittore.
- la send: inserisce un nuovo messaggio nella coda associata alla porta del destinatario, e **lo risveglia** in caso si fosse sospeso precedentemente. Non è mai sospensiva in quanto asincrona
- la receive recupera un messaggio dalla sua coda, se quest'ultima è vuota il processo si sospende in attesa che una send lo risvegli. Chiaramente bloccante











## Comunicazione con sincronizzazione estesa
Due diverse modalità di implementazione lato ricevente:
- chiamata di procedura remota
    - Per ogni operazione che un processo client può richiedere viene dichiarata, lato server, una procedura;
    - per ogni richiesta di operazione al server **viene creato un nuovo processo (thread)** che ha il compito di eseguire la procedura corrispondente.
    - **NB**: RPC rappresenta **solo un meccanismo di comunicazione tra processi**: la possibilità che più operazioni siano eseguite concorrentemente sul server implica la necessità di sincronizzazione tra i vari processi servitori (semafori, ecc.). La sincronizzazione è a carico del programmatore. 
- rendez-vous esteso 
    - L'operazione richiesta viene specificata come un insieme di istruzioni che può comparire in un punto qualunque del processo servitore (es. linguaggio ADA)
    - Il processo servitore utilizza un'istruzione di input (accept) che lo sospende in attesa di una richiesta dell'operazione.
    - All'arrivo della richiesta il processo esegue il relativo insieme di istruzioni ed i risultati ottenuti sono inviati al chiamante.
    - **NB**: combina **comunicazione con sincronizzazione**. Esiste, infatti, **un solo processo servitore** al cui interno sono definite le istruzioni che consentono di realizzare il servizio richiesto. **Il processo servitore si sincronizza con il processo cliente quando esegue l'operazione di accept** (es: ADA). 


### ADA
Comunicazione di tipo asimmetrico a rendez vous esteso:
- Ogni task può definire delle operazioni pubbliche (entry) visibili da altri task.
- L’interazione tra Task avviene attraverso il meccanismo del rendezvous: il Task Q chiama una entry E del task P(entry call); Q attende che l’esecuzione di E da parte di P sia completata (rendez-vous esteso)

**Call & Accept**
Una entry dichiarata in un task server S e resa visibile all’esterno di S, può essere chiamata da un task cliente C mediante call:
    S.entryname (<parametri effettivi>);

La comunicazione tra C e S avviene quando S esprime la volontà di eseguire la entryname mediante accept:
    accept entryname (in <par–in>, out <par–out>);
        do I1; I2;..In;
    end entryname;

Durante l’esecuzione delle istruzioni I1,..In i task C e S rimangono sincronizzati: C attende la terminazione della entry.

**OSS**: Call e Accept possono essere visti come un'evoluzione delle normali chiamate send e receive nel contesto della comunicazione con sincronizzazione estesa. Entrambe sono chiamate sincrone e quindi entrambe sono sospensive fino a quando l'esecuzione della entry non è terminata.

- Ad una stessa entry possono essere associate più accept. Ad esse possono corrispondere azioni diverse a seconda della fase di esecuzione del task.

**Coda associata ad ogni entry (gestita FIFO)**: una stessa entry può essere chiamata da più task prima che il task che la definisce esegua la corrispondente accept; ogni richiesta non ancora servita viene accodata.

**Select**
Anche in ADA c'è il problema di far esporre più servizi ad un unico task server. Come sempre la soluzione è un comando con guardia "_select_" che ha praticamente la stessa semantica di GO se non per la sincronizzazione estesa. 














## Algoritmi di sincronizzazione distribuiti
