## Protezione nei sistemi operativi

### Sicurezza & Protezione
Sicurezza:
- riguarda utenti non autenticati, ...
- le tecnologie di sicurezza realizzano i meccanismi di identificazione (chi sei? username), autenticazione (dimostramelo, password) e autorizzazione (cosa puoi fare) di utenti
- questi meccanismi impediscono accessi non autorizzati al sistema

Protezione:
- meccanismi che riguardano gli utenti identificati, autenticati e autorizzati, che definiscono:
    - quali siano le risorse alle quali l'utente può accedere
    - con quali operazioni può accedervi
- questo è stabilito dal sistema di protezione tramite le tecniche di **controllo degli accessi (RWX)**


### Politiche di protezione
Le politiche di protezione definiscono le regole con le quali i soggetti possono accedere agli oggetti.

Classificazione delle politiche:
- **Discretional access control (DAC)**
    - Il soggetto creatore di un oggetto controlla i diritti di accesso per quell’oggetto (UNIX).
    - La definizione dei diritti di accesso è decentralizzata.
- **Mandatory access control (MAC)**
    - I diritti di accesso vengono definiti in modo centralizzato (amministratore di sistema).
    - Installazioni di alta sicurezza (es., enti governativi).
- **Role Based Access Control (RBAC)**
    - Ad un ruolo sono assegnati specifici diritti di accesso sulle risorse.
    - Gli utenti possono appartenere a diversi ruoli.
    - I diritti attribuiti ad ogni ruolo vengono assegnati in modo centralizzato.

**principio del privilegio minimo**:
Caratteristica desiderabile per tutte le politiche di protezione è che: ad ogni soggetto vengano garantiti i diritti di accesso solo agli oggetti strettamente necessari per la sua esecuzione (POLA principle of least authority).

### Domini di protezione
Ad ogni soggetto è associato un dominio che specifica i diritti di accesso posseduti dal soggetto (es: utente) nei confronti di ogni risorsa (e quindi specifica anche a quali risorse può accedere).
-  Un dominio definisce un insieme di coppie (una per ogni oggetto del sistema), ognuna contenente l’identificatore di un oggetto e l’insieme delle operazioni che il soggetto associato al dominio può eseguire su ciascun oggetto (diritti di accesso)
    - dominio = {(OBJ1 : diritti), (OBJ2: diritti), ...}

Le operazioni vengono svolte da **processi che operano per conto di soggetti** (a cui sono associati i domini). Un dominio di protezione è unico per ogni soggetto, mentre un processo può eventualmente cambiare dominio durante la sua esecuzione.

### Matrice degli accessi
Un sistema di protezione può essere rappresentato a livello astratto utilizzando la matrice degli accessi.
- Ogni colonna è associata a un oggetto (es: risorsa, file)
- Ogni riga è associata a un soggetto (es: utente);
    - riga=dominio
- ogni cella specifica i diritti di accesso ad una determinata risorsa da parte di un determinato utente

**NB**: I diritti di accesso contenuti nella matrice possono cambiare nel tempo  per effetto di operazioni che ne consentono la modifica. (es: aggiunta/rimozione oggetti, aggiunta/rimozione soggetti, aggiunta/rimozione diritti).
- sono i meccanismi di protezione che consentono la modifica dello stato di protezione (diritti di accesso sugli oggetti) in seguito ad ogni richiesta autorizzata da parte di un processo
- la modifica dello stato di protezione si ottiene operando sulla matrice come descritto sopra (aggiungendo/togliendo righe/colonne e/o modificando celle)
- NB: teoricamente, anche la matriche degli accessi stessa è un oggetto e come tale è rappresentato da una colonna dentro se stessa

### Realizzazione della matrice degli accessi
La Matrice degli accessi è una notazione astratta che rappresenta lo stato di protezione. Nella rappresentazione concreta è necessario considerare elevata dimensione della matrice
- Rappresentare le informazioni contenute nella matrice degli accessi in una struttura dati matriciale Ns x No risulterebbe non ottimale per l’occupazione della memoria.

La Rappresentazione concreta dello stato di protezione deve essere ottimizzata sia riguardo all’occupazione della memoria, sia rispetto all’efficienza nell’accesso delle informazioni di protezione.

Due approcci:

**Access Control List (ACL):**
per **ogni oggetto è associata una lista che contiene tutti i soggetti** che possono accedere all’oggetto, con i relativi diritti di accesso per l’oggetto.
- Rappresentazione per colonne 
- ogni nodo della lista di un determinato oggetto è una coppia <soggetto, insieme dei diritti>
    - il soggetto può essere il singolo utente oppure la coppia <UID, GID> per ogni gruppo a cui utente appartiene
    - i diritti del gruppo si aggiungono a quelli dell'utente

**Capability List:**
ad **ogni soggetto è associata una lista che contiene gli oggetti accessibili** dal soggetto ed i relativi diritti di accesso
- rappresentazione per righe 
- ogni nodo della lista di un determinato soggetto è una coppia <oggetto, diritti>

**NB**: ACL e CL sono strutture dati del kernel e in quanto tali vanno accedute solamente mediante system call

### Soluzione mista (ACL e CL combinate)
Un sistema di protezione realizzato esclusivamente con ACL o capability list può presentare alcuni problemi di efficienza:
- ACL: L’informazione di **quali diritti di accesso possieda un soggetto S è sparsa** nelle varie ACL relative agli oggetti del sistema.
- Capability list: L’informazione relativa a **tutti i diritti di accesso applicabili ad un certo oggetto O è sparsa** nelle varie CL.

esempi:
- In un sistema di protezione può essere necessario revocare i diritti di accesso per un oggetto.
    - ACL: semplice, Si fa riferimento alla ACL associata all’oggetto e si cancellano i diritti di accesso che si vogliono revocare.
        - accesso ad una singola lista
    - CL: complesso, è necessario verificare per ogni soggetto se contiene la capability con riferimento all’oggetto considerato
        - accesso alle liste di tutti gli utenti

- In un sistema multi-user l’amministratore può applicare modifiche all’insieme degli utenti autorizzati.
    - ACL: la cancellazione di U è complessa. E’ necessario eliminare da ogni ACL gli eventuali elementi associati a U.
        - accesso all liste di tutti gli oggetti
    - CL: è sufficiente eliminare la CL associata all’utente eliminato.
        - accesso ad una singola lista

**conclusione**: ACL è più conveniente nelle operazioni che riguardano singoli oggetti (tutte le informazioni di protezione di un oggetto sono contenute in una singola lista), mentre il metodo delle Capability List è vantaggioso quando si devono compiere azioni sui singoli soggetti (tutte le informazioni di protezione del soggetto sono contenute in una singola lista).

**Soluzione Mista**: La soluzione che viene adottata nella maggior parte dei sistemi è di usare una combinazione dei due metodi.


### Sicurezza multilivello
La maggior parte dei sistemi operativi permette a singoli utenti di determinare chi possa leggere e scrivere i loro file ed i loro oggetti (DAC). In alcuni ambiti è richiesto un più stretto controllo sulle regole di accesso alle risorse (es. ambiente militare, aziende, ospedali, ecc.). L’organizzazione a cui appartiene il sistema definisce delle politiche MAC che stabiliscono regole **generali** su chi può accedere e a che cosa, tramite l’adozione di un modello di sicurezza.
- Tali regole si aggiungono (multilivello) alle politiche di protezione (DAC).

I modelli di sicurezza multilivello più usati sono due:

### Modello Bell-La Padula
Progettato per garantire la confidenzialità delle informazioni (non l'integrità):
- Quattro Livelli di sensibilità degli oggetti (o documenti):
- Quattro Livelli di autorizzazione (clearance) per i soggetti:

Associa a un sistema di protezione (matrice degli accessi) due regole di **sicurezza** (MAC), che stabiliscono il flusso di propagazione delle informazioni tra i livelli nel sistema.
1. un processo in esecuzione al livello di sicurezza k può leggere solo oggetti al suo livello o a livelli inferiori.
2. un processo in esecuzione al livello di sicurezza k può scrivere solamente oggetti al suo livello o a quelli superiori

Pertanto i processi possono leggere verso il basso e scrivere verso l’alto, ma non il contrario.
- **Il flusso delle informazioni è dal basso verso l’alto e non viceversa**.
- queste regole si aggiungono le regole di protezione (DAC) specificate dalla matrice degli accessi.

**NB**: Il modello Bell-La Padula è stato concepito per mantenere i segreti, non per garantire l’integrità dei dati. E’ possibile, infatti, sovrascrivere l’informazione appartenente ad un livello superiore. Si presuppono che con la matrice degli accessi questo problema venga mitigato


**esempio cavallo di troia**
Regole di protezione (es. ACL):
- Marco ha permessi di lettura e scrittura per il suo file Fm.
- Marco dà a Paolo il permesso di scrittura su Fm.
- Marco concede a Paolo il diritto di esecuzione su CT
- Il file Fp (contenente i dati da proteggere) è leggibile solo da Paolo.

Marco induce Paolo ad eseguire il cavallo di Troia CT per esempio, spacciandolo come un programma di utilità
- il processo creato varà l'identità di paolo e quindi potrà leggere Fp

Il programma CT, eseguito da Paolo tramite il processo PCT copia la stringa dei caratteri riservati nel file “tasca posteriore” Fm di Marco
- sia l’operazione di lettura che quella di scrittura soddisfano i vincoli imposti dalle ACL
- riservatezza violata

**difesa**
Vengono fissati 2 livelli di sicurezza, riservato e pubblico:
- Ai processi ed al file Fp di Paolo viene assegnato il livello di sicurezza “riservato”.
- A quelli di Marco (Fm e CT) il livello “pubblico”.

Quando Paolo esegue CT, il processo Pct creato acquisisce il livello di sicurezza di Paolo (riservato) e può vedere la stringa di caratteri riservata S.

Quando Pct tenta di scrivere S nel file Fm pubblico (la «tasca posteriore») la proprietà sulle scritture verso il basso è violata ed il tentativo viene negato dal sistema, nonostante l’ACL lo consenta

### Modello Biba.
Il modello Bell-La Padula è stato concepito per mantenere i segreti, non per garantire l’integrità dei dati.

Modello Biba
- Obiettivo: integrità dei dati.
- in sostanza uguale a BLP ma con flusso dei dati invertito

Proprietà:
1. un processo in esecuzione al livello di sicurezza k può scrivere solamente oggetti al suo livello o a quelli inferiori (nessuna scrittura verso l’alto).
2. un processo in esecuzione al livello k può leggere solo oggetti al suo livello o a quelli superiori (nessuna lettura verso il basso)

**NB**:
- Chiaramente, con questo modello siamo suscettibili all'attacco con cavallo di troia precedente
- B-LP vs. BIBA: I due modelli sono in conflitto tra loro e non si possono utilizzare contemporaneamente.



## Sistemi fidati
sistemi per i quali è possibile definire formalmente dei requisiti di sicurezza. In sostanza si differenziano rispetto ad un sistema normale per la presenza di:
- **Reference monitor**:
    - E’ un elemento di controllo realizzato dall’hardware e dal SO che regola l’accesso dei soggetti agli oggetti sulla base di parametri di sicurezza (es. modello Bell-La Padula).
- **Trusted computing base**:
    - base di dati fidata accedibile dal RM che contiene:
        - Privilegi di sicurezza (autorizzazioni di sicurezza) di ogni soggetto.
        - Attributi (classificazione rispetto alla sicurezza) di ciascun oggetto.

Il RM impone le regole di sicurezza (B-LP: no read-up, no-write down) ed ha le seguenti proprietà:
- **Mediazione completa**: le regole di sicurezza vengono applicate ad ogni accesso e non solo, ad esempio, quando viene aperto un file.
- **Isolamento**: il monitor dei riferimenti e la base di dati sono protetti rispetto a modifiche non autorizzate
- **Verificabilità**: la correttezza del RM deve esser provata, cioè deve esser possibile dimostrare formalmente che il monitor impone le regole di sicurezza ed fornisce mediazione completa ed isolamento