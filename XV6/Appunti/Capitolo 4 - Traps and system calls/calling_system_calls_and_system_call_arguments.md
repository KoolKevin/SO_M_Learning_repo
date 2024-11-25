## Calling system calls
... quello che sai gia ...

**OSS**: When *sys_exec* returns, *syscall* records its return value in *p->trapframe->a0*. This will cause the original user-space call to exec() to return that value, since the **C calling convention on RISC-V** places return values in a0.



## System call arguments
System call implementations in the kernel need to find the arguments passed by user code. Because **user code calls system call wrapper functions**, the arguments are initially where the RISC-V C calling convention places them: in registers. **The kernel trap code saves user registers to the current process’s trap frame**, where kernel code can find them.

### Funzioni di utilità per per recuperare argomenti di un system call dal trapframe:
The kernel functions *argint*, *argaddr*, and *argfd* retrieve the n’th system call argument from the trap frame respectively as an integer, pointer, or a file descriptor (io in realtà vedo argstr al posto di argfd, ?). They all call *argraw* to retrieve the appropriate saved user register (kernel/syscall.c:34).

### MOLTO INTERESSANTE | Funzioni di utilità per copiare memoria user nella memoria kernel:
Some system calls pass pointers as arguments, and the kernel must use those pointers to read or write **user memory**. The exec system call, for example, passes the kernel an array of pointers referring to string arguments in user space. **These pointers pose two challenges**. 
- First, the user program may be buggy or malicious, and may pass the kernel an invalid pointer or a pointer intended to trick the kernel into accessing kernel memory instead of user memory.

- Second, the xv6 kernel page table mappings are not the same as the user page table mappings, so the kernel cannot use ordinary instructions to load or store from user-supplied addresses.

Per risolvere il secondo problema, the kernel implements **functions that safely transfer data to and from user-supplied addresses**.
*fetchstr* is an example (kernel/syscall.c:25) . File system calls such as *exec* use **fetchstr** to retrieve string file-name arguments from user space. *fetchstr* calls **copyinstr** to do the hard work. *copyinstr* (kernel/vm.c:415) copies up to *max* bytes to *dst* from virtual address *srcva* **in the user page table _pagetable_**. Since *pagetable* is not the current page table, *copyinstr* uses walkaddr (which calls *walk*) to look up *srcva* in *pagetable*, yielding physical address *pa0*.  

Per quanto riguarda il primo problema, *walkaddr* (kernel/vm.c:109) checks that the user-supplied virtual address is part of the process’s user address space, **so programs cannot trick the kernel into reading other memory**. A similar function, *copyout*, copies data from the kernel to a user-supplied address.

### MOLTO IMPORTANTE | Esempio di utilità del direct mapping
The kernel’s page table maps all of physical RAM at virtual addresses that are equal to the RAM’s physical address. **This allows *copyinstr* to directly copy string bytes from pa0 to dst**.

**Spiegazione**: l'indirizzo *pa0* recuperato da *walkaddr* è un indirizzo fisico puntato dalla entry della page table del processo utente. Tuttavia, *copyinstr* sta eseguendo nel kernel e quindi considera pa0 come un indirizzo **virtuale** nel contesto della kernel page table. A questo punto, accedere a pa0 in un kernel con direct mapping non da problemi, in quanto l'indirizzo fisico (come pa0 viene considerato nello spazio utente) e l'indirizzo virtuale (come pa0 viene considerato dal kernel) corrispondono. La traduzione automatica dell'HW riscv non modifica niente e il kernel accede alla stessa area di memoria considerata nello spazio user.

In un sistema **senza direct mapping**, non ci sarebbe questa corrispondenza diretta, e quindi:
   - se non si facesse niente: pa0 verrebbe considerato come indirizzo virtuale, che: potrebbe mancare e generare un page fault (nel migliore dei casi), oppure potrebbe avere un mapping e venire in un indirizzo fisico totalmente sbagliato (nel paggiore dei casi)
   - una possibile soluzione potrebbe essere creare temporanemente un mapping nella pagetable del kernel e associare quindi pa0 con un indirizzo virtuale disponibile. Successivamente, bisognerà usare questo indirizzo virtuale al posto di pa0, ed infine bisognerà ricordarsi di eliminare il mapping alla fine della gestione. 

## System call wrapper functions 
In user space, i wrapper per le invocazioni di system call sono implementati da tre file: 
- **usys.pl**
    - uno script Perl che genera automaticamente il file *usys.S*.
- **user/usys.S**
    - il file generato automaticamente contenente l'implementazione dei wrapper delle system call.
    - è dove si esegue effettivamente l'istruzione *ecall*.
- **user/user.h**
    - file che rende pubblici i metodi wrapper

### Perché i wrapper sono utili?
- Portabilità: Nascondono i dettagli specifici dell'architettura (ad esempio, uso di ecall o del registro a7).
- Semplicità: Consentono agli sviluppatori di usare chiamate di sistema direttamente in C senza dover scrivere codice assembly.
- Conformità: Garantiscono che i parametri siano passati secondo le convenzioni stabilite.

### Come aggiungere una nuova system call lato user
Se volessimo aggiungere una nuova syscall (es. rename), basterebbe aggiungere *entry("rename");* allo script perl. Per rendere poi il wrapper generato visibile bisognerà anche aggiungere una entry relativa entry dentro a *user/user.h*.

