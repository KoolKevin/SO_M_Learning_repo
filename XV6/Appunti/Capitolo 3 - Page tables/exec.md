exec is a system call that replaces a process’s user address space with data read from a file, called a binary or executable file. A binary is typically the output of the compiler and linker, and holds machine instructions and program data.

exec (kernel/exec.c:23) opens the named binary path using _namei_ (kernel/exec.c:36), which is explained in Chapter 8. Then, it reads the __ELF header__.
__Xv6 binaries__ are formatted in the widely-used ELF format, defined in (kernel/elf.h). An ELF binary consists of an __ELF header__, _struct elfhdr_ (kernel/elf.h:6), followed by a sequence of __program section headers__, _struct proghdr_ (kernel/elf.h:25). __Each _proghdr_ describes a section of the application that must be loaded into memory__; xv6 programs have two program section headers: one for __instructions__ and one for __data__.

The first step is a quick check that the file probably contains an ELF binary. __An ELF binary starts with the four-byte “magic number”__ 0x7F , ‘E’ , ‘L’ , ‘F’ , or ELF_MAGIC (kernel/elf.h:3). If the ELF header has the right magic number, exec assumes that the binary is well-formed.

exec:
- __allocates a new page table with no user mappings__ with _proc_pagetable_ (kernel/exec.c:49)
- __allocates memory for each ELF segment__ with _uvmalloc_ (kernel/exec.c:65)
- and __loads each segment into memory__ with _loadseg_ (kernel/exec.c:10)
    - loadseg uses walkaddr to find the physical address of the allocated memory at which to write each page of the ELF segment, and readi to read from the binary ELF file.

#### stack della nuova immagine
Now exec allocates and initializes the __user stack__. It allocates just one stack page. exec copies the __argument strings__ to the top of the stack one at a time, recording the __pointers to them in ustack__. It places a __null pointer__ at the end of what will be the argv list passed to main. 

The values for argc and argv are passed to main through the system-call return path: argc is passed via the system call return value, which goes in a0, and argv is passed through the a1 entry of the process’s trapframe. (guarda in fondo alla funzione)

exec places an inaccessible page just below the stack page, so that programs that try to use more than one page will fault. This inaccessible page also allows exec to deal with arguments that are too large; in that situation, the copyout (kernel/vm.c:359) function that exec uses to copy arguments to the stack will notice that the destination page is not accessible, and will return -1.

#### liberazione della vecchia immagine
During the preparation of the new memory image, if exec detects an error like an invalid program segment, it jumps to the label bad, frees the new image, and returns -1. exec must wait to free the old image until it is sure that the system call will succeed: __if the old image is gone, the system call cannot return -1 to it__. The only error cases in exec happen during the creation of the image. Once the image is complete, exec can commit to the new page table (kernel/exec.c:125) and free the old one (kernel/exec.c:129).

#### (opzionale) rischi di exec
    exec loads bytes from the ELF file into memory at addresses specified by the ELF file. Users or processes can place whatever addresses they want into an ELF file. Thus exec is risky, because the addresses in the ELF file may refer to the kernel, accidentally or on purpose.
    
The consequences for an unwary kernel could range from a crash to a malicious subversion of the kernel’s isolation mechanisms (i.e., a security exploit). Xv6 performs a number of checks to avoid these risks. For example if(ph.vaddr + ph.memsz < ph.vaddr) checks for whether the sum overflows a 64-bit integer. The danger is that a user could construct an ELF binary with a ph.vaddr that points to a user-chosen address, and ph.memsz large enough that the sum overflows to 0x1000, which will look like a valid value. In an older version of xv6 in which the user address space also contained the kernel (but not readable/writable in user mode), the user could choose an address that corresponded to kernel memory and would thus copy data from the ELF binary into the kernel. In the RISC-V version of xv6 this cannot happen, because the kernel has its own separate page table; loadseg loads into the process’s page table, not in the kernel’s page table.

It is easy for a kernel developer to omit a crucial check, and real-world kernels have a long history of missing checks whose absence can be exploited by user programs to obtain kernel privileges. It is likely that xv6 doesn’t do a complete job of validating user-level data supplied to the kernel, which a malicious user program might be able to exploit to circumvent xv6’s isolation.
