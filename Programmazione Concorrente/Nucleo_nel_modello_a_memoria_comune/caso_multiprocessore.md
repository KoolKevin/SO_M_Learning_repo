U_n = unità di elaborazione indipendenti
    M_n     = memoria privata
    I/O_n   = ?
 
### Che cosa cambia nell'organizzazione del sistema operativo nel caso multiprocessore

E’ possibile che processi che eseguono su CPU diverse richiedano contemporaneamente funzioni del nucleo (es. System Call): poichè, in generale,
ogni funzione comporta un accesso alle strutture dati interne al nucleo, occorre fare in modo che gli accessi al nucleo avvengano in modo sincronizzato:

    competizione tra CPU nell'esecuzione delle funzionalità del nucleo → necessità di sincronizzazione

...
### modello a più lock
...
Ogni struttura dati individua una classe di sezione critiche distinta -> associazione ad ogni struttura dati di un proprio lock.

...

piuttosto che politiche di scheduling globali (i.e. bilanciamento del carico tra core) si preferisce uno scheduling "locale" per vari motivi

### Aggiungi schema prof