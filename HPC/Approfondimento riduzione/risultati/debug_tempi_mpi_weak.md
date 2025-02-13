Dim matrice in input: 1600x1600
Dim matrice in output: 800x800
Num di processi paralleli: **1**
Num di righe del risultato calcolate da ogni singolo processo: 800

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.002111 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 9.779905 ms per effettuare l'invio dei dati

**riduzione**:
[processo 0]: 29.113345 ms per effettuare il calcolo di 800 righe del risultato

**gather**:
[processo 0]: 2.839006 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 41.734366 ms
----------------------- FINE -----------------------



Dim matrice in input: 3200x3200
Dim matrice in output: 1600x1600
Num di processi paralleli: **4**
Num di righe del risultato calcolate da ogni singolo processo: 400

**calcolo send_counts[] e input_offsets[]**:
[processo 0]: 0.058093 ms per preparare l'invio dei dati
[processo 1]: 0.047569 ms per preparare l'invio dei dati
[processo 2]: 0.057647 ms per preparare l'invio dei dati
[processo 3]: 0.006419 ms per preparare l'invio dei dati

**scatterv**:
[processo 0]: 23.880804 ms per effettuare l'invio dei dati
[processo 1]: 233.287815 ms per effettuare l'invio dei dati
[processo 2]: 233.286888 ms per effettuare l'invio dei dati
[processo 3]: 233.287750 ms per effettuare l'invio dei dati

**riduzione**:
[processo 0]: 29.299730 ms per effettuare il calcolo di 400 righe del risultato
[processo 1]: 29.299583 ms per effettuare il calcolo di 400 righe del risultato
[processo 2]: 29.299985 ms per effettuare il calcolo di 400 righe del risultato
[processo 3]: 29.299777 ms per effettuare il calcolo di 400 righe del risultato

**gather:**
[processo 0]: 12.361936 ms per recuperare i dati con la gather dai vari processi
[processo 1]: 12.362435 ms per recuperare i dati con la gather dai vari processi
[processo 2]: 12.362801 ms per recuperare i dati con la gather dai vari processi
[processo 3]: 12.362167 ms per recuperare i dati con la gather dai vari processi



Tempo impiegato: 66.369824 ms
----------------------- FINE -----------------------


Dim matrice in input: 6400x6400
Dim matrice in output: 3200x3200
Num di processi paralleli: **16**
Num di righe del risultato calcolate da ogni singolo processo: 200

[processo 12]: 0.059560 ms per preparare l'invio dei dati
[processo 0]: 0.054319 ms per preparare l'invio dei dati
[processo 4]: 0.056379 ms per preparare l'invio dei dati
...

[processo 0]: 133.433237 ms per effettuare l'invio dei dati
[processo 12]: 897.127412 ms per effettuare l'invio dei dati
[processo 4]: 897.119910 ms per effettuare l'invio dei dati
...

[processo 12]: 29.504459 ms per effettuare il calcolo di 200 righe del risultato
[processo 0]: 29.502799 ms per effettuare il calcolo di 200 righe del risultato
[processo 13]: 29.504174 ms per effettuare il calcolo di 200 righe del risultato
...

[processo 12]: 27.882498 ms per recuperare i dati con la gather dai vari processi
[processo 13]: 27.882272 ms per recuperare i dati con la gather dai vari processi
[processo 0]: 27.887023 ms per recuperare i dati con la gather dai vari processi
...



Tempo impiegato: 190.877378 ms
----------------------- FINE -----------------------


Dim matrice in input: 9576x9576
Dim matrice in output: 4788x4788
Num di processi paralleli: 36
Num di righe del risultato calcolate da ogni singolo processo: 133
[processo 0]: 0.852448 ms per preparare l'invio dei dati
[processo 1]: 0.852467 ms per preparare l'invio dei dati
[processo 2]: 0.849348 ms per preparare l'invio dei dati
...

[processo 0]: 349.349231 ms per effettuare l'invio dei dati
[processo 21]: 2128.635340 ms per effettuare l'invio dei dati
[processo 1]: 2128.632454 ms per effettuare l'invio dei dati
...

[processo 24]: 30.026786 ms per effettuare il calcolo di 133 righe del risultato
[processo 8]: 30.026393 ms per effettuare il calcolo di 133 righe del risultato
[processo 4]: 30.029083 ms per effettuare il calcolo di 133 righe del risultato
...

[processo 24]: 71.578478 ms per recuperare i dati con la gather dai vari processi
[processo 8]: 71.577390 ms per recuperare i dati con la gather dai vari processi
[processo 4]: 71.584107 ms per recuperare i dati con la gather dai vari processi
...



Tempo impiegato: 451.589588 ms
----------------------- FINE -----------------------
