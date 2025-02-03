Dim matrice in input: 200x200
Dim matrice in output: 100x100
Num di processi paralleli: **1**
Num di righe del risultato calcolate da ogni singolo processo: 100

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.001065 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 0.49 ms per effettuare l'invio dei dati

**riduzione**:
[processo 0]: 0.45 ms per effettuare il calcolo di 100 righe del risultato

**gather**:
[processo 0]: 0.44 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 1.399420 ms
----------------------- FINE -----------------------

Dim matrice in input: 400x400
Dim matrice in output: 200x200
Num di processi paralleli: **2**
Num di righe del risultato calcolate da ogni singolo processo: 100

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.004036 ms per preparare l'invio dei dati
[processo 1]: 0.002798 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 1.848752 ms per effettuare l'invio dei dati
[processo 1]: 5.110663 ms per effettuare l'invio dei dati   // aspetta l'inizializzazione della matrice

**riduzione**:
[processo 0]: 0.908334 ms per effettuare il calcolo di 100 righe del risultato  // adesso lunghe 200 e non 100
[processo 1]: 0.908308 ms per effettuare il calcolo di 100 righe del risultato

**gather**:
[processo 0]: 0.606147 ms per recuperare i dati con la gather dai vari processi
[processo 1]: 0.606025 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 3.367268 ms
----------------------- FINE -----------------------

Dim matrice in input: 2000x2000
Dim matrice in output: 1000x1000
Num di processi paralleli: **10**
Num di righe del risultato calcolate da ogni singolo processo: 100

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.042334 ms per preparare l'invio dei dati
[processo 1]: 0.008805 ms per preparare l'invio dei dati
[processo 2]: 0.009844 ms per preparare l'invio dei dati
...

**scatterv**:
[processo 0]: 11.612857 ms per effettuare l'invio dei dati
[processo 1]: 85.613186 ms per effettuare l'invio dei dati
[processo 2]: 85.613870 ms per effettuare l'invio dei dati
...

**riduzione**:
[processo 0]: 4.552120 ms per effettuare il calcolo di 100 righe del risultato  // adesso lunghe 1000
[processo 1]: 4.552204 ms per effettuare il calcolo di 100 righe del risultato
[processo 2]: 4.552088 ms per effettuare il calcolo di 100 righe del risultato
...

**gather**:
[processo 0]: 4.717248 ms per recuperare i dati con la gather dai vari processi
[processo 1]: 4.717213 ms per recuperare i dati con la gather dai vari processi
[processo 2]: 4.717408 ms per recuperare i dati con la gather dai vari processi
...



Tempo impiegato: 20.924559 ms
----------------------- FINE -----------------------

Dim matrice in input: 10000x10000
Dim matrice in output: 5000x5000
Num di processi paralleli: **50**
Num di righe del risultato calcolate da ogni singolo processo: 100

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.605844 ms per preparare l'invio dei dati
[processo 1]: 0.607676 ms per preparare l'invio dei dati
[processo 2]: 0.602126 ms per preparare l'invio dei dati
...


**Scatterv**:
[processo 0]: 265.113923 ms per effettuare l'invio dei dati
[processo 20]: 2103.091441 ms per effettuare l'invio dei dati
[processo 30]: 2103.089538 ms per effettuare l'invio dei dati
...

**Riduzione**:
[processo 20]: 22.777933 ms per effettuare il calcolo di 100 righe del risultato
[processo 30]: 22.778172 ms per effettuare il calcolo di 100 righe del risultato
[processo 0]: 22.780310 ms per effettuare il calcolo di 100 righe del risultato
...

**Gather**:
[processo 0]: 64.679780 ms per recuperare i dati con la gather dai vari processi
[processo 20]: 64.679033 ms per recuperare i dati con la gather dai vari processi
[processo 30]: 64.678350 ms per recuperare i dati con la gather dai vari processi
...




Tempo impiegato: 353.179857 ms
----------------------- FINE -----------------------
