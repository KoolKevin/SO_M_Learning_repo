## Realizzazione degli strumenti di comunicazione nel modello a scambio di messaggi | caso singolo nodo
le porte di un processo possono in generale essere organizzate in liste linkate. (nell'esempio viene usato poco)

... 

Nella receive, la funzione *test_porta* sospende il processo finchè qualcuno non inserisce almeno un messaggio sulla porta su cui vuole ricevere (PC si blocca li)

### Ricezione su più porte
aspettare su più porte contemporaneamente senza sospendersi e senza attesa attiva, abbiamo visto che è una esigenza comune
...
nella *receive_any* il ciclo serve perchè devo individuare la porta sul quale ricevere

## Caso di più nodi
Distinzione tra sistemi operativi distribuiti e sistemi operativi di rete:
- nel primo caso, esegue la stessa **istanza** di os su tutti i nodi; os omogeneo in tutti i nodi
    - trasparenza nell'interazione fornita dall'os
- nel secodo caso, ogni nodo è governato da un os in generale diverso e indipendente dagli altri
    - trasparenza nell'interazione fornita da un **middleware**

...

I canali dell'interfaccia di rete inviano degli interrupt nei casi di:
- arrivo di un pacchetto
- completamento dell'invio di un pacchetto 

...

I due canali di trasmissione possono essere occupati da attività di invio/ricezione riconducibili ad altri pacchetti rispetto a quello considerato.

Attivazione del canale tramite set di uno specifico registro di controllo della periferica

# BELLA NON CHIEDE:
Realizzazione “nativa” delle primitive sincrone: implementazione di send sincrona e receive come primitive di nucleo

potrebbe essere utile una realizzazione nativa delle primitive sincrone, in quanto con solo quelle asincrone si otterrebbero più chiamate di sistema per una operazione logica -> overhead.