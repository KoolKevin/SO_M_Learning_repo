Come si può usare la memoria virtuale per far si che programmi diversi non corrompano la memoria l'uno dell'altro?

Innanzitutto ogni processo deve avere la sua PAGE TABLE personale, in quanto un'unica page table di sistema non permetterebbe di 
garantire isolamento
- ad esempio, virtual page number 0x00000 punterebbe alla stessa locazione  di memoria per tutti i processi;
- lo spazio di indirizzamento rimane difatti condiviso.

Con una page table personale al processo gli spazi di indirizzamento vengono separati 
-ad esempio:  virtual page number 0x00000 potrebbe puntare alla pagina fisica 0x0000A per il processo A mentre a 0x0000B per il processo B

Questo separazione garantisce isolamento ed evita che un processo possa accedere direttamente alla memoria di un altro processo.
- in page table diverse, mapping diversi per lo stesso indirizzo virtuale 
- stesso mapping se si vuole condividere memoria

Inoltre, una tabella delle pagine personale permette a ciascun processo di avere lo stesso intervallo di indirizzi virtuali, senza
che questo causi conflitti tra processi, poiché il sistema operativo traduce questi indirizzi virtuali in indirizzi fisici in maniera indipendente per ciascun processo.

### SPAZIO DI INIDIRIZZAMENTO DI UN PROCESSO IN LINUX
in una macchina a 32 bit, OGNI processo vede uno spazio di indirizzamento grande 4GiB che comprende le seguenti componenti. 
- L'ultimo GiB è riservato al kernel e non è accessibile ai programmi utente
- Successivamente si trova lo stack del processo, che cresce verso il basso
- Poi troviamo delle librerie, che potrebbero essere anche condivise
- Poi troviamo l'heap, che cresce verso l'alto
- Poi troviamo l'area Data, che contiene costanti, variabili globali, ecc.
- Infine abbiamo l'area Text, ovvero quella del codice del processo 
- (All'inizio dello spazio di indirizzamento troviamo anche 128MiB di spazio riservato per operazioni di I/O) 

con la memoria virtuale ogni processo mappa queste aree del loro spazio di indirizzamento in aree distinte (o non in caso di
condivisione). Ad esempio:
- L'area riservata al kernel viene condivisa tra tutti i processi (stesso mapping in memoria fisica)
- Stack, Text e Data di ogni programma invece, vengono mappati in aree fisiche diverse

NB: mediante questi mapping è facile condividere memoria, per esempio tutti i thread di un processo potrebbero avere accesso alla stessa heap.