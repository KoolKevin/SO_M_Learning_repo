### MODELLI DI INTERAZIONE TRA PROCESSI (CONCORRENTI)
    • Modello a memoria comune (ambiente globale, shared memory)
    • Modello a scambio di messaggi (ambiente locale, distributed memory)
due modelli di interazione utilizzati per gestire la comunicazione e la sincronizzazione tra processi o thread che eseguono operazioni simultaneamente.

### MODELLO A MEMORIA COMUNE
Il modello a memoria comune rappresenta la naturale astrazione del funzionamento di un sistema in multiprogrammazione costituito da uno o più processori che hanno accesso ad una memoria comune. Ad ogni processore può essere associata una memoria privata, ma ogni interazione avviene tramite oggetti contenuti nella memoria comune.

Ogni applicazione concorrente viene strutturata come un insieme di componenti, suddiviso in due sottoinsiemi disgiunti:
    • processi (componenti attivi)
    • risorse (componenti passivi).
Le relazioni tra risorse e processi di una applicazione concorrente vengono rappresentate tramite un grafo di allocazione delle risorse.

Risorsa: qualunque oggetto, fisico o logico, di cui un processo necessita per portare a termine il suo compito.
    • Le risorse sono raggruppate in classi; una classe identifica l’insieme di tutte e sole le operazioni che un processo può eseguire per operare su risorse di quella classe.
    • ogni risorsa si identifica con una struttura dati (che la rappresenta) allocata nella memoria comune.
    (Vale anche per risorse fisiche: descrittore del dispositivo)


### Gestore di una risorsa
Per ogni risorsa R, il suo gestore definisce, in ogni istante t, l’insieme SR(t) dei processi che, in tale istante, hanno il diritto di operare su R.

Classificazione risorse:
    • Risorsa R dedicata:               se SR(t) ha una cardinalità sempre ≤ 1
    • Risorsa R condivisa:              in caso contrario
    • Risorsa R allocata staticamente:  se SR(t) è una costante: SR(t) = SR (t0), per ogni t
    • Risorsa R allocata dinamicamente: se SR(t) è funzione variabile del tempo

Distinguiamo anche tra risorse private e risorse comuni.

![alt text](.\tipologie_di_allocazione_delle_risorse.png)

Per ogni risorsa allocata staticamente, l’insieme SR(t) è definito prima che il programma inizi la propria esecuzione;
il gestore della risorsa è il programmatore che, in base alle regole di visibilità del linguaggio, stabilisce quale processo può “vedere” e quindi operare su R.

Per ogni risorsa R allocata dinamicamente, il relativo gestore GR definisce l’insieme SR(t) in fase di esecuzione e quindi deve essere un componente della stessa applicazione, nel quale l’allocazione viene decisa a run-time in base a politiche date.

Compiti del gestore di una risorsa:
1. mantenere aggiornato l’insieme SR(t) e cioè lo stato di allocazione della risorsa;
2. fornire i meccanismi che un processo può utilizzare per acquisire il diritto di operare sulla risorsa, entrando a far parte dell’insieme SR(t), e per rilasciare tale diritto quando non è più necessario;
3. implementare la strategia di allocazione della risorsa e cioè definire quando, a chi e per quanto tempo allocare la risorsa.

Se R è allocata come risorsa condivisa, (modalità B e D) è necessario assicurare che gli accessi avvengano in modo non divisibile. Ovvero, le funzioni di accesso alla risorsa devono essere programmate come una classe di sezioni critiche.
    -> questo è il caso del modello a memoria comune

### SEZIONE CRITICA CONDIZIONALE
Formalismo che consente di esprimere la specifica di qualunque vincolo di sincronizzazione.

Data una risorsa R condivisa:
    region R << S_a; when(C) S_b;>>
- il corpo della region rappresenta un’operazione da eseguire sulla risorsa condivisa R e quindi costituisce una sezione critica che deve essere eseguita in __mutua esclusione__ con le altre operazioni definite su R.
- il corpo della region è costituito da due istruzioni da eseguire in sequenza: l’istruzione Sa e, successivamente, l’istruzione Sb.
- In particolare,una volta terminata l’esecuzione di Sa viene valutata la condizione C
    - se C è vera l’esecuzione continua con Sb,
    - se C è falsa il processo che ha invocato l’operazione attende che la condizione C diventi vera. A quel punto (quando C è vera) l’esecuzione della region può riprendere e essere completata mediante l’esecuzione di Sb.
        -> NB: la risorsa viene liberata dalla mutua esclusione se la condizione non viene soddisfatta