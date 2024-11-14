Each RISC-V CPU has a set of __control registers__ that the kernel writes to tell the CPU how to handle traps, and that the kernel can read to find out about a trap that has occurred.

Here’s an outline of the most important registers:
- **stvec** (Supervisor Trap VECtor): The kernel writes the __address of its trap handler__ here; the RISC-V jumps to the address in _stvec_ to handle a trap.
- **sepc** (Supervisor Exception Program Counter): When a trap occurs, RISC-V __saves the program counter here__ (since the _pc_ is then overwritten with the value in _stvec_). The _sret_ (return from trap) instruction copies _sepc_ to the _pc_. 
    - OSS: The kernel can write _sepc_ to control where sret goes. Lo fa ad esempio in start.c per "ritornare" al main.
- **scause** (Supervisor CAUSE): RISC-V puts a number here that describes the reason for the trap.
- **sscratch** (Supervisor SCRATCH): The trap handler code uses _sscratch_ to help it avoid overwriting user registers before saving them.
- **sstatus** (Supervisor STATUS): registro che contiene vari bit che controllano vari aspetti del comportamento del sistema in modalità supervisore, come la gestione delle interruzioni e l'indicazione di come è stato gestito l'ultimo trap.
    - The SIE bit in _sstatus_ controls whether __device interrupts are enabled__. If the kernel clears SIE, the RISC-V will defer device interrupts until the kernel sets SIE.
    - The SPP bit indicates whether a trap came from user mode or supervisor mode, and controls to what mode _sret_ returns.

__NB__: The above registers relate to __traps handled in supervisor mode__, and they cannot be read or written in user mode.
___NB_2__: Each CPU on a multi-core chip has its own set of these registers, and more than one CPU may be handling a trap at any given time.

### Cosa succede quando si verifica una trap?
When it needs to force a trap, the RISC-V __hardware__(quindi in maniera __automatica__) does the following for all trap types:
1. If the trap is a device interrupt, and the _sstatus_ SIE bit is clear, don’t do any of the following.
2. __Disable interrupts__ by clearing the SIE bit in _sstatus_.
3. Copy the _pc_ to _sepc_.
4. Save the current mode (user or supervisor) in the SPP bit in _sstatus_.
5. Set _scause_ to reflect the trap’s cause.
6. Set the mode to supervisor.
7. Copy _stvec_ to the _pc_.
8. Start executing at the new _pc_.

__NB__: Note that the CPU
- doesn’t switch to the kernel page table
- doesn’t switch to a stack in the kernel
- and doesn’t save any registers other than the _pc_.
__Kernel software must perform these tasks__.

One reason that the CPU does minimal work during a trap is to provide __flexibility__ to software; for example, some operating systems omit a page table switch in some situations to increase trap performance. 