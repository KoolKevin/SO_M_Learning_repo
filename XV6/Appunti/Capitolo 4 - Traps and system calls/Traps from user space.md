### Premessa
Xv6 handles traps differently depending on whether the trap occurs while executing in the kernel or in user code. Here is the story for traps from user code.

A trap may occur while executing in user space if the user program makes a system call (ecall instruction), or does something illegal, or if a device interrupts. The high-level __function call path__ of a trap from user space is
1. _uservec_ (kernel/trampoline.S:22)
2. then, _usertrap_ (kernel/trap.c:37)
3. and when returning, _usertrapret_ (kernel/trap.c:90)
4. and then _userret_ (kernel/trampoline.S:101) .