## Caratteristiche del modello
Modello alternativo, rispetto al modello a memoria comune, nel modellare l'interazione tra processi in un sistema operativo concorrente.

- Ogni processo può accedere esclusivamente alle risorse allocate nella propria memoria locale.
- Ogni risorsa del sistema è accessibile _direttamente_ ad un solo processo (il Gestore della risorsa, processo server)
- Se una risorsa è necessaria a più processi applicativi, ciascuno di questi (processi clienti) dovrà delegare l'unico processo che può operare sulla risorsa (processo gestore, o server) l'esecuzione delle operazioni richieste; al termine di ogni operazione il gestore restituirà al cliente gli eventuali risultati.

__Ricorda__: data una risorsa R, il suo gestore GR è costituito da:
- una risorsa condivisa, in un sistema organizzato secondo il modello a memoria comune
- un processo, in un sistema organizzato secondo il modello a scambio di messaggi

In questo modello, ogni processo, per usufruire dei servizi offerti da una risorsa, dovrà comunicare con il relativo gestore.
    
    Il meccanismo di base utilizzato dai processi per qualunque tipo di interazione con la risorsa è costituito dallo scambio di messaggi.

## Canali di comunicazione
... vari richiami sui parametri che caratterizzano un canale

Di particolare interesse è: il __tipo di sincronizzazione fra i processi comunicanti__.

Possiamo individuare 3 tipologie di canale per questo parametro (in ordine di paralellismo permesso):
- Comunicazione asincrona:
    - Semantica: Il processo mittente continua la sua esecuzione immediatamente dopo l’invio del messaggio.
    - L’invio di un messaggio non è un punto di sincronizzazione per mittente e destinatario. Questo tipo di comunicazione favorisce il grado di concorrenza.
    - Teoricamente si avrebbe necessità di un buffer infinito come canale su cui accodare i messaggi. In pratica quando il canale è pieno il processo mittente viene sospeso.
    - La receive sul canale è comunque __bloccante__ anche in questo tipo di semantica 
- Comunicazione sincrona (rendez-vous semplice)
    - Semantica: Il primo dei due processi comunicanti che esegue l'invio (mittente) o la ricezione (destinatario) si sospende in attesa che l'altro sia pronto ad eseguire l'operazione corrispondente.
    - L’invio di un messaggio è un punto di sincronizzazione.
    - Non è necessaria l'introduzione di un buffer: un messaggio può essere inviato solo se il ricevente è pronto a riceverlo.
- Comunicazione con sincronizzazione estesa (rendez-vous esteso)
    - Assunzione: ogni messaggio inviato rappresenta una richiesta al destinatario dell’esecuzione di una certa azione.
    - Semantica: Il processo mittente continua a rimanere in attesa fino a che il ricevente non ha terminato di svolgere l'azione richiesta.
    - Semantica tipica di un modello cliente-servitore (RPC)
    - Questo tipo di comunicazione è quello che sincronizza di più è che quindi permette il minor grado di parallelismo

La semantica asincrona è quella più "granulare", permette di implementare tutti gli altri tipi di semantica. Non è vero il contrario. Per questo motivo è quella sicuramente implementata nei linguaggi di programmazione.

### Tipologie di canale 
1. link: da-uno-a-uno (canale simmetrico)
2. port: da-molti-a-uno (canale asimmetrico)
3. mailbox: da-molti-a-molti (canale asimmetrico)

### Send 
send è la primitiva che esprime l’invio di un messaggio.

Semantica di una send può essere
- asincrona 
    - canale bufferizzato
    - il processo attende solo se il canale è pieno
- sincrona 
    - canale a capacità nulla.
    - Il processo attende che il destinatario esegua la primitiva di ricezione (receive) corrispondente;

### Receive
send è la primitiva che esprime la ricezione di un messaggio.

Semantica:
- __Semantica Bloccante__ (default):
    - La primitiva __sospende__ il processo se non ci sono messaggi sul canale; quando c’è almeno un messaggio nel canale, ne estrae il primo e ne assegna a <variabile> il valore. La receive restituisce un valore del tipo predefinito che __identifica il nome del processo mittente__.
- Alcuni linguaggi offrono anche una __semantica non bloccante__
    - se il canale è vuoto, il processo continua; se contiene almeno un messaggio, estrae il primo e lo assegna a <variabile>.
    - una sorta di peek.

la semantica bloccante è il default nella maggior parte dei casi.

## Meccanismo di ricezione ideale | comando con guardia
Un comando con guardia è una istruzione che i linguaggi di programmazione basati sul modello a scambio di messaggi prevedono.

### Comando con guardia alternativo

__Semantica__:
Vengono __valutate le guardie di tutti i rami__. Si possono verificare 3 casi:
1. se una o più guardie sono valide viene scelto, in maniera __non deterministica__, uno dei rami con guardia valida e la relativa guardia viene eseguita (cioè, viene eseguita la receive); viene quindi eseguita l'istruzione relativa al ramo scelto, e con ciò __termina l'esecuzione dell'intero comando alternativo__.
    - Non esistono servizi/rami privilegiati.

2. se tutte le guardie non fallite sono ritardate, il __processo in esecuzione si sospende__ in attesa che arrivi un messaggio che abilita la transizione di una guardia da ritardata a valida e a quel punto procede come nel caso precedente.

3. se tutte le guardie sono fallite il comando termina.

### Comando con guardia alternativo
Vengono __valutate le guardie di tutti i rami__. Si possono verificare 3 casi:
1. Se una o più guardie sono valide, come nel caso alternativo ma invece di terminare dopo l'esecuzione del ramo, si passa ad una iterazione sucessiva del comando.

2. come nel caso alternativo, ma dopo l'arrivo di un messaggio che sblocca uno dei rami sospesi e l'esecuzione della relativa istruzione, si passa ad una nuova iterazione che rivaluta i rami che prima erano falliti.

3. come nel caso alternativo

__NB__: In questo coso, per uscire dal _do_ bisogna prevedere delle guardie logiche per ogni ramo. Il comando termina solo se tutte le guardie sono fallite. 
