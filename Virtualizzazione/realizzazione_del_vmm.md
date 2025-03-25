Apriamo la scatola e capiamo come è possibile realizzare le vm.

## VMM
In generale, il VMM deve offrire alle diverse macchine virtuali le risorse HW (virtuali) che sono necessarie per il loro funzionamento:
- CPU
- Memoria
- Dispositivi di I/O

**REQUISITI VMM**
1. Ambiente di esecuzione per i programmi sostanzialmente identico a quello della macchina reale.
    - Gli stessi programmi che eseguono sull’architettura non virtualizzata possono essere eseguiti nelle VM senza modifiche.
2. Garantire un’elevata efficienza nell’esecuzione dei programmi.
    - Quando possibile, il VMM deve permettere l’esecuzione diretta delle istruzioni impartite dalle macchine virtuali 
    - Le istruzioni non privilegiate vengono eseguite direttamente in hardware senza coinvolgere il VMM (forwarding delle istruzioni non privilegiate).
3. Garantire la stabilità e la sicurezza dell’intero sistema.
    - Il VMM deve rimanere sempre nel pieno controllo delle risorse hardware
    - I programmi in esecuzione nelle macchine virtuali e le vm stesse, non possono accedere all’hardware in modo privilegiato.

**NB**: Siccome Bisogna, garantire stabilità e sicurezza, il VMM si trova ad eseguire nel ring 0. I kernel delle VM invece, si trovano ad eseguire in un ring che NON PUò essere quello di massimo privilegio. 

### CLASSIFICAZIONE VMM
Distinguiamo due caratteristiche:
- Livello dove è collocato il VMM:
    - **VMM di sistema**: eseguono direttamente sopra l’hardware dell’elaboratore (es. vmware, xen, kvm)
        - in sostanza, dei mini sistemi operativi: gestiscono le risorse fisiche dell'HW e le assegnano alle VM, in maniera analoga a quello che fa un SO con i processi. Un VMM è però più leggero rispetto un intero kernel in quanto non deve implementare funzionalità come il file system oppure la gestione della memoria. Queste funzionalità sono responsabilità dei SO delle VM.
        - chiaramente un VMM dato che deve interagire con le periferiche deve disporre di tutti i driver per quest'ultime
    - **VMM ospitati**: eseguiti come applicazioni sopra un S.O. esistente (es. virtualbox)
        - il VMM opera nello spazio utente e **accede all’hardware tramite le system call del S.O. su cui viene installato**.
        - Più semplice l’installazione (come un’applicazione).
        - Può fare riferimento al S.O. sottostante per la gestione delle periferiche e può utilizzare altri servizi del S.O. (es. scheduling, gestione dei dispositivi, ecc.).
        - Peggiore la performance a causa dell'indirezione (rispetto al VMM di sistema).

- Modalità di dialogo per l’accesso alle risorse fisiche tra la macchina virtuale ed il VMM:
    - **Virtualizzazione pura**: le macchine virtuali usano la stessa interfaccia (istruzioni macchina) dell’architettura fisica. Il codice (assembly) eseguito dalla vm è esattamento lo stesso di quello nel caso in cui la macchina non fosse virtuale. Posso installare esattamente la stessa versione del SO che installerei in una macchina non virtuale.
        - es. Vmware
    - **Paravirtualizzazione** (xen): il VMM presenta un’interfaccia diversa da quella dell’architettura hw. Il sistema operativo delle vm deve essere ricompilato per l'interfaccia esposta dal VM. Il codice (istruzioni privilegiate) deve adattarsi all'interfaccia del VMM. In questo caso devo usare una versione del SO specifica per il VMM.
        - es. Xen

**NOMENCLATURA**
- Host: piattaforma di base sulla quale si realizzano macchine virtuali. Comprende la macchina fisica ed il VMM (ed il sistema operativo dell'host in caso di VMM OSPITATO).
- Guest: la macchina virtuale. Comprende applicazioni e sistema operativo (della VM)

**OSS**: il vmm è una sorta di kernel di un SO, 


## VMM e RING
HP: Facciamo riferimento ai VMM di sistema

Ricordando il terzo requisito: "Il vmm deve essere l'unica componente autorizzata ad avere controllo dell'HW (istruzioni privilegiate)" 
- unica componente che esegue nel ring 0, il resto, comprese le VM eseguono in ring > 0.
- tuttavia in ogni VM abbiamo un kernel di un SO progettati per eseguire nel ring 0 (contiene istruzioni privilegiate)
    -> Come si puo permettere a tali kernel di eseguire queste istruzioni per permettere loro un corretto funzionamento, pur non trovandosi nel ring 0?

### PROBLEMI
- **Ring deprivileging**: il s.o. della macchina virtuale esegue in un ring che non gli e` proprio. Le istruzioni privilegiate richieste dal sistema operativo nell’ambiente guest non possono essere eseguite (perchè richiederebbero il ring 0, e il kernel della VM esegue in un ring di livello superiore).
- **Ring compression**: se i ring utilizzati sono solo 2, applicazioni e s.o. della macchina virtuale eseguono allo stesso livello:
    -> scarsa protezione tra  SO e applicazioni della VM.

### SOLUZIONI A RING DEPRIVILEGING
Una prima soluzione è fare **trap & emulate**. Se il guest tenta di eseguire un’istruzione privilegiata:
- la CPU notifica un’eccezione al VMM (trap) e gli trasferisce il controllo
- il VMM controlla la correttezza dell’operazione richiesta e ne emula il comportamento (emulate).

NB: Le istruzioni non privilegiate possono essere eseguite direttamente dalle VM senza alcun intervento da parte del VMM (ricorda requisito due di efficienza).

**Esempio**: tentativo di esecuzione da parte del guest dell’istruzione privilegiata per la disabilitazione delle interruzioni (popf).
- Se la richiesta della macchina virtuale fosse eseguita direttamente sul processore, sarebbero disabilitati gli interrupt per tutto il sistema.
    - Il VMM non potrebbe riguadagnare il controllo della CPU.
- Il comportamento desiderato è che la consegna degli interrupt vada sospesa solamente per la macchina virtuale richiedente.
- Con trap&emulate, il VMM riceve la notifica di tale richiesta (effettuata da una particolare VM) e ne emula il comportamento atteso sospendendo le interruzioni solo per la VM richiedente.

### SUPPORTO HW ALLA VIRTUALIZZAZIONE
L’architettura della CPU si dice NATURALMENTE VIRTUALIZZABILE se prevede l’invio di trap allo stato supervisore per ogni
istruzione privilegiata invocata da un livello di protezione diverso dal Supervisore.
- avremo un registro che conterrà l'indirizzo del vettore per le trap al supervisore

Se l'architettura della CPU e` naturalmente virtualizzabile, abbiamo:
- la realizzazione del VMM e` semplificata: per ogni trap generato dal tentativo di esecuzione di istruzione privilegiata dal guest viene eseguita una routine di emulazione. (Approccio “trap-and-emulate”)
- Supporto nativo all'esecuzione diretta. 
- Es. Intel VT, AMD-V

**PROBLEMA: Non tutte le architetture sono naturalmente virtualizzabili!** 

Esempio: Intel IA32; Alcune istruzioni privilegiate di questa architettura invocate a livello user non provocano una trap, ma:
- vengono ignorate non consentendo quindi l’ intervento trasparente del VMM,
- in alcun casi provocano il crash del sistema.

**SOLUZIONE**: Se il processore non fornisce alcun supporto alla virtualizzazione, è necessario ricorrere a soluzioni TOTALMENTE SOFTWARE. Due possibilità:
- **FAST BINARY TRANSLATION**: Il VMM scansiona dinamicamente il codice dei SO guest prima dell’esecuzione per sostituire a run time i blocchi contenenti istruzioni privilegiate in blocchi equivalenti dal punto di vista funzionale, contenenti **chiamate al VMM**.
    - I blocchi tradotti sono eseguiti e conservati in cache per eventuali riusi futuri.
    - chiamate al VMM (hypercall) ricordano le chiamate al kernel fatte dai processi in user space mediante system call.
    - Pro: ogni macchina virtuale è una esatta replica della macchina fisica
        - possibilità di installare gli stessi s.o. di architetture non virtualizzate **(Virtualizzazione pura)**
    - Contro: la traduzione dinamica è costosa!
- **PARAVIRTUALIZZAZIONE**: versione statica di quanto detto sopra 
    - i kernel dei SO per le VM devono essere ricompilati preventivamente per sostituire le chiamate a istruzioni privilegiate con chiamate al hypervisor
        - hypercall, system call al vmm.
    - Pro: la struttura del VMM è semplificata perché non deve più preoccuparsi di tradurre dinamicamente i tentativi di operazioni privilegiate dei S.O. guest.
    - Contro: necessità di porting (ricompilazione del kernel) dei S.O. guest (le applicazioni rimangono invariate). Soluzione preclusa a molti sistemi operativi proprietari non open source.

**NB**: in entrambe queste soluzione non si generano più trap, ma piuttosto si effettuano chiamate esplicite all'hypervisor che gestirà l'esecuzione dell'istruzione privilegiata richiesta.   

**NB**: L’ uscita sul mercato di processori con supporto nativo alla virtualizzazione (Intel VT, AMD-V) ha dato l’impulso allo sviluppo di VMM semplificati, basati su virtualizzazione PURA:
- No Ring Compression/Aliasing: il s.o. guest esegue in un ring intermedio separato da quello delle applicazioni
- Ring Deprivileging: ogni istruzione privilegiata richiesta dai SO guest genera un trap gestita dal VMM mediante trap & emulate dall'Hypervisor grazie al supporto hardware alla virtualizzazione.
- Efficienza: non c’è bisogno di binary translation.
- Trasparenza: l’API presentata dall’hypervisor è la stessa offerta dal processore.




## PROTEZIONE in x86 classico
in questo contesto con protezione si intende la distinzione tra: sistema operativo, che possiede controllo assoluto sulla macchina fisica sottostante, e le applicazioni che possono interagire con le risorse fisiche solo facendone richiesta al SO (syscall).

    Registro CS nelle vecchie architetture x86: i due bit meno significativi vengono riservati per rappresentare il livello corrente di privilegio (CPL). Quattro possibili ring:
        - Ring 0 dotato dei maggiori privilegi e quindi destinato al kernel del sistema operativo.
        - …
        - Ring 3, quello dotato dei minori privilegi e quindi destinato alle applicazioni utente.

**PROTEZIONE DELLA CPU**: non è permesso a ring diversi dallo 0 di eseguire le istruzioni privilegiate, che sono destinate solo al kernel del sistema operativo, in quanto considerate critiche e potenzialmente pericolose. Una qualsiasi violazione di questo comportamento può provocare un’eccezione, con l’immediato passaggio al sistema operativo, il quale, catturandola, potrà correttamente gestirla, terminando ad esempio l’applicazione in esecuzione.

**PROTEZIONE DELLA MEMORIA**
- Segmentazione: ogni segmento è rappresentato da un descrittore in una tabella (Global Descriptor Table(GDT) o Local Descriptor Table(LDT)); nel descrittore sono indicati il livello di protezione richiesto (PL) e i permessi di accesso (r,w,x).

Una violazione dei vincoli di protezione provoca una eccezione. Cio’ accade, ad esempio, se il valore di CPL è maggiore del PL del segmento di codice contenente l’istruzione invocata.


**USO DEI RING**:
Nonostante la famiglia x86 possieda 4 ring di sicurezza, ne sono comunemente utilizzati soltanto 2 per mantenere la massima portabilità dei sistemi operativi verso processori con solo 2 ring di protezione:
- Il ring 0 (quello a più alto privilegio) destinato al sistema operativo;
- Il ring 3 nel quale sono eseguite le applicazioni.

I VMM in questa architettura eseguono tipicamente con una delle seguenti configurazioni:
- 0/1/3: 
    - affetta da ring deprivileging
        - si risolve come visto sopra (trap&emulate se HW virtualizzabile, altrimenti binary translation/paravirtualizzazione)
    - non affetta da ring compression
        - i programmi utente possono generare eccezioni rispetto al SO guest, quest'ultime vengono catturate dall'hypervisor che le inoltra inalterate al giusto SO guest
    - non compatibile con sistemi a 64 bit, non si sa il perchè
- 0/3/3: 
    - affetta da ring deprivileging ma anche ring compression
        - i programmi utente della macchina guest possono fare le stesse cose del SO guest. Non bello.
        - i programmi utente non possono generare eccezioni rispetto al SO guest in quando hanno lo stesso privilegio
            - il VMM si complica e deve sempre controllare tutto







## GESTIONE DI VM
Come abbiamo gia visto un VMM non è troppo diverso da un SO, invece però di gestire processi, gestisce VM. Oltre alla spartizione dell'HW dell'host tra le VM, il VMM ha i seguendi compiti:
- creazione/eliminazione delle VM
- spegnimento/accensione delle VM
- migrazione live delle VM

**STATI DI UNA VM**: analoghi a quelli dei processi
- **Running** (o attiva): la macchina è accesa e occupa memoria nella RAM del server sul quale è allocata.
- **Paused**: la macchina virtuale è in attesa di un evento (es. I/O richiesto da un processo nell’ambiente guest). Stato in RAM
- **Suspended**: la macchina virtuale è stata sospesa dal VMM; il suo stato e le risorse utilizzate sono salvate nel FILE SYSTEM (file immagine). L’uscita dallo stato di sospensione avviene tramite l’operazione resume da parte del VMM.
    - non devo rifare il boot, devo solo caricare la VM da disco
- **Inactive** (Powered Off/spenta): la macchina è spenta ed è conservata nel FILE SYSTEM tramite un file IMMAGINE.
    - devo rifare il boot

**MIGRAZIONI di VM**: In datacenter di server virtualizzati è sempre più sentita la necessità di una gestione agile delle VM per fare fronte a:
- Variazioni dinamiche del carico: load balancing, consolidamento
- Manutenzione “online” dei server
- Gestione finalizzata al risparmio energetico
- Tolleranza ai guasti/disaster recovery

In tutti questi casi la possibilità di muovere VM tra server, **mantenendo la continuità di servizio**, è un meccanismo fondamentale per la soluzione.
- migrazione live = VM per maggior parte del tempo accesa (running e paused) e minima parte del tempo sospesa. Mai spenta.

Il VMM può mettere in stand-by una VM tramite l’operazione suspend: lo stato della macchina viene salvato in memoria secondaria. Una VM suspended può riprendere l’esecuzione, a partire dallo stato in cui si trovava quando è stata sospesa tramite l’operazione resume. Lo stato salvato viene ripristinato in memoria centrale.
- Poichè una VM è quasi completamente indipendente dal server fisico su cui è allocata, la resume può avvenire su un nodo diverso da quello in cui era prima della sospensione (migrazione).

**REALIZZAZIONE DELLA LIVE MIGRATION**
E’ desiderabile minimizzare:
- Downtime (tempo in cui la macchina non risponde alle richieste degli utenti)
- Tempo di migrazione
- Consumo di banda (Se il file system è condiviso tra A e B, non c’è bisogno di copiare il file immagine)
    
**PRECOPY**
La migrazione viene realizzata in 6 passi:
1. Pre-migrazione: 
    - individuazione della VM da migrare e dell’host (B) di destinazione
2. Reservation:
    - viene inizializzata una VM sul server di destinazione
3. Pre-copia iterativa delle pagine:
    - viene eseguita una copia nell’host B di tutte le pagine allocate in memoria sull’host A per la VM da migrare; successivamente vengono iterativamente copiate da A a B tutte le pagine modificate (dirty pages) fino a quando il numero di dirty pages è inferiore a una soglia data.
        - notare che le pagine dirty vengono trasferite inutilmente ma lo devo per forza fare se voglio mantere la VM live
4. Sospensione della VM e copia dello stato nelle dirty pages rimanenti da A a B.
    - Siccome la VM in questa fase è sospesa, lo stato non puo più cambiare.
    - Siccome ho fatto una precopia della maggior parte delle pagine, il tempo in cui la VM è sospesa è minimo   
5. Commit: la VM viene eliminata dal server A
6. Resume: la VM viene attivata nel server B

**POSTCOPY**
In alternativa a precopy, la macchina viene sospesa e vengono copiate (non iterativamente, non viene modificato niente) pagine e stato. Tempo di migrazione più basso (non trasferisco pagine inutili), ma downtime molto più elevato.
