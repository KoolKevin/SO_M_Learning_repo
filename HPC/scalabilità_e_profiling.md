## Obiettivi del calcolo parallelo
- Risolvere lo stesso problema in minor tempo
    - stessa dimensione dei dati
    - aumento del numero di processori
- Risolvere un problema più grande nello stesso tempo
    - più processori
    - più dati

### Metriche
- **Speedup**, *S(p, N) = T(1, N)/T(p, N)*
    - con N = la dimensione del problema
    - p il numero di processori utilizzati per il calcolo
    - misura il guadagno in termini di tempo di esecuzione derivante dalla parallelizzazione
    - **idealmente**: S(p, N) = p; ovvero, con p processori vado p volte più veloce *T(p,n) = 1/p*T(1, N)*
    - in realtà, **la parallelizzazione ha un costo**
        - creazione/allocazione processi
        - comunicazione/sincronizzazione
        - distribuzione non bilanciata del lavoro
        - ...
    - inoltre, la maggior parte delle volte **non tutto il problema è parallelizzabile** 
    - quindi, **nei casi reali**: S(p, N) < p
- **Efficienza**, *E = S/p*
    - **idealmente**: E = 1 (= (S=p)/p) 
    - **nei casi reali**: E è un valore tra [0, 1]
    - misura quanto è stato effettivamente utile incrementare il numero di processori rispetto allo speedup ottenuto
        - tipicamente l'efficienza (superata una certa soglia) decresce all'aumentare del numero di nodi in quanto la parte non parallelizzabile comincia a pesare sempre di più
        - se un programma mantiene la stessa efficienza al variare del numero di processori utilizzati, e/o al variare della quantità di dati da elaborare, si dice che è **idealmente scalabile**


### Legge di Amdahl
Definisce l'andamento **teorico** dello speedup all'aumentare del numero di nodi 

Abbiamo già osservato che non tutto il problema può essere parallelizzabile. Pertanto, distinguiamo:
- T(1,N) = T_seq(1,N)+ T_par(1,N)
Successivamente:
- T_seq(p, N) = T_seq(1, N) = T_seq
- r = T_seq/T(1, N); frazione del tempo totale spesa nella parte non parallelizzabile
- T_par(1, N) = (1-r)T(1, N)

... varie sostituzioni ...

```
Legge di Amdahl:    S(p, N) = 1 / (r+(1-r)/p)
```

Osservazioni:
- 1 == frazione di tempo spesa nel problema == r + (1-r)
- se r != 0 (parte non parallelizzabile) lo speedup è minore di p
- S(p, N) ha un **comportamento asintotico**
    - il limite per *p* che tende ad infinito converge asintoticamente a **_1/r_**; 
    - **c'è un limite massimo teorico alla crescita dello speedup** dovuto alla parte non parallelizzabile
    - aumentare il numero di nodi, diminuisce solo il tempo speso nella parte parallelizzabile

**Efficienza**:
```
Legge di Amdahl applicata all'efficienza: E = S(p, N)/p = 1 / p*(r+(1-r)/p) = 1 / pr+(1-r)
```

Anche **l'efficienza ha un comportamento asintotico**: al limite per *p* che tende ad infinito, **tende a 0**
- come accennato prima, all'aumentare del numero di nodi l'efficienza cala




## Scalabilità strong e Scalabilità weak
In generale, il concetto di scalabilità esprime per un programma parallelo la capacità di **mantenere invariata l’efficienza al variare del numero dei nodi e/o della dimensione del problema**.

Dato un problema:
- si può valutare quanto si può **guadagnare nella sua soluzione mantenendo costante N e aumentando il numero di processori**
    - **questo si studia con la legge di Amdahl** e calcolando il parametro r caratteristico del problema in considerazione 
        - abbiamo un limite teorico allo speedup
    - (scalabilità strong).
- oppure si può valutare se **aumentando la dimensione possiamo risolvere il problema nello stesso tempo**
    - (scalabilità weak).
    - **questo si studia con la legge di gustafson** (vedi dopo)

La **scalabilità strong** misura la variazione di efficienza e/o speedup **al variare del numero di processori**.
- nel caso ideale, al crescere di *p*, l’efficienza si mantiene costante.
- **il lavoro del singolo nodo diminuisce al crescere del numero dei nodi**

La **scalabilità weak** misura la variazione di efficienza e/o speedup al crescere del numero dei nodi **mantenendo costante il carico di lavoro per singolo nodo**.
- con 1 processore: dimensione N; con 2 processori: dimensione 2\*N; con p processori: dimensione p\*N
- la dimensione del problema aumenta in proporzione al numero p di nodi utilizzati.



### Legge di Gustafson
Dato un problema di dimensione *pN* consideriamo la sua soluzione parallela con *p* processori. Come prima: 
- abbiamo una frazione di tempo spesa nella parte non parallelizzabile: *T_seq = rT(p, pN)* 
    - con *r = T_seq / T(p, pN)*
- e la frazione complementare spesa nella parte parallelizzabile: *T_par = (1-r)T(p, pN)*
    - T(p, pN) = T_seq + T_par

**NB**: risolvendo lo stesso problema (lasciando invariata la dimensione dei dati) **con un solo processore** avremo che:
- la percentuale di tempo spesa nella parte parallelizzabile sarebbe *p* volte più grande rispetto a quella del caso con *p* processori
- T_par(1) = p*T_par(p)
    - ho da calcolare i dati di *p* processori con uno solo

Abbiamo quindi che il tempo per svolgere lo stesso lavoro di p processori con uno solo è:

    T(1, pN) = Tseq + pTpar = rT(p, pN) + p(1-r)T(p, pN) = T(p, pN) (r + p(1-r))

Lo speedup diventa quindi:

    Legge di Gustafson: S(p, pN) = T(1, pN) / T(p, pN) = r + p(1-r)

Osservazioni:
- al crescere di p non si ha più un comportamento asintotico, ma lineare!
    - **Ogni processore riceve un workload costante (1-r)**, lo speedup cresce linearmente con il numero dei processori
    - non c'è più un limite intrinseco del problema allo speedup

