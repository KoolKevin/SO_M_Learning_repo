## Caratterisitche generali dei processi in xv6
- An xv6 process consists of __user-space memory__ (text, data (comprende anche heap), and stack) and __per-process state private to the kernel__.
    - la convenzione in questo libro sembra quella di includere __nell'area data anche la heap__ e non sono le variabili globali/statiche.
- Xv6 time-shares processes: it transparently switches __all__ the available CPUs among the set of processes waiting to execute.
- When a process is not executing, xv6 saves the process’s CPU registers, restoring them when it next runs the process.
- The kernel associates a process identifier, or PID, with each process.

### Fork 
A process may create a new process using the fork system call. __fork gives the new process an exact copy of the calling process’s memory__: it copies the instructions, data, and stack of the calling process into the new process’s memory. fork returns in both the original and new processes. In the original process, fork returns the new process’s PID. In the new process, fork returns zero. The original and new processes are often called the parent and child.

Although the child has the same memory contents as the parent initially, __the parent and child are executing with separate memory and separate registers__: changing a variable in one does not affect the other. 

### Exec 
The exec system call replaces the calling process’s memory with a new memory image (quella che sopra viene chiamata user space memory, il kernel state rimane uguale) loaded from a file stored in the file system. The file must have a particular format, which specifies which part of the file holds instructions, which part is data, at which instruction to start, etc. __Xv6 uses the ELF format__. Usually the file is the result of compiling a program’s source code.

When exec succeeds, it does not return to the calling program; instead, the instructions loaded from the file start executing at the entry point declared in the ELF header.

    exec, quando ha successo, è quasi come se sostituiesse il processo corrente con uno nuovo che esegue il programma specificato come se si fosse invocato quest'ultimo da linea di comando.
    
    In verità, non c'è creazione di un nuovo processo, ma l'immagine in memoria del processo chiamante viene sostituita con quella del programma desiderato. Rimangono poi invariati: PID, Ambiente e File aperti.

You might wonder why fork and exec are not combined in a single call (qualcosa come runNewProcess()); we will see later that the shell exploits the separation in its implementation of I/O redirection (dopo exec mantengo gli stessi file aperti!).

__NB__: __Xv6 allocates most user-space memory implicitly__: fork allocates the memory required for the child’s copy of the parent’s memory, and exec allocates enough memory to hold the executable file.

A process that needs more memory at run-time (perhaps for malloc) can call sbrk(n) to grow its data memory by n zero bytes; sbrk returns the location of the new memory.

__Curiosità__: sbrk sta per "set break", ed è una chiamata di sistema storica che permette a un processo di modificare il limite (chiamato program break) della propria area di memoria dedicata ai dati. Più precisamente, sbrk è usata per espandere o ridurre l'area di memoria destinata all'heap, che è la zona in cui vengono allocate dinamicamente variabili e oggetti durante l'esecuzione (ad esempio tramite malloc in C).

Quando un processo chiama sbrk(n), il kernel incrementa il "program break" di n byte, cioè estende (o riduce, se n è negativo) l'area di memoria disponibile al processo. La memoria aggiuntiva viene inizializzata a zero, e sbrk restituisce il puntatore all'inizio della nuova area di memoria allocata.