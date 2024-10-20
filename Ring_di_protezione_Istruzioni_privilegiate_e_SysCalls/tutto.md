## Istruzioni Privilegiate
Ogni processore ha definito nella sua ISA un insieme di istruzioni che può eseguire. Per permettere a processi diversi di avere un grado di controllo differenziato sul sistema è possibile definire un sottoinsieme di queste istruzioni eseguibile solo da alcuni processi. Questo sottoinsieme prende il nome di insieme delle __istruzioni privilegiate__.

Le istruzioni privilegiate includono operazioni critiche come la gestione dell'hardware, l'accesso diretto alla memoria fisica e la gestione delle interruzioni, che devono essere eseguite con il massimo controllo per garantire la stabilità e la sicurezza del sistema.

Per implementare questa differenziazione, l'__hardware__ deve supportare più modalità di esecuzione, consentendo di separare le operazioni eseguite dai programmi utente da quelle eseguite dal sistema operativo. La modalità utente, ad esempio, non permette di eseguire istruzioni privilegiate, che sono invece riservate alla modalità kernel.

## Ring di Protezione
Per garantire una maggiore granularità nel controllo delle risorse e della sicurezza, le architetture hardware, implementano i ring di protezione. Questi ring sono dei livelli di privilegio gerarchici che delimitano l'accesso a istruzioni e risorse del sistema. Ogni ring ha un grado di privilegio differente, e le istruzioni privilegiate possono essere eseguite solo dai livelli più bassi (cioè quelli con più privilegi).

Nell'architettura x86-64, ci sono generalmente quattro ring, numerati da Ring 0 (massimo privilegio) a Ring 3 (minimo privilegio):
- Ring 0: È il livello con il massimo privilegio ed è riservato al kernel del sistema operativo. Qui si eseguono le istruzioni più critiche, come la gestione della memoria e dell'hardware.
- Ring 1 e Ring 2: Sono destinati a servizi di sistema, driver di dispositivi, e macchine virtuali che richiedono accesso parziale a risorse critiche ma non al livello del kernel. In pratica, questi livelli sono poco utilizzati nelle moderne implementazioni di sistemi operativi.
- Ring 3: È il livello con il minor privilegio ed è dove vengono eseguite le applicazioni in modalità utente. I programmi in esecuzione in Ring 3 non possono accedere direttamente all'hardware o alle istruzioni privilegiate.

In molte architetture moderne, i sistemi operativi utilizzano principalmente due livelli: Ring 0 per il kernel e Ring 3 per le applicazioni utente. Questa suddivisione garantisce che i processi in modalità utente non possano compromettere la sicurezza o la stabilità del sistema, lasciando al kernel la gestione di operazioni sensibili tramite le istruzioni privilegiate.

Grazie al sistema dei ring di protezione e alle diverse modalità di esecuzione, è possibile isolare i processi in modo sicuro e garantire che solo le parti più critiche del sistema operativo abbiano accesso completo alle risorse e alle istruzioni più pericolose, proteggendo così il sistema da errori o attacchi provenienti da applicazioni utente.

### Come si modifica il ring corrente?
Tipicamente si passa da user mode a kernel mode mediante un __interrupt__, o meglio una __trap__.

All'arrivo di determinate trap il ring corrente viene modificato, e si salta ad una nuova locazione di memoria contenente l'handler della trap che si occuperà della gestione della situazione che ha richiesto il cambio da user mode a kernel mode.

## Kernel-mode VS User-mode
Che cosa si può fare in kernel-mode che non si può fare in user-mode?
- interagire con dispositivi di I/O
    - manipolazione diretta di dispositivi come schede di rete, disco e periferiche varie richiede istruzioni privilegiate e quindi di kernel mode
- interagire con la Memory Management Unit per l'accesso alla memoria
    - pensa a separazione degli spazi di indirizzamento
- gestione degli interrupt/traps

In user-mode invece, i programmi possono eseguire tutte le istruzioni che hai visto a Calcolatori-T. Ad esempio load e store di dati dal loro spazio di indirizzamento, operazioni aritmetiche e logiche, branch, salti, ecc...

Tuttavia, i programmi in user-mode hanno comunque bisogno di fare cose come: leggere file, mostrare testo e inviare messaggi sulla rete. Queste operazioni però sono ristrette a kernel-mode.

    I programmi in user-mode hanno bisogno di accedere all'hardware, come fare? 

## System Calls
Per permettere ai programmi in user-mode di avere accesso all'hardware il sistema operativo offre l'interfaccia delle system call, un insieme di funzione che fornisce questo accesso ai programmi in user-mode in maniera controllata.

O meglio, direttamente da Tanenbaum: "To obtain services from the operating system, a user program must make a system call, which traps into the kernel and invokes the operating system. The trap instruction (e.g., __syscall__ on x86-64 processors) switches from user mode to kernel mode and starts the operating system. When the operating system is done, it returns control to the user program at the instruction following the system call".

Più nel dettaglio: "If a process is running a user program in user mode and needs a system service, such as reading data from a file, it has to execute a trap instruction to transfer control to the operating system. The operating system then figures out what the calling process wants by inspecting the parameters. Then it carries out the system call and returns control to the instruction following the system call".

    In a sense, making a system call is like making a special kind of procedure call—only system calls enter the kernel and procedure calls do not.

### Funzionamento delle system call

... preparazione della chiamata alla funzione di libreria che implementa la system call copiando i suoi parametri in appositi registri ...

Then comes the actual call to the library procedure (step 4). This instruction is the normal procedure-call instruction used to call all procedures. The library procedure, written in assembly language, typically puts the system-call number in a place where the operating system expects it, such as the RAX register (step 5). __Then it executes a trap instruction (such the X86-64 SYSCALL instruction) to switch from user mode to kernel mode and start execution at a fixed address within the kernel__ (step 6).

The kernel code that starts following the trap examines the system-call number in the RAX register and then dispatches to the correct system-call handler, usually via a table of pointers to system-call handlers indexed on system-call number (step 7). At that point, the system-call handler runs (step 8). 

Once it has completed its work, control may be returned to the user-space library procedure at the instruction following the trap instruction (step 9). This procedure then returns to the user program in the usual way procedure calls return (step 10), which then continues with the next instruction in the program (step 11).

In step 9 above, we said __‘'may be returned to the user-space library procedure’’__ for good reason. The system call may block the caller, preventing it from continuing. For example, if it is trying to read from the keyboard and nothing has been typed yet, the caller has to be blocked. In this case, the operating system will look around to see if some other process can be run next. Later, when the desired input is available, this process will get the attention of the system and run steps 9 and 10.

__OSS__: Le system call sono una potente astrazione che fornisce una interfaccia semplice ed efficace verso l'hardware ma hanno anche un costo riguardante il cambio di contesto tra User-mode e Kernel-mode. 