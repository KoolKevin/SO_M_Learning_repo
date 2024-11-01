The job of an operating system is to share a computer among multiple programs and to __provide a more useful set of services than the hardware alone supports__. 

    Il kernel di un sistema operativo non è nient'altro che un processo speciale (ma sempre un processo come quelli in user space) che offre servizi ai processi in user space astraendo i dettagli dell'hardware e offrendo trasparenza ed una interfaccia più semplice.

When a process needs to invoke a kernel service, it invokes a system call, one of the calls in the operating system’s interface. The system call enters the kernel; the kernel performs the service and returns. Thus a process alternates between executing in user space and kernel space. 

    The collection of system calls that a kernel provides is the only interface towards the hardware that user programs see.

The rest of this chapter outlines xv6’s services—processes, memory, file descriptors, pipes, and a file system—and illustrates them with code snippets and discussions of how the shell, Unix’s command-line user interface, uses them.