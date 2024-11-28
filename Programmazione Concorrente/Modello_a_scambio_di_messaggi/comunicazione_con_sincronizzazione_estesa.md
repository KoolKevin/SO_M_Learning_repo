Modello di comunicazione/sincronizzazione in cui:
- il processo mittente richiede l’esecuzione di un servizio al processo destinatario
- il processo mittente rimane sospeso fino al completamento del servizio richiesto.

**NB**: I processi rimangono sincronizzati durante l'esecuzione del servizio da parte del ricevente fino alla ricezione dei risultati da parte del mittente.

**OSS**: Analogia con una normale chiamata di funzione, solo che in questo caso, il servitore è un processo diverso.

## Due modalità di implementazione
Lato ricevente, possiamo distingure tra RPC (possibile necessità di gestione di corse critiche e sincronizzazione a carico del programmatore) e rendez-vouz esteso.

### chiamata di procedura remota (RPC -Remote Procedure Call)
Per ogni operazione che un processo client può richiedere viene dichiarata, lato server, una procedura;

**Differenza sostanziale**: il ricevente crea un nuovo thread ad ogni richiesta che ha il compito di eseguire la procedura corretta 
- concorrenza lato servitore
- presenza di sezioni critiche da gestire 

### rendez-vous esteso
Il processo servitore utilizza un'istruzione di input (accept) che lo sospende in attesa di una richiesta dell'operazione.

All'arrivo della richiesta il processo esegue il relativo insieme di istruzioni ed i risultati ottenuti sono inviati al chiamante.

**Differenza sostanziale**: lato servitore si ha una esecuzione sequenziale delle richieste


### RPC v. Rendez Vous esteso
- RPC rappresenta solo un meccanismo di comunicazione tra processi

- Rendez-vous esteso combina comunicazione con sincronizzazione. Il processo servitore si sincronizza con il processo cliente quando esegue l'operazione di **accept**