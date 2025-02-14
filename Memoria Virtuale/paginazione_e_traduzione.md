TERMINOLOGIA
virtual memory:     is what the program sees
phisical memory:    the physical RAM on the computer
virtual addresses:  indirizzi utilizzati dai programmi
physical addresses: indirizzi utilizzati dall'hardware per comunicare con la RAM 

### PROCESSO DI TRADUZIONE
come fa un programma ad accedere alla memoria fisica se vede solamente indirizzi virtuali?
- attraverso un processo di traduzione degli indirizzi

1. un programma specifica un determinato indirizzo in una delle sue istruzioni (ad es. load)
    - questo è un indirizzo virtuale
2. il SO traduce questo indirizzo virtuale nel corrispondente inidirizzo fisico facendo una lookup all'interno di una tabella 
    - indirizzo virtuale diventa un indice nella tabella di traduzione
3. (se l'indirizzo fisico punta al disco, la porzione di dati di interesse viene caricata in memoria mediante uno swap e la tabella
      viene aggiornata con la nuova locazione)
4. a questo punto il SO può accedere alla locazione di memoria fisica corretta in cui si trova il dato desiderato

### PAGE TABLES
La struttura dati che associa indirizzi virtuali a indirizzi fisici si chiama page table.

**PRIMA PROBLEMATICA | DIMENSIONE DELLA PAGE TABLE**
se consideriamo una struttura dati unica per questo mapping, ovvero un'unica struttura che contiene tutte le traduzioni di ogni
indirizzo virtuale, otterremmo una tabella con una entry per indirizzo virtuale 
- consideriamo un sistema con 2^32 indirizzi virtuali
- 4byte * 2^32 entry = 16GiB occupati in memoria solo per la tabella di traduzione!!!
- Come è possibile rendere questa tabella più maneggevole?

Soluzione, salvare nella tabella blocchi di indirizzi (pages) per entry al posto di singoli indirrizzi
- es. VA: 0-4095 -> PA: 4096-8191  -> pagine di 4kiB
- necessarie molte meno entry nella tabella per coprire tutto lo spazio di indirizzamento
    -> dim = 16GiB/4kiB = 4MiB di memoria occupata dalla tabella 

**NB**: abbiamo ridotto notevolmente la dimensione della tabella ma abbiamo anche perso di flessibilità. Adesso l'unità minima di memoria che possiamo gestire è la pagina invece del singolo indirizzo. Questo significa che, ad esempio, operazioni di swap da e verso il disco devono caricare intere pagine di memoria anche se si ha necessità di un solo indirizzo. Oppure non possiamo più tappare buchi in memoria grandi 1 solo indirizzo, dobbiamo per forza caricare in memoria pagine da 4kiB nella loro interezza di nuovo, anche nel caso in cui l'indirizzo di cui si ha neccessità è solo uno. 

**NB**: in generale più la dimensione delle pagine cresce più la dimensione della tabella diminuisce. Tuttavia allo stesso modo diminuisce anche la flessibilità nella gestione della memoria.

**SECONDA PROBLEMATICA | COME SI ACCEDE AD INDIRIZZI SPECIFICI DENTRO AD UNA PAGINA**
La gestione della memoria in pagine comporta anche che indirizzi fisici appartenenti alla stessa pagina diventano contigui in RAM.

Quindi, se la pagina virtuale [0-4095] è mappata sulla pagina fisica [4096-8191], questo significa che all'indirizzo virtuale 4
corrisponde l'indirizzo fisica 4100. 
- parte dell'indirizzo virtuale diventa un OFFSET per accedere alla corretta locazione FISICA DI MEMORIA  



### ADDRESS TRANSLATION
Assumiamo di avere una macchina con uno spazio di indirizzamento di 32bit, 256kiB di memoria fisica, e pagine da 4kiB. GLi indirizzi virtuali saranno grandi 32 bit, mentre l'offset all'interno di una pagina sarà composto da 12 bit (dimensione della pagina).

La traduzione da indirizzo virtuale a indirizzo fisico consiste in:
- tradurre i 20 most significant bit rimanenti da un indirizzo virtuale non considerando i bit di offset (VIRTUAL PAGE NUMBER), nel corrispettivo PHYSICAL PAGE NUMBER. 
    - l'indice della PAGE TABLE diventa il VIRTUAL PAGE NUMBER
    - lasciare inalterati i 12 bit di offset

### PAGINAZIONE A PIU LIVELLI
Per una macchina a 32 bit con pagine grandi 4kiB abbiamo bisogno di:
- 1M di Page table entries 
- Ogni entry è grande 4 byte (20 bit per physical page number + 12 di permission bits )
- in totale 4MiB di memoria fisica occupate per la page table

Non male! se non che ogni processo ha bisogno della sua page table...
- in una situazione tranquilla in cui ci 100 processi in esecuzione 400 MiB di memoria occupata dalle pagine!!
- inoltre, queste page tables non possono subire uno swap out verso il disco anche se non stanno venendo utilizzate, in quanto: se non sono in RAM non c'è più modo di accedervi e quindi non c'è più modo di trovarle...

Soluzione: aggiungere altri livelli di indirezione! 

1st Level Page Table
associa ad un virtual page table number un riferimento ad una entry 2st Level Page Table entry

2st Level Page Table entry
associa ad un virtual page table number un physical page table number

...traduzione più complicata...

NB: adesso c'è bisono di avere solo la 1st Level Page Table in memoria in quanto le tabelle di secondo livello posso anche essere
swappate sul disco e comunque la tabella di primo livello riesce a mantenere un riferimento.

