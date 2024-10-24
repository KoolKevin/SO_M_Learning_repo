Il nucleo è il solo modulo che è conscio dell'esistenza delle interruzioni!

quando viene attivata una interruzione essa viene gestita dal kernel e non viene mai propagata in user space.
    -> interruzioni invisibili ai processi user

Obiettivo di questo capitolo: realizzazione dei processi e della loro sincronizzazione

distinguiamo tra processo attivo: non c'è niente che blocca la sua esecuzione, e processo bloccato: il processo è in attesa di un evento per proseguire.

compiti fondamentale del nucleo:
- gestione delle transizioni di stato dei processo mediante salvataggio e ripristino del contesto.
...