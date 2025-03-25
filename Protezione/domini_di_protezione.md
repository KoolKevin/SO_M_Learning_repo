--- DOMINI DI PROTEZIONE
Ad ogni SOGGETTO è associato un SINGOLO DOMINIO, che rappresenta l’ambiente di protezione nel quale il soggetto esegue; il dominio
specifica i diritti di accesso posseduti dal soggetto (es: utente) nei confronti di OGNI risorsa.

Le operazioni vengono svolte da PROCESSI che operano per conto di soggetti, a cui sono associati i domini.

Un dominio di protezione è UNICO per ogni SOGGETTO, mentre un PROCESSO può eventualmente CAMBIARE DOMINIO durante la sua esecuzione.

Ad esempio:
    • un processo P può eseguire inizialmente in un dominio di protezione D_i per conto del soggetto S_i 
    • in una fase di tempo successiva, P può eseguire in un altro dominio D_j per conto del soggetto S_j

DEF: Un dominio definisce un insieme di coppie, ognuna contenente l’identificatore di un oggetto e l’insieme delle operazioni che il
soggetto associato al dominio può eseguire su ciascun oggetto (diritti di accesso):
    D(S) = {<o, diritti> | o è un oggetto, diritti è un insieme di operazioni}
Ogni dominio è associato univocamente ad un soggetto; il soggetto può accedere solo agli oggetti definiti nel suo dominio, utilizzando
i diritti specificati dal dominio.

è anche possibile che due o più soggetti effettuino alcune operazioni comuni su un oggetto condiviso:
    D1                          D2                          D3
    <File1, (read, write)>      <File1, (execute)>          <File2, (read)>
    <File3, (execute) >         <File2, (read, write)>      <File3, (read)>

--- ASSOCIAZIONE TRA PROCESSO E DOMINIO
Di due tipi:

• Statica: l’insieme delle risorse disponibili ad un processo rimane fisso durante il suo tempo di vita (unico dominio per tutto il ciclo di 
  vita del processo).
    - Osservazioni:
    -> L’insieme globale delle risorse che un processo potrà usare può non essere un’informazione disponibile prima dell’esecuzione
       del processo.
    -> L’insieme minimo (politica del minimo privilegio) delle risorse necessarie ad un processo cambia dinamicamente durante
       l’esecuzione.
    -> L’associazione statica non è adatta nel caso si voglia limitare per un processo l’uso delle risorse a quello strettamente
       necessario (privilegio minimo).

• Dinamica: associazione tra processo e dominio varia durante l’esecuzione del processo.
    - In questo modo si può mettere in pratica il principio del privilegio minimo: cambiando dinamicamente dominio, in ogni fase
      della sua esecuzione il processo acquisisce diritti diversi (privilegio minimo: solo quelli strettamente necessari).

    -> Occorre un meccanismo per consentire il passaggio da un dominio all’altro del processo!

es unix:
Dominio associato all’utente: ogni processo è caratterizzato dall’attributo UserID (UID). Il cambio di dominio corrisponde al cambio
temporaneo di identità (UID) del processo.

--- MATRICE DEGLI ACCESSI
Un sistema di protezione può essere rappresentato a livello astratto utilizzando la matrice degli accessi.
  • Ogni colonna è associata a un oggetto (es: risorsa, file)
  • Ogni riga è associata a un soggetto (es: utente) 
  • Ogni cella contiene l'insieme dei diritti di accesso (es: read, write)
    -> riga=dominio

La matrice consente di rappresentare il MODELLO e le POLITICHE di protezione nel sistema considerato, specificando:
  • i soggetti
  • gli oggetti
  • i diritti accordati ai soggetti sugli oggetti
La matrice degli accessi offre ai meccanismi di protezione le informazioni che consentono di verificare il rispetto dei vincoli di
accesso.