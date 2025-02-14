**MANCANZA DI MEMORIA FISICA**
Lo spazio di indirizzamento di un processore è spesso più grande della memoria fisica disponibile. Oppure la memoria fisica è in gran parte occupata da altri programmi. Senza memoria virtuale queste difficolta mi impedirebbero di eseguire determinati programmi.

Con l'indirettezza che la memoria virtuale fornisce indirizzi virtuali possono puntare anche a locazioni sul disco risolvendo questo problema (seppur a scapito delle performance). 

**BUCHI NELLO SPAZIO DI INDIRIZZAMENTO**
Senza memoria virtuale, la memoria assegnata ad un programma dovrebbe essere salvata in modo contiguo. Questo tuttavia impedirebbe 
l'esecuzione di un programma nei casi in cui, nonostante ci sia abbastanza memoria libera, essa sia spezzettata nello spazio di indirizzamento. 

Abbiamo dello spazio libero frammentato lasciato in questo modo, ad esempio da programmi che hanno terminato la loro esecuzione.

Con la memoria virtuale questo requisito di contiguità della memoria libera sparisce.

**ISOLAMENTO**
Senza memoria virtuale, programmi diversi (che hanno accesso allo stesso spazio di indirizzamento) potrebbero accedere in maniera non coordinata alla stessa locazione di memoria.

Con la memoria virtuale gli spazi di indirizzamento vengono separati, ognuno vede il proprio.