VMM open source che opera secondo i principi della paravirtualizzazione.
    • Porting di Linux su XEN (XenoLinux, Suse, CentOs, …). Modifica del Kernel di Linux per dialogare con le API di XEN pari a circa 3000 linee di codice (1,36% del
      totale).
    • Porting di Windows XP (XenoXP) in collaborazione con Microsoft. Lavoro non completato.    
    • Praticamente sono disponibili solo distribuzioni Linux come VM. 

--- ARCHITETTURA DI XEN
Sistema paravirtualizzato con virtualizzazione nativa.
    • il vmm (hypervisor) si occupa della virtualizzazione della CPU, della memoria e dei dispositivi per ogni macchina virtuale (chiamate domain_i)
    • Xen dispone di un'interfaccia di controllo in grado di gestire la divisione di queste risorse tra i vari domini. (praticamente console di virtualbox)
    • L'accesso a questa interfaccia di controllo è ristretta; può essere controllata solamente utilizzando una VM dedicata a questo compito: domain 0. In questo dominio
      viene eseguita l'applicazione software che gestisce il controllo di tutto il sistema.
        -> domain0: particolare VM che serve ad amministrare il resto del sistema
    • Il software di controllo nel domain 0 (dom0) è separato dallo stesso hypervisor -> separazione dei meccanismi dalle politiche.
    
    
--- XEN | CARATTERISTICHE
Paravirtualizzazione:
  • Le macchine virtuali eseguono direttamente le istruzioni NON privilegiate
  • L’esecuzione di istruzioni privilegiate viene delegata al VMM tramite chiamate al VMM (hypercalls).
Protezione (stesso modello di x86):
  • I sistemi operativi guest OS sono collocati nel ring 1
  • VMM collocato nel ring 0

--- XEN | GESTIONE DELLA MEMORIA E PAGINAZIONE
Gestione della Memoria:
  • i SO guest gestiscono la memoria virtuale, mediante i tradizionali meccanismi/politiche di paginazione
  • page faults gestiti direttamente a livello HW (TLB, come per x86)

Soluzione adottata, le tabelle delle pagine delle VM:
  • vengono mappate nella memoria fisica dal VMM (shadow page tables);
  • non possono essere accedute in scrittura dai kernel guest, ma solo dal VMM;
  • sono accessibili in modalità read-only anche dai guest; in caso di necessità di update, interviene il VMM che valida le richieste di update dei guest e le esegue.

Ecco come funziona:
  • Tabelle delle pagine:
    Ogni sistema operativo gestisce la memoria attraverso una tabella delle pagine, che mappa gli indirizzi virtuali usati dalle applicazioni in indirizzi fisici.
    Tuttavia, in un ambiente virtualizzato, gli indirizzi fisici che vede una VM non sono gli indirizzi fisici reali della macchina fisica (host).
  • Shadow Page Tables: Il VMM crea delle shadow page tables che riflettono l'associazione tra gli indirizzi virtuali della VM e gli indirizzi fisici reali della
    macchina host.
      -> doppia indirezione e quindi doppia traduzione
Controllo del VMM:
  • Accesso read-only per i guest: I guest OS possono solo leggere le loro tabelle delle pagine, ma non possono modificarle direttamente.
  • Intervento del VMM per modifiche: Se il guest OS vuole aggiornare le tabelle delle pagine (ad esempio, mappare un nuovo indirizzo virtuale a uno fisico), non
    può farlo autonomamente. Deve richiedere al VMM, che controllerà la richiesta, la validerà e poi effettuerà la modifica nelle shadow page tables.

MEMORY SPLIT
  • lo spazio di indirizzamento virtuale per ogni VM è strutturato in modo da contenere xen e il kernel in segmenti separati.
  • Xen risiede nei primi 64 MB del virtual address space di ogni VM:
  • In questo modo solo il VMM ha accesso all'area di memoria di Xen (ring 0), solo xen e il kernel della VM ha accesso all'area kernel (ring 1) e lo spazio user viene 
    acceduto da tutti (ring 3)

CREAZIONE DI UN PROCESSO
Il SO guest richiede una nuova tabella delle pagine al VMM:
  • Alla tabella vengono aggiunte le pagine appartenenti al segmento di xen.
  • Xen registra la nuova tabella delle pagine e acquisisce il diritto di scrittura esclusiva.
  • ogni successiva update da parte del guest provocherà un protection-fault, la cui gestione comporterà la verifica e l’effettivo aggiornamento della PT.

BALOON PROCESS
La paginazione è a carico dei guest.
  -> Occorre un meccanismo efficiente che consenta al VMM di reclamare ed ottenere, in caso di necessità, dalle altre macchine virtuali pagine di memoria meno utilizzate.

Soluzione: su ogni macchina virtuale è in esecuzione un processo (balloon process) che comunica direttamente con il VMM, e che viene interpellato ogni volta che il
VMM ha bisogno di ottenere nuove pagine ad esempio per l’attivazione di una VM.

Funzionamento: In caso di necessità di pagine per una nuova VM, interviene il VMM per chiedere ad altre VM di liberare memoria.

Il VMM, comunicando direttamente con il baloon process di altre VM, chiede ad essi di “gonfiarsi”, cioè di richiedere al proprio s.o. (guest) altre pagine.
La richiesta del balloon process provoca da parte del S.O. guest l’allocazione di nuove pagine al balloon process, che, una volta ottenute, le cede quindi al VMM.

--- XEN | VIRTUALIZZAZIONE DELLA CPU
Il VMM definisce un ’architettura virtuale simile a quella del processore, nella quale, però, le istruzioni privilegiate sono sostituite da opportune hypercalls:
  • L’invocazione di una hypercall determina il passaggio da guest a xen (ring1 -> ring 0)
  • I kernel dei sistemi guest devono essere modificati di conseguenza

Il VMM si occupa dello scheduling delle macchine virtuali: Borrowed Virtual Time scheduling algorithm
  • Si basa sulla nozione di virtual-time
  • algoritmo general-purpose, che consente, in caso di vincoli temporali stringenti (es. applicazioni time dependent, TCP/IP, servizi RT..) di ottenere 
    schedulazioni efficienti
  • Due clock:
    - real-time (tempo del processore, inizia al boot)
    - virtual-time (associato alla VM, avanza solo quando la VM esegue)
    - I tempi vengono comunicati ai guest tramite eventi.
    
--- XEN | VIRTUALIZZAZIONE DELL' I/O
Soluzione adottata:
• Back-end driver: per ogni dispositivo, il suo driver è isolato all’interno di una particolare macchina virtuale (tipicamente Dom0). 
    -> questi domini hanno quindi accesso diretto all’HW.
• Front-end driver: ogni guest prevede un driver virtuale semplificato che consente l’accesso al device tramite il backend:
    - Pro: portabilità (v. migrazione), isolamento, semplificazione del VMM.
    - Contro: necessità di comunicazione con il back-end
