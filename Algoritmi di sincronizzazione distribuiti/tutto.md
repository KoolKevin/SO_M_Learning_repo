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