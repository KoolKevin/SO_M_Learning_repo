## VIRTUALIZZAZIONE
Dato un sistema costituito da un insieme di risorse (hardware e software), virtualizzare il sistema significa presentare all’utilizzatore una visione delle risorse del sistema diversa da quella reale. Ciò si ottiene introducendo un livello di indirezione tra la vista logica e quella fisica delle risorse.
  - memoria virtuale: indirezione == traduzione con tabella delle pagine
  - processi:         indirezione == cambi di contesto con lo scheduler
  - codice Java:      indirezione == esecuzione sulla JVM
  - VM:               indirezione == gestione dell'hardware da parte dell'hypervisor

Con la virtualizzazione si disaccoppia il comportamento delle risorse di un sistema di elaborazione offerte all’utente, dalla loro realizzazione fisica.
- le risorse virtualizzate appaiono all’utente con caratteristiche in generale diverse e indipendenti da quelle “reali” (memoria virtuale illimitata e contigua, memoria fisica limitata e frammentata).

### ESEMPI DI VIRTUALIZZAZIONE
- virtualizzazione a livello di processo:    -> scheduling
  I sistemi multitasking permettono la contemporanea esecuzione di più processi, ognuno dei quali dispone di una macchina virtuale (CPU, memoria, dispositivi) dedicata. La virtualizzazione è realizzata dal kernel del sistema operativo.
- virtualizzazione della memoria            -> memoria virtuale
  In presenza di memoria virtuale, ogni processo vede uno spazio di indirizzamento di dimensioni indipendenti dallo spazio fisico effettivamente a disposizione. La virtualizzazione è realizzata dal kernel del sistema operativo.
- Astrazione:
  in generale un oggetto astratto (risorsa virtuale) è la rappresentazione semplificata di un oggetto (risorsa fisica):
    - esibendo le proprietà significative per l’utilizzatore
    - nascondendo i dettagli realizzativi non necessari.
  Es: tipi di dato vs. rappresentazione binaria nella cella di memoria. Il disaccoppiamento è realizzato dalle operazioni (interfaccia) con le quali è possibile utilizzare l’oggetto.

### EMULAZIONE
Altro esempio di virtualizzazione. Quando si ha la necessità di eseguire un programma compilato per una certa architettura, su una architettura diversa (ISA diverse) si utilizza emulazione. Traduzione da istruzioni macchina sorgente(nintendo) a istruzioni macchina destinazione(pc x86_64).  

Vengono emulate le singole istruzioni dell’architettura ospitata permettendo a sistemi operativi o applicazioni, pensati per determinate architetture, di girare, non modificati, su architetture completamente differenti.
- Vantaggi: interoperabilità tra ambienti eterogenei,
- Svantaggi: questo processo di traduzione ha ripercussioni sulle performance (problemi di efficienza).

L’approccio dell’emulazione ha seguito nel tempo due strade: **l’interpretazione e la ricompilazione dinamica.**
- **interpretazione**: ad ogni singola istruzione del codice macchina che deve essere eseguito, corrisponde l'esecuzione di una o più spesso, molteplici istruzioni dell’host virtualizzante per ottenere semanticamente lo stesso risultato
  - vantaggi: molto flessibile, posso fare cose come usare memoria per simulare dei registri che sulla mia architettura non ho  :)
  - svantaggi: inefficiente :(
- **ricompilazione dinamica (JIT)**: praticamente una compilazione a blocchi. Interpreta interi blocchi di codice traducendoli per la nuova architettura e nel frattempo ottimizzandoli. Si fa caching dei blocchi già compilati cosi se ricompaiono non si ricompila (hotspot).
  - vantaggi: gli stessi dell'interpretazione in più è molto più efficiente!
  - Tutti i più noti emulatori (es: QEMU , Virtual PC, Mame) utilizzano questa tecnica per implementare l’emulazione
- In toria è anche possibile la ricompilazione statica,  ma viene usata raramente perché i vantaggi in termini di performance spesso non superano la complessità aggiuntiva. 

### LIVELLI DI VIRTUALIZZAZIONE
- **virtual ISA**: Virtualizzazione dell'ISA tramite emulazione del set di istruzioni. Programmi che necessitano di un ISA che non possiedo, hanno l'impressione di avere l'ISA corretta.
- **virtual machine**: Questo è il livello della virtualizzazione delle macchine virtuali. La virtualizzazione qui emula un'intera macchina fisica, compreso l'hardware, in modo che VARI sistemi operativi possano girare su un UNICO host fisico. Ogni macchina virtuale ha l'impressione di avere le proprie risorse HW dedicate -> virtualizzazione dell'hardware
- **container**: Virtualizzazione del Sistema Operativo. Ogni container ha l'impressione di avere un proprio sistema operativo.
- **virtual library**: Librerie che traducono le chiamate di sistema di un ambiente virtuale in quelle dell'ambiente ospite. Ad esempio, WINE consente l'esecuzione di applicazioni Windows su Linux traducendo le chiamate di Windows in chiamate compatibili con Linux.
- **virtual application**:  Virtualizzazione del runtime dell'applicazione. Le applicazioni vengono eseguite in un ambiente runtime virtualizzato. permettendo loro di essere isolate dall'hardware e dal sistema operativo sottostante. Questo porta a una serie di vantaggi, tra cui portabilità, compatibilità e un migliore utilizzo delle risorse. Per esempio, la JVM permette di eseguire programmi Java su qualsiasi sistema operativo, indipendentemente dall'hardware sottostante.

NB: questa classificazione non è quasi mai inclusiva. I livelli sono separati.

### MACCHINE VIRTUALI
Una singola piattaforma hardware viene condivisa da più elaboratori virtuali (macchine virtuali o VM) ognuno gestito da un proprio sistema operativo. Il disaccoppiamento è realizzato da un componente chiamato Virtual Machine Monitor (VMM, o hypervisor) il cui compito è consentire la condivisione da parte di più macchine virtuali di una singola piattaforma hardware. Ogni VM contiene un proprio sistema operativo, che definisce un ambiente di esecuzione distinto e isolato della altre macchine virtuali, che consente l’esecuzione di applicazioni all’interno di esso.

Il VMM è il mediatore unico nelle interazioni tra le macchine virtuali e l’hardware sottostante, il che garantisce:
- isolamento tra le VM
- stabilità del sistema

### PERCHè VIRTUALIZZIAMO?
- Consolidamento HW
  -> utilizzo efficiente delle risorse hardware 
  -> Risparmio di spazio, costi, consumi, sprechi, ecc.
- Molteplici SO sulla stessa macchina fisica
  -> Possibilità di esecuzione di applicazioni concepite per un particolare s.o.
- Isolamento
  -> sandboxing, possibilità di effettuare testing di applicazioni preservando l'integrità degli altri ambienti e del VMM
  -> sicurezza, eventuali attacchi da parte di malware o spyware sono confinati alla singola macchina virtuale
- Gestione facilitata
  - riproducibilità
  - load balancing
  - migrazioni/recovery
  -> La creazione di VM è semplice grazie all'utilizzo delle immagini (archivi persistenti nel file system). Più istanze di una macchina virtuale possono utilizzare la stessa immagine. Esistono anche repository di immagini gia pronte.
  -> L'amministrazione di macchine virtuali (reboot, ricompilazione kernel, etc.)
  -> Migrazione a caldo di macchine virtuali tra macchine fisiche. Consiste nello spostare una macchina virtuale da un nodo fisico(magari guasto) ad un altro in maniera tale da garantire continuità di servizio. Utile in caso di: manutenzione, load-balancing, disaster-recovery. 
    -> A caldo = illusione che la macchina sia sempre online durante il trasferimento.