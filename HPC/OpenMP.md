OpenMP = Open specifications for Multi-Processing

Libreria per programmazione parallela su sistemi shared memory

...

## Direttive # pragma omp

### Creazione di thread
- \# pragma omp **parallel** [num thread] [altre eventuali clausole] { ... }
    - Il blocco di istruzioni che segue la direttiva viene eseguito da un insieme di **thread paralleli**.
    - In assenza di ulteriori indicazioni il numero di thread viene deciso dal sistema. Di solito: 1 thread/core disponibile
    - Modello cobegin-coend puro
    - sincronizzazione implicita: al termine del blocco parallelo il processo master attende che tutti i workers siano terminati.

### Scope delle variabili
Default:
- ogni variabile definita esternamente a # pragma omp parallel è condivisa tra i thread paralleli
- ogni variabile definita internamente al blocco parallelo è locale al singolo thread
- C’è la possibilità di specificare diversamente dal default con le clausole **_shared_**, **_private_**, **_firstprivate_**

### Mutua esclusione
- \# pragma omp **critical** \<sezione critica\>


... varie clausole

### Parallelizzazione di cicli

clausola for molto importante

...

### Distribuzione del carico