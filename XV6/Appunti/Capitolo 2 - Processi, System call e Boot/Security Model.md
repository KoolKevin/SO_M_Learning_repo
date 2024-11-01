You may wonder how the operating system deals with buggy or malicious code. Firstly:

    The operating system must assume that a process’s user-level code will do its best to wreck the kernel or other processes.

User code may try to:
- dereference pointers outside its allowed address space
- it may attempt to execute any RISC-V instructions, even those not intended for user code
- it may try to read and write any RISC-V control register
-  it may try to directly access device hardware
-  and it may pass clever values to system calls in an attempt to trick the kernel into crashing or doing something stupid


The kernel’s goal is to:

    restrict each user processes so that all it can do is read/write/execute its own user memory, use the 32 general-purpose RISC-V registers, and affect the kernel and other processes only in the ways that system calls are intended to allow.

__The kernel must prevent any other actions__. This is typically an absolute requirement in kernel design.

The expectations for the kernel’s own code are quite different. Kernel code is assumed to be written by well-meaning and careful programmers. Kernel code is expected to be bug-free, and certainly to contain nothing malicious. This assumption affects how we analyze kernel code. For example, there are many internal kernel functions (e.g., the spin locks) that would cause serious problems if kernel code used them incorrectly.

Of course in real life things are not so straightforward. It’s difficult to prevent clever user code from making a system unusable (or causing it to panic) by consuming kernel-protected resources: disk space, CPU time, process table slots, etc. It’s usually impossible to write bug-free kernel
code or design bug-free hardware; if the writers of malicious user code are aware of kernel or hardware bugs, they will exploit them. Even in mature, widely-used kernels, such as Linux, people discover new vulnerabilities continuously. It’s worthwhile to design safeguards into the kernel against the possibility that it has bugs: __assertions__, __type checking__, __stack guard pages__, etc.

Finally, the distinction between user and kernel code is sometimes blurred: some privileged user-level processes may provide essential services and effectively be part of the operating system, and in some operating systems privileged user code can insert new code into the kernel (as with Linux’s loadable kernel modules).