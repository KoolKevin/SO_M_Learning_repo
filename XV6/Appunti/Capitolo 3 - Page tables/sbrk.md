sbrk is the system call for a process to shrink or grow its memory. __The system call is implemented by the function growproc__ (kernel/proc.c:260). 

growproc calls uvmalloc or uvmdealloc, depending on whether n is positive or negative.
- uvmalloc (kernel/vm.c:233) allocates physical memory with kalloc, zeros the allocated memory, and adds PTEs to the user page table with mappages.
- uvmdealloc calls uvmunmap (kernel/vm.c:178), which uses walk to find PTEs and kfree to free the physical memory they refer to.

__NB__: Xv6 uses a process’s page table not just to tell the hardware how to map user virtual addresses, but also as __the only record of which physical memory pages are allocated to that process__. That is the reason why freeing user memory (in uvmunmap) requires examination of the user page table.