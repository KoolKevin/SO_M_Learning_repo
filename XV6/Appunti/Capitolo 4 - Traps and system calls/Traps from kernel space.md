Xv6 handles traps from kernel code in a **different way** than traps from user code.

Innanzitutto, when entering the kernel (hai visto i motivi per cui questo può accadere prima: syscall, exception, device interrupts), *usertrap* points *stvec* to the assembly code at **kernelvec** (kernel/kernelvec.S:12).

Since *kernelvec* only executes if xv6 was already in the kernel, *kernelvec* can rely on **satp** being set to the kernel page table, and on the stack pointer referring to a valid kernel stack. *kernelvec* pushes all 32 registers onto the stack, from which it will later restore them so that the interrupted kernel code can resume without disturbance.

**OSS**: *kernelvec* saves the registers on the stack of the interrupted kernel thread (non inteso come core), which makes sense because the register values belong to that thread. This is particularly important if the trap causes a switch to a different thread – in that case the trap will actually return from the stack of the new thread, leaving the interrupted thread’s saved registers safely on its stack.

### Kerneltrap()
*kernelvec* jumps to *kerneltrap* (kernel/trap.c:135) after saving registers. *kerneltrap* is prepared for two types of traps: device interrupts and exceptions (chiaramente le syscall non esistono se siamo già dentro al kernel). It calls **devintr** (kernel/trap.c:185) to check for and handle the former. If the trap isn’t a device interrupt, it must be an exception, and that is always a fatal error if it occurs in the xv6 kernel; the kernel calls panic and stops executing.

If _kerneltrap_ was called due to a timer interrupt, and a process’s kernel thread is running (as opposed to a scheduler thread), *kerneltrap* calls **yield** to give other threads a chance to run. At some point one of those threads will yield, and let our thread and its *kerneltrap* resume again. Chapter 7 explains what happens in yield.

When *kerneltrap*’s work is done, it needs to return to whatever code was interrupted by the trap. Because a *yield* may have disturbed *sepc* and the previous mode in *sstatus*, *kerneltrap* saves them when it starts. It now restores those control registers and returns to *kernelvec* (kernel/kernelvec.S:38). *kernelvec* pops the saved registers from the stack and executes *sret*, which **copies sepc to pc** and resumes the interrupted kernel code.

    ??? It’s worth thinking through how the trap return happens if kerneltrap called yield due to a timer interrupt. ???

Cambiando argomento, Xv6 sets **a** CPU’s *stvec* to *kernelvec* when that CPU enters the kernel from user space; you can see this in *usertrap* (kernel/trap.c:29). There’s a window of time when the kernel has started executing but *stvec* is still set to *uservec*, and it’s crucial that no device interrupt occur during that window. Luckily the RISC-V always disables interrupts (vedi trap_machinery.md) when it starts to take a trap, and *usertrap* doesn’t enable them again until after it sets *stvec*.