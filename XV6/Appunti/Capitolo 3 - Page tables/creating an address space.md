Guarda la sezione 3.3, Si descrive principalmente vm.c e kalloc.c

### Alcuni dettagli interessanti
- A pagetable_t may be either the kernel page table, or one of the per-process page tables.

- Functions starting with kvm manipulate __the__ kernel page table; functions starting with uvm manipulate __a__ user page table;

- Early in the boot sequence, main calls kvminit (kernel/vm.c:54) to create the kernel’s page table using kvmmake (kernel/vm.c:20) . This call occurs __before xv6 has enabled paging__ on the RISC-V, so __addresses refer directly to physical memory__.

- The code from _kvmmap_ __depends on physical memory being direct-mapped into the kernel virtual address space__. For example, as walk descends levels of the page table, it pulls the (physical) address of the next-level-down page table from a PTE ( pagetable = (pagetable_t)PTE2PA(*pte); ), and then uses that address as a virtual address to fetch the PTE at the next level down ( pte_t *pte = &pagetable[PX(level, va)]; ).
    - boh, non ho capito

- main calls kvminithart (kernel/vm.c:62) to install the kernel page table. It writes the physical address of the root page-table page into the register satp. __After this the CPU will translate addresses using the kernel page table__.
    - __NB__: Since the kernel uses a direct mapping, the __now virtual__ address of the next instruction will map to the right physical memory address.
