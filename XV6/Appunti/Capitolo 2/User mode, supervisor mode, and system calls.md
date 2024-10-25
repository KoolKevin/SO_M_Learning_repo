__Strong isolation requires a hard boundary between applications and the operating system__. If the application makes a mistake, we don’t want the operating system to fail or other applications to fail. Instead, the operating system should be able to clean up the failed application and continue
running other applications. 

To achieve strong isolation, the operating system must arrange that applications cannot modify (or even read) the operating system’s data structures and instructions and that applications cannot access other processes’ memory.

__CPUs provide hardware support for strong isolation__. For example, __RISC-V has three modes__ in which the CPU can execute instructions: __machine mode__, __supervisor mode__, and __user mode__. Instructions executing in machine mode have full privilege; a CPU starts in machine mode. 

Machine mode is mostly intended for setting up the computer during boot. Xv6 executes a few lines in machine mode and then changes to supervisor mode.

In supervisor mode the CPU is allowed to execute privileged instructions: for example, enabling and disabling interrupts, reading and writing the register that holds the address of a page table, etc. If an application in user mode attempts to execute a privileged instruction, then the CPU doesn’t execute the instruction, but switches to supervisor mode so that supervisor-mode code can terminate the application, because it did something it shouldn’t be doing.

An application can execute only user-mode instructions (e.g., adding numbers, etc.) and is said to be running in user space, while the software in supervisor mode can also execute privileged instructions and is said to be running in kernel space. The software running in kernel space (or in supervisor mode) is called the kernel.

An application that wants to invoke a kernel function (e.g., the read system call in xv6) must transition to the kernel; __an application cannot invoke a kernel function directly__. CPUs provide a special instruction that switches the CPU from user mode to supervisor mode and enters the kernel at an entry point specified by the kernel. (RISC-V provides the _ecall_ instruction for this purpose.) Once the CPU has switched to supervisor mode, the kernel can then validate the arguments of the system call (e.g., check if the address passed to the system call is part of the application’s memory), decide whether the application is allowed to perform the requested operation (e.g., check if the application is allowed to write the specified file), and then deny it or execute it.

__NB__: It is important that the kernel control the entry point for transitions to supervisor mode; if the application could decide the kernel entry point, a malicious application could, for example, enter the kernel at a point where the validation of arguments is skipped (and do whatever it wants).