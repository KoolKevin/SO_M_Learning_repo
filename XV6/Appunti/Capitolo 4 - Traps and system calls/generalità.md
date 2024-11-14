### Tipologie di trap
There are __three__ kinds of event which cause the CPU to set aside ordinary execution of instructions and force a transfer of control to special code that handles the event. 
- One situation is a __system call__, when a user program executes the _ecall_ instruction to ask the kernel to do something for it.
-  Another situation is an __exception__: an instruction (user or kernel) does something illegal, such as divide by zero or use an invalid virtual address.
- The third situation is a __device interrupt__, when a device signals that it needs attention, for example when the disk hardware finishes a read or write request.

### Tipico flow di gestione di una trap
This book uses trap as a generic term for these situations. Typically whatever code was executing at the time of the trap will later need to resume, and shouldn’t need to be aware that anything special happened. That is, we often want __traps to be transparent__; this is particularly important for
device interrupts, which the interrupted code typically doesn’t expect. 

The usual sequence is that __a trap forces a transfer of control into the kernel__; the kernel saves registers and other state so that execution can be resumed; the kernel executes appropriate handler code (e.g., a system call implementation or device driver); the kernel restores the saved state and returns from the trap; and the original code resumes where it left off.

Xv6 handles all three types of traps in the kernel; traps are not delivered to user code.
- Handling traps in the kernel is natural for system calls.
- It makes sense for interrupts since isolation demands that only the kernel be allowed to use devices, and because the kernel is a convenient mechanism with which to share devices among multiple processes.
- It also makes sense for exceptions since xv6 responds to all exceptions from user space by killing the offending program.

Xv6 trap handling proceeds in four stages:
- hardware actions taken by the RISC-V CPU
- some assembly instructions that prepare the way for kernel C code
- a C function that decides what to do with the trap
- and the system call or device-driver service routine.

__NB__: While commonality among the three trap types suggests that a kernel could handle all traps with a single code path, it turns out to be convenient to have separate code for two distinct cases: traps from user space, and traps from kernel space.