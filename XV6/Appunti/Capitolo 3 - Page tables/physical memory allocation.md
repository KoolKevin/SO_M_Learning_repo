Anche qua, guarda il codice e fai prima...

#### Cosa interessante sulla gestione dei puntatori e dei cast
The allocator sometimes treats addresses as integers in order to perform arithmetic on them (e.g., traversing all pages in freerange ), and sometimes uses addresses as pointers to read and write memory (e.g., manipulating the run structure stored in each page); this dual use of addresses is the main reason that the allocator code is full of C type casts.

#### Cosa interessante sull'inizializzazione della memoria liberata
The function kfree (kernel/kalloc.c:47) begins by setting every byte in the memory being freed to the value 1. This will cause code that uses memory after freeing it (uses “dangling references”) to read garbage instead of the old valid contents; hopefully that will cause such code to break faster.
    - debug