Dim matrice in input: 2000x2000
Dim matrice in output: 1000x1000

----------------------------------------------------
Num di processi paralleli: **1**
Num di righe del risultato calcolate da ogni singolo processo: 1000

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.001065 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 11.540365 ms per effettuare l'invio dei dati

**riduzione**:
[processo 0]: 43.548816 ms per effettuare il calcolo di 1000 righe del risultato

**gather**:
[processo 0]: 2.675585 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 57.765830 ms
----------------------- FINE -----------------------

Num di processi paralleli: **2 (1 nodo, 2 processi)**
Num di righe del risultato calcolate da ogni singolo processo: 500

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.025867 ms per preparare l'invio dei dati
[processo 1]: 0.001755 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 10.187820 ms per effettuare l'invio dei dati
[processo 1]: 84.517636 ms per effettuare l'invio dei dati 
- **NB**: i processi non master (rank!=0), dovendosi sincronizzare alla barriera, considerano anche il tempo di inizializzazione della matrice che non è da contare

**riduzione**:
[processo 0]: 22.404104 ms per effettuare il calcolo di 500 righe del risultato
[processo 1]: 22.404354 ms per effettuare il calcolo di 500 righe del risultato

**gather**:
[processo 0]: 3.526687 ms per recuperare i dati con la gather dai vari processi
[processo 1]: 3.526654 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 36.144476 ms
----------------------- FINE -----------------------

Num di processi paralleli: **10 (1 nodo, 10 processi)**
Num di righe del risultato calcolate da ogni singolo processo: 100

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 4.410464 ms per preparare l'invio dei dati
[processo 1]: 4.409862 ms per preparare l'invio dei dati
[processo 2]: 4.355190 ms per preparare l'invio dei dati
...

**scatterv**:
[processo 0]: 11.385890 ms per effettuare l'invio dei dati
[processo 1]: 86.282337 ms per effettuare l'invio dei dati
[processo 2]: 86.282853 ms per effettuare l'invio dei dati
...

**riduzione**:
[processo 0]: 4.565735 ms per effettuare il calcolo di 100 righe del risultato
[processo 1]: 4.566125 ms per effettuare il calcolo di 100 righe del risultato
[processo 2]: 4.565905 ms per effettuare il calcolo di 100 righe del risultato
...

**gather**:
[processo 0]: 3.400742 ms per recuperare i dati con la gather dai vari processi
[processo 1]: 3.400716 ms per recuperare i dati con la gather dai vari processi
[processo 2]: 3.400639 ms per recuperare i dati con la gather dai vari processi
...



Tempo impiegato: 23.762831 ms
----------------------- FINE -----------------------

Num di processi paralleli: **50 (5 nodi, 10 processi)**
Num di righe del risultato calcolate da ogni singolo processo: 20

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.403235 ms per preparare l'invio dei dati
[processo 40]: 0.410163 ms per preparare l'invio dei dati
[processo 30]: 0.403817 ms per preparare l'invio dei dati
...

**scatter**:
[processo 40]: 87.745068 ms per effettuare l'invio dei dati
[processo 30]: 87.742802 ms per effettuare l'invio dei dati
[processo 0]: 13.267328 ms per effettuare l'invio dei dati
...

**riduzione**:
[processo 40]: 0.930158 ms per effettuare il calcolo di 20 righe del risultato
[processo 30]: 0.930475 ms per effettuare il calcolo di 20 righe del risultato
[processo 20]: 0.930963 ms per effettuare il calcolo di 20 righe del risultato
...

**gather**:
[processo 0]: 3.546184 ms per recuperare i dati con la gather dai vari processi
[processo 20]: 3.550489 ms per recuperare i dati con la gather dai vari processi
[processo 42]: 3.546771 ms per recuperare i dati con la gather dai vari processi
...




Tempo impiegato: 18.641313 ms
----------------------- FINE -----------------------

