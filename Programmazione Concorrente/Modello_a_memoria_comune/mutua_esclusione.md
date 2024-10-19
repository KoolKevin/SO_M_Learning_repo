### Mutua Esclusione
- Il problema della mutua esclusione nasce quando più di un processo alla volta può aver accesso a variabili comuni.
- La regola di mutua esclusione impone che le operazioni con le quali i processi accedono alle variabili comuni non si sovrappongano nel tempo.
- Nessun vincolo è imposto sull’ordine con il quale le operazioni sulle variabili vengono eseguite.

NB: se devo garantire solamente mutua esclusione non è necessario definire una soluzione per l'accesso alla risorsa condivisa che imponga un __ordine__ di accesso alla risorsa

SEZIONE CRITICA
La sequenza di istruzioni con le quali un processo accede e modifica un insieme di variabili comuni prende il nome di sezione critica.

Ad un insieme di variabili comuni possono essere associate una sola sezione critica (usata da tutti i processi) o più sezioni critiche (classe di sezioni critiche). 

Associare più sezioni critiche a un insieme di variabili comuni vuol dire che non tutti i processi devono utilizzare la stessa sezione critica per accedere alle variabili condivise. In questo caso, le variabili condivise possono essere raggruppate o suddivise in base alle operazioni che i processi eseguono su di esse, e ogni gruppo di operazioni può avere la propria sezione critica.

Questo è utile quando i processi eseguono operazioni differenti sulle variabili condivise non conflittuali. In questo modo non devono  attendere l'accesso alla sezione critica, causando possibili ritardi o rallentamenti inutili.

La regola di mutua esclusione stabilisce che: Sezioni critiche appartenenti alla stessa classe devono escludersi mutuamente nel tempo.

### Strumenti linguistici per la programmazione di interazioni

__IL SEMAFORO__
strumento generale per risolvere qualunque problema di sincronizzazione nel modello a memoria comune.
- Strumento software
- non serve solo per la mutua esclusione

E’ realizzato dal nucleo della macchina concorrente:
- L’eventuale attesa nell’esecuzione può essere realizzata utilizzando i meccanismi di gestione dei thread (sospensione, riattivazione) offerti dal nucleo.
- E’ normalmente utilizzato per realizzare strumenti di sincronizzazione di livello più alto (es: condition).

__DEF__: Un semaforo è una variabile __intera non negativa__, alla quale è possibile accedere solo tramite le due operazioni P e V.

Specifica delle due operazioni:
- void P(semaphore s): region s << when(vals > 0) vals--;>>
    - p -> sospensione
- void V(semaphore s): region s << vals++;>>
    - v -> risveglio

NB: Essendo l’oggetto s condiviso, le due operazioni P e V vengono definite come sezioni critiche da eseguire in mutua esclusione.