DOMANDA: QUANTO COSTA LA MEMORIA VIRTUALE?
Per ogni operazione di accesso alla memoria, abbiamo bisogno di:
- Accedere alla Page Table in memoria
- Tradurre l'indirizzo virtuale
- Accedere effettivamente il dato che ci interessa

Due accessi effettivi alla memoria per ogni accesso logico (costo di traduzione)
- costo elevato! Sopratutto considerando che in media ad ogni istruzione dell'ISA corrispondono 1.33 accessi logici alla memoria   

L'overhead è dovuto dal dover accedere prima anche alla Page Table (accesso in memoria aggiuntivo)
- come possiamo rendere una look-up all'interno della page table molto veloce?
    - cache per la page table!

### Translation Lookaside Buffer
Cache, inizialmente vuota, in cui vengono caricate/sostiuite le traduzioni indirizzi virtuale - indirizzo fisico a cui i programmi accedono (anche nel TLB, come nella memoria centrale, le entry (tipicamente quelle usate meno di recente) possono subire eviction se non si ha piu spazio).

Il processore, prima di andare a cercare nella page table in memoria, controlla il TLB e se trova li la traduzione desiderata, si ha un'accesso alla memoria molto veloce. Se non la trova si ha un accesso alla memoria lento.

Per essere veloci i TLB devono essere di piccole dimensioni, tipicamente 64 entries per pagine da 4kiB (Page Table ne contiene 1M!). Con una copertura così piccola, bisogna spendere una discreta quantità di tempo ad aggiornare il contenuto del TLB.

La ragione per cui questa rimane comunque una soluzione efficacie è data dal principio di località, siccome le pagine sono grandi 4kiB, fino a che il programma ha bisogno di accedere ad indirizzo di una stessa pagina non ci sarà bisogno di aggiornare il TLB. Questo in pratica avviene spesso proprio per il menzionato principio di località (i Miss rate sono una percentuale piccola).

