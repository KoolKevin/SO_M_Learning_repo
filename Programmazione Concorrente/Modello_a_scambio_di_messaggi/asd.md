alternativa rispetto al modello a memoria comune nel modellare l'interazione tra processi in un sistema operativo concorrente.

... vari richiami

comunicazione asincrona
e comunicazione sincrona
e rendez-vous (sincronizzazione estesa)
in ordine di paralellismo permesso

la semantica asincrona è quella più granulare, permette di implementare tutti gli altri tipi di semantica. Non è vero il contrario.

...

receive bloccante e non bloccante 
bloccante è il default nella maggior parte dei casi

### Meccanismo di ricezione ideale | comando con guardia
una istruzione che i linguaggi di programmazione basati sul modello a scambio di messaggi prevedono.

...

NB: non determinismo nell'ordine di esecuzione dei rami con guardia valida. In altri termini, non esistono servizi/rami privilegiati

per uscire dal _do_ bisogna prevedere delle condizioni per ogni ramo 

### Primitive di comunicazione asincrone
Premessa: modello a scambio di messaggi con send asincrona

