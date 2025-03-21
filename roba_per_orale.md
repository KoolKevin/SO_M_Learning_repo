## Realizzazione dei semafori nel nucleo

### Caso modello a memoria comune

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