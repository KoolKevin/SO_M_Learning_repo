Che cosa succede se un programma richiede accesso ad una pagina fisica non presente in RAM?
- Innanzitutto, la page table entry punta ad una locazione sul disco
- La CPU genererà una PAGE FAULT EXCEPTION
- Questo attiva il page fault handler del sistema operativo che si occuperà dello swap in memoria della pagina desiderata:
    - Se la memoria è piena, il SO sceglie (con dei suoi algoritmi) una pagina a cui applicare uno swap out verso il disco
        - in particolare se la pagina da evictare è dirty (è stata modificata rispetto alla sua versione su disco) bisogna anche aggiornare la sua versione sul disco, altrimenti l'accesso al disco si può evitare
    - Successivamente il SO, legge la pagina desiderata dal disco e la carica in memoria
    - Per poi aggiornare la relativa page table entry con la nuova locazione in memoria della pagina
    - Infine, il SO salta indietro all'istruzione che ha generato la page fault exception che adesso puo accedere alla memoria
        che gli serviva e quindi può eseguire.

NB: questo è un processo molto lento siccome l'accesso al disco è molto più lento rispetto all'accesso in memoria. Tuttavia sempre meglio che fare crashare il programma perchè non c'è più spazio (anche se alcuni SO lo fanno -> OOM Killers)