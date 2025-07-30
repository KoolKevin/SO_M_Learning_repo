I major e minor number di un device file in sistemi Unix/Linux sono due numeri che identificano univocamente un dispositivo hardware o una sua parte, permettendo al kernel di interagire correttamente con esso tramite il file nel filesystem (tipicamente in /dev).

Definizione:
- **Major number**: identifica il driver del dispositivo. In pratica, dice al kernel quale modulo/driver usare per gestire le operazioni su quel dispositivo.
- **Minor number**: identifica un dispositivo specifico gestito da quel driver. Per esempio, due dischi gestiti dallo stesso driver avranno lo stesso major number ma minor diversi.