Dato un semaforo S, siano:
- vals: valore dell’intero non negativo associato al semaforo;
- Is: valore intero≥0 con cui il semaforo s viene inizializzato;
- nvs: numero di volte che l’operazione V(s) è stata eseguita;
- nps: numero di volte che l’operazione P(s) è stata **completata**.

Ad ogni istante dell’esecuzione possiamo esprimere il valore del semaforo come:
- vals = Is + nvs – nps

da cui (vals >= 0):
- nps ≤ Is + nvs -> **relazione di invarianza**