--- DEFINIZIONI
    - Protezione: riguarda l’insieme delle TECNICHE PER REGOLAMENTARE L'ACCESSO degli utenti al sistema di elaborazione.
      La sicurezza impedisce accessi non autorizzati al sistema e i conseguenti tentativi dolosi di alterazione e distruzione dei dati. 
    - Sicurezza: insieme di attività volte a garantire il controllo dell’accesso alle risorse logiche e fisiche da parte degli utenti
      autorizzati all’uso di un sistema di elaborazione.
        -> definizione simile ma leggermente più ampia

SICUREZZA
Le tecnologie di sicurezza di un sistema informatico realizzano meccanismi per l’IDENTIFICAZIONE, l’AUTENTICAZIONE e l’AUTORIZZAZIONE
di utenti fidati.

PROTEZIONE
Per ogni utente identificato, autenticato e autorizzato è necessario stabilire:
    • quali siano le risorse alle quali può accedere
    • con quali operazioni può accedervi
Questo è stabilito dal sistema di protezione tramite le tecniche di CONTROLLO degli ACCESSI.

In un sistema il controllo degli accessi si esprime tramite la definizione di tre livelli concettuali:
    • modelli
    • politiche
    • meccanismi

--- MODELLI DI PROTEZIONE
Un modello di protezione definisce i soggetti, gli oggetti ai quali i soggetti hanno accesso, ed i diritti di accesso:
    • Oggetti costituiscono la parte passiva, cioè le risorse fisiche e logiche alle quali si può accedere e su cui si può operare. 
        -> Ad es: i file.
    • Soggetti rappresentano la parte attiva di un sistema, cioè le entità che possono richiedere l’accesso alle risorse.
        -> Ad es: gli utenti, o i processi che eseguono per conto degli utenti.
    • Diritti di accesso: sono le operazioni con le quali è possibile operare sugli oggetti.

NB: Un soggetto può avere diritti di accesso sia per gli oggetti che per altri soggetti (ad es. un soggetto può controllarne un altro) 

--- POLITICHE
Le politiche di protezione definiscono le REGOLE con le quali i soggetti possono accedere agli oggetti.

Classificazione delle politiche:
    • Discretional access control (DAC). Il soggetto creatore di un oggetto controlla i diritti di accesso per quell’oggetto (UNIX).
      La definizione delle politiche è decentralizzata -> stabilita dai singoli utenti
    • Mandatory access control (MAC). I diritti di accesso vengono definiti in modo centralizzato (i.e. amministratore di sistema).
      Installazioni di alta sicurezza (es., enti governativi).
    • Role Based Access Control (RBAC). Ad un ruolo sono assegnati specifici diritti di accesso sulle risorse. Gli utenti possono
      appartenere a diversi ruoli. I diritti attribuiti ad ogni ruolo vengono assegnati in modo centralizzato.

PRINCIPIO DEL PRIVILEGIO MINIMO
Ad ogni soggetto sono garantiti i diritti di accesso solo agli oggetti strettamente necessari per la sua esecuzione (POLA, principle of
least authority). Caratteristica desiderabile per tutte le politiche di protezione.

--- MECCANISMI
I meccanismi di protezione sono gli STRUMENTI messi a disposizione dal sistema di protezione per imporre una determinata politica.

Principi di realizzazione:
    • Flessibilità del sistema di protezione: i meccanismi di protezione devono essere sufficientemente generali per consentire
      l’applicazione di qualunque politica di protezione.
    • Separazione tra meccanismi e politiche. La politica definisce cosa va fatto ed il meccanismo come va fatto. E’ desiderabile
      la massima indipendenza tra le due componenti.
    
ESEMPIO UNIX
L’utente (politica DAC) definisce la politica, ovvero il valore dei bit di protezione per ogni oggetto di sua proprietà.

Il S.O. fornisce un meccanismo per definire e interpretare per ciascun file i tre bit di read, write e execute per il proprietario
del file, il gruppo e gli altri.

