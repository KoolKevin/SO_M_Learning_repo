Metriche:
- speedup
- efficienza

...

### Legge di Amdahl
...

r = frazione del tempo di esecuzione totale del programma spesa nella parte NON parallelizzabile

... varie sostituzioni per arrivare alla legge di Amdahl

Osservazioni:
- se r!=0 lo speedup è minore di p
- il limite con p che tende ad infinito converge asintoticamente a 1/r; c'è un limite alla crescita dello speedup

...

### Legge di Gustafson
Dato un problema:
- si può valutare quanto si può guadagnare nella sua soluzione mantenendo costante N e aumentando il numero di processori
    - questo si studia con la legge di Amdahl e calcolando il parametro r caratteristico del problema in considerazione 
    - (scalabilità strong)
- oppure si può valutare se aumentando la dimensione possiamo risolvere il problema nello stesso tempo
    - (scalabilità weak)

...

Osservazioni:
- al crescere di p non si ha più un comportamento asintotico, ma lineare!

