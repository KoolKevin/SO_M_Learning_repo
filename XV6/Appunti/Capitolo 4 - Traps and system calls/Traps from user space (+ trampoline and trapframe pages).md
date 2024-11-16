### Premessa
__Xv6 handles traps differently depending on whether the trap occurs while executing in the kernel or in user code__. Here is the story for traps from user code.

A trap may occur while executing in user space if the user program makes a system call (_ecall_ instruction), or does something illegal, or if a device interrupts. The high-level __function call path__ of a trap from user space is:
1. _uservec_ (kernel/trampoline.S:22)
2. then, _usertrap_ (kernel/trap.c:37)
3. and when returning, _usertrapret_ (kernel/trap.c:90)
4. and then _userret_ (kernel/trampoline.S:101).

### Perchè è necessaria la pagina trampolino?
A major constraint on the design of xv6’s trap handling is the fact that the __RISC-V hardware does not switch page tables when it forces a trap__. This means that the trap handler address in _stvec_ must have a valid mapping in the user page table, since that’s the page table in force when the trap handling code starts executing. Furthermore, xv6’s trap handling code needs to switch to the kernel page table; in order to be able to continue executing the trap handler after that switch, __the kernel page table must also have a mapping for the handler pointed to by *stvec*__.

    Xv6 satisfies these requirements using a trampoline page. The trampoline page contains uservec, the xv6 trap handling code that stvec points to.

The trampoline page is mapped in __every process’s page table__ at address __TRAMPOLINE__ , which is at the top of the virtual address space so that it will be above memory that programs use for themselves. The trampoline page is also mapped at address TRAMPOLINE in the kernel page table. Because the trampoline page is mapped in the user page table, traps can start executing there in supervisor mode. Because the trampoline page is mapped at the same address in the kernel address space, the trap handler can continue to execute after it switches to the kernel page table.

### Entering kernel space
#### uservec
The code for the _uservec_ trap handler is in _trampoline.S_ (kernel/trampoline.S:22) . When _uservec_ starts, all 32 Riscv GP registers contain values owned by the interrupted user code. These 32 values need to be saved somewhere in memory, so that later on the kernel can restore them before returning to user space. Storing to memory requires use of a register to hold the address, but at this point there are no general-purpose registers available! Luckily RISC-V provides a helping hand in the form of the _sscratch_ register. The _csrw_(control and status register write) instruction at the start of _uservec_ saves a0 in _sscratch_. Now uservec has one register (a0) to play with.

#### Complemento: Categorie di registri in RISC-V
- Registri temporanei (t0–t6)
    - Usati per operazioni temporanee.
    - Il chiamante di una funzione è responsabile di salvarli se necessario.

- Registri salvati (s0–s11)
    - Detti anche "callee-saved", sono preservati dalle funzioni chiamate.
    - Se una funzione li modifica, deve salvarli e ripristinarli.

- Registri di argomenti (a0–a7)
    - Usati per passare argomenti alle funzioni e restituire valori.
    - Il chiamante può sovrascriverli liberamente.

- Altri registri speciali:
    - ra: Registro del return address. Contiene l'indirizzo di ritorno quando una funzione è chiamata.
    - sp: Stack pointer, punta alla cima dello stack.
    - gp: Global pointer, punta alle variabili globali.
    - tp: Thread pointer, usato in ambienti multithreadi
    - zero: registro read-only che contiene sempre il valore zero

__NB__: questi nomi sono alias i nomi veri sono x0, ..., x31

#### A che cosa serve la pagina trampolino
_uservec_’s next task is to save the 32 user registers. __The kernel allocates, for each process, a page of memory for a trapframe structure__ that (among other things) has space to save the 32 user registers (kernel/proc.h:43). Because _satp_ still refers to the user page table, _uservec_ needs
the __trapframe to be mapped in the user address space__. Xv6 maps each process’s trapframe at virtual address TRAPFRAME in that process’s user page table; TRAPFRAME is just below TRAMPOLINE. The process’s p->trapframe also points to the trapframe, though at its physical address so the kernel can use it through the kernel page table (ricorda direct mapping).

Thus _uservec_ loads address TRAPFRAME into a0 and saves all the user registers there, including the user’s a0, read back from _sscratch_.

The trapframe also contains:
- the address of the current process’s kernel stack
- the current CPU’s hartid
- the address of the __usertrap__ function
- and the address of the kernel page table

_uservec_ retrieves these values, switches _satp_ to the kernel page table, and jumps to _usertrap_.

### Usertrap
The job of usertrap is to __determine the cause of the trap, process it, and return__ (kernel/trap.c:37). It first changes _stvec_ so that a trap while in the kernel will be __handled by *kernelvec* rather than *uservec*__. It saves the _sepc_ register (the saved user program counter), because
_usertrap_ might call _yield_ to switch to another process’s kernel thread, and that process might return to user space, in the process of which __it will modify *sepc*__. If the trap is a system call, _usertrap_ calls _syscall_ to handle it; if a device interrupt, _devintr_; otherwise it’s an exception, and the __kernel kills the faulting process__. The system call path adds four to the saved user program counter because RISC-V, in the case of a system call, leaves the program pointer pointing to the _ecall_ instruction but user code needs to resume executing at the subsequent instruction.
On the way out, usertrap checks if the process has been killed or should yield the CPU (if this trap is a timer interrupt).

### Returing to user space
The first step in returning to user space is the call to _usertrapret_ (kernel/trap.c:90). This function sets up the RISC-V control registers to prepare for a future trap from user space: setting _stvec_ to _uservec_ and preparing the trapframe fields that _uservec_ relies on. _usertrapret_ sets _sepc_ to the previously saved user program counter. At the end, _usertrapret_ calls _userret_ on the trampoline page that is mapped in both user and kernel page tables; the reason is that assembly code in _userret_ will switch page tables.

_usertrapret_’s call to _userret_ __passes a pointer to the process’s user page table in a0__ (kernel/trampoline.S:101). _userret_ switches _satp_ to the process’s user page table. Recall that the user page table maps both the trampoline page and TRAPFRAME, but nothing else from the kernel. __The trampoline page mapping at the same virtual address in user and kernel page tables allows _userret_ to keep executing after changing satp__.

     From this point on, the only data userret can use is the register contents and the content of the trapframe.
     
_userret_ loads the TRAPFRAME address into a0, restores saved user registers from the trapframe via a0, restores the saved user a0, and executes _sret_ to return to user space.