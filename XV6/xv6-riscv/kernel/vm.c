#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"

/*
 * the kernel's page table.
 */
pagetable_t kernel_pagetable;

extern char etext[];  // kernel.ld sets this to end of kernel code.

extern char trampoline[]; // trampoline.S

// Make a direct-map page table for the kernel.
pagetable_t
kvmmake(void)
{
  pagetable_t kpgtbl;

  kpgtbl = (pagetable_t) kalloc();
  memset(kpgtbl, 0, PGSIZE);

  // uart registers
  kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  kvmmap(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // PLIC
  kvmmap(kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  kvmmap(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

  // allocate and map a kernel stack for each process.
  proc_mapstacks(kpgtbl);
  
  return kpgtbl;
}

// Initialize the one kernel_pagetable
void
kvminit(void)
{
  kernel_pagetable = kvmmake();
}

// Switch h/w page table register to the kernel's page table,
// and enable paging.
void
kvminithart()
{
  // wait for any previous writes to the page table memory to finish.
  sfence_vma();

  w_satp(MAKE_SATP(kernel_pagetable));

  // flush stale entries from the TLB.
  sfence_vma();
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.

/*
  KKoltraka:
  Questa funzione è un pò un casino, innanzitutto ritorna solamente l'indirizzo della entry corretta dell'ultimo livello
  in cui dovrà andare ad essere creato il mapping, non crea il mapping da se. In secondo luogo la struttura ad albero della
  tabella delle pagine non è evidente.

  Per questi motivi è molto utile tenere sotto la figura 3.2 del book quando si guarda questa funzione.
*/
pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  for(int level = 2; level > 0; level--) {
    /*
      KKoltraka:
      La macro PX ritorna semplicemente i 9 bit del va considerando il livello corrente.
      L'intera riga: pte_t *pte = &pagetable[PX(level, va)]; considera quindi la entry di indice PX(level, va) nella page table
      (ricorda che pagetable_t è un puntatore a uint64, si procede di 8 byte alla volta, esattamente la dimensione della entry)
    */
    pte_t *pte = &pagetable[PX(level, va)];   
    /*
        KKoltraka:
        Se la entry trovata è valida passiamo al prossimo livello. 
        Altrimenti, allochiamo il prossimo livello della pagetable e aggiorniamo e validiamo la entry corrente 
        NB: pte adesso punta al primo elemento di questa nuova page
      */
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
        return 0;
      memset(pagetable, 0, PGSIZE);
      /*
        KKoltraka:
        La macro PA2PTE dato un indirizzo fisico elimina i 12 bit di offset e prepara i 10 bit di flag.
        In sostanza trasforma l'indirizzo fisico in un formato adeguato per una PTE.

        Qui si sotto si sta quindi facendo puntare (e si valida) la entry del livello precedente alla tabella appena
        allocata nel livello corrente. 
      */
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }

  /*
    KKoltraka:
    Qua il ciclo è terminato e quindi pagetable fa riferimento all'ultimo livello. Ritorniamo quindi l'indirizzo della PTE che DOVRÀ (qua è
    ancora tutto a zero) contenere il mapping con l'indirizzo fisico 
  */
  return &pagetable[PX(0, va)];
}

// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa;

  if(va >= MAXVA)
    return 0;

  pte = walk(pagetable, va, 0);
  if(pte == 0)
    return 0;
  if((*pte & PTE_V) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;

  pa = PTE2PA(*pte);
  return pa;
}

// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(mappages(kpgtbl, va, sz, pa, perm) != 0)
    panic("kvmmap");
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa.
// va and size MUST be page-aligned.
// Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("mappages: va not aligned");

  if((size % PGSIZE) != 0)
    panic("mappages: size not aligned");

  if(size == 0)
    panic("mappages: size");
  
  a = va;
  last = va + size - PGSIZE;
  for(;;){
    if((pte = walk(pagetable, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V)
      panic("mappages: remap");
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}


// kkoltraka: uguale a sopra a meno di un check.
// Creo una nuova funzione per evitare di rompere il kernel
// in punti che non sto considerando
int
remappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("mappages: va not aligned");
  if((size % PGSIZE) != 0)
    panic("mappages: size not aligned");
  if(size == 0)
    panic("mappages: size");
  
  a = va;
  last = va + size - PGSIZE;
  for(;;){
    printf("\trimappo va: %lx in pa: %lx\n", a, pa);
    if((pte = walk(pagetable, a, 0)) == 0)
      return -1;
    // if(*pte & PTE_V)
    //   panic("mappages: remap");
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  uint64 a;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("uvmunmap: not aligned");

  for(a = va; a < va + npages*PGSIZE; a += PGSIZE){
    if((pte = walk(pagetable, a, 0)) == 0)
      panic("uvmunmap: walk");
    if((*pte & PTE_V) == 0)
      panic("uvmunmap: not mapped");
    if(PTE_FLAGS(*pte) == PTE_V)    // se la entry ha impostato solamente il bit di validità allora è un nodo intermedio?
      panic("uvmunmap: not a leaf");

    if(do_free){
      uint64 pa = PTE2PA(*pte);
      kfree((void*)pa);
    }
    *pte = 0;
  }
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t
uvmcreate()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t) kalloc();
  if(pagetable == 0)
    return 0;
  memset(pagetable, 0, PGSIZE);
  return pagetable;
}

// Load the user initcode into address 0 of pagetable,
// for the very first process.
// sz must be less than a page.
void
uvmfirst(pagetable_t pagetable, uchar *src, uint sz)
{
  char *mem;

  if(sz >= PGSIZE)
    panic("uvmfirst: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pagetable, 0, PGSIZE, (uint64)mem, PTE_W|PTE_R|PTE_X|PTE_U);
  memmove(mem, src, sz);  // kkoltraka: notare direct mapping
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
  char *mem;
  uint64 a;

  if(newsz < oldsz)
    return oldsz;

  oldsz = PGROUNDUP(oldsz);
  for(a = oldsz; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      // kkoltraka: marcia indietro
      uvmdealloc(pagetable, a, oldsz);  
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_R|PTE_U|xperm) != 0){
      kfree(mem);
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  if(newsz >= oldsz)
    return oldsz;

  if(PGROUNDUP(newsz) < PGROUNDUP(oldsz)){
    int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
    uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
  }

  return newsz;
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void
freewalk(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V) && (pte & (PTE_R|PTE_W|PTE_X)) == 0){
      // this PTE points to a lower-level page table.
      uint64 child = PTE2PA(pte);
      freewalk((pagetable_t)child);
      pagetable[i] = 0;
    } else if(pte & PTE_V){
      panic("freewalk: leaf");
    }
  }
  kfree((void*)pagetable);
}

// Free user memory pages,
// then free page-table pages.
void
uvmfree(pagetable_t pagetable, uint64 sz)
{
  if(sz > 0)
    uvmunmap(pagetable, 0, PGROUNDUP(sz)/PGSIZE, 1);
  freewalk(pagetable);
}

// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  uint64 pa, i;
  uint flags;
  char *mem;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto err;
    memmove(mem, (char*)pa, PGSIZE);
    if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0){
      kfree(mem);
      goto err;
    }
  }
  return 0;

 err:
  uvmunmap(new, 0, i / PGSIZE, 1);
  return -1;
}




/* ------------ kkoltraka: roba scritta da me ----------- */



// date le tabelle delle pagine di un processo padre e di un processo figlio, 
// questa funzione aggiunge nella tabella del figlio tutti i mapping necessari
// per riferire la memoria del padre. Inoltre, siccome ora la memoria è condivisa
// da due processi, viene resettato PTE_W in tutti i PTE di entrambe le tabelle.
// (questa funzione viene usata da fork_cow)
int uvmshare(pagetable_t old, pagetable_t new, uint64 sz) {
  pte_t *pte;
  uint64 pa, i;
  uint flags;

  for(i = 0; i < sz; i += PGSIZE) {
    // recupero il PTE corrente dalla tabella del padre
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmshare: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmshare: page not present");

    #ifdef DEBUG
    printf("\tpadre condivide: 0x%lx e rendo tutto read-only\n", i);
    #endif

    // sostituisco i flag PTE_W con PTE_COW 
    if (*pte & PTE_W) {
        *pte &= ~PTE_W;  
        *pte |= PTE_COW; 
    }
    // recupero indirizzo fisico e flags dal PTE
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    // rimappo la memoria del padre nella tabella del figlio (con i flag corretti)
    if(mappages(new, i, PGSIZE, (uint64)pa, flags) != 0){
      // non ho abbastanza memoria per tutte le pagine della tabella,
      // faccio marcia indietro
      uvmunmap(new, 0, i/PGSIZE, 1);  
      return -1;
    }
    increase_physical_page_refs((uint64)pa);
  }

  return 0;
}

void coredump(pagetable_t table, uint64 sz) {
  pte_t *pte;
  uint64 pa, va;
  uint flags;

  for(va = 0; va < sz; va += PGSIZE){
    // recupero il PTE corrente dalla tabella del padre
    if((pte = walk(table, va, 0)) == 0)
      panic("coredump: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("coredump: page not present");

    // recupero indirizzo fisico e flags dal PTE
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    
    printf("\tva: 0x%lx -> pa: 0x%lx\t", va, pa);
    if(flags & PTE_V)
      printf("V|");
    if(flags & PTE_R)
      printf("R|");
    if(flags & PTE_W)
      printf("W|");
    if(flags & PTE_X)
      printf("X|");
    if(flags & PTE_COW)
      printf("COW|");
    if(flags & PTE_U)
      printf("U");

    int refs = get_physical_page_refs(pa);
    printf("\trefs = %d\n", refs);
    
    printf("\n");
  }
}













// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0)
    panic("uvmclear");
  *pte &= ~PTE_U;
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
/*
  kkoltraka:
  Il passaggio da kernel a user sta nel fatto che src è un puntatore a memoria del kernel
  che sta venendo copiato nella memoria descritta nella pagetable del user

  ATTENZIONE
  devo tenere conto anche di fork_cow!
*/
int
copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  uint64 n, va0, pa0;
  pte_t *pte;

  while(len > 0){
    va0 = PGROUNDDOWN(dstva);
    if(va0 >= MAXVA)
      return -1;

    pte = walk(pagetable, va0, 0);
    if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0 ||
       ((*pte & PTE_W) == 0 && (*pte & PTE_COW) == 0) )
      return -1;

    pa0 = PTE2PA(*pte); // direttamente accessibile dal kernel dato che è directly-mapped
    n = PGSIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)(pa0 + (dstva - va0)), src, n);

    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }
  return 0;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  uint64 n, va0, pa0;

  while(len > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;

    n = PGSIZE - (srcva - va0);
    if(n > len)
      n = len;
    memmove(dst, (void *)(pa0 + (srcva - va0)), n);

    len -= n;
    dst += n;
    srcva = va0 + PGSIZE;
  }
  return 0;
}

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int
copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
  uint64 n, va0, pa0;
  int got_null = 0;

  while(got_null == 0 && max > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;

    n = PGSIZE - (srcva - va0);
    if(n > max)
      n = max;

    /*
      KKoltraka
      Da notare molto bene: in realtà, quelli che qua si stanno considerando come indirizzi fisici (pa0 e p) recuperati
      dalla page table dell'utente, sono comunque indirizzi virtuali mappati anche nella tabella delle pagine del kernel.
      Si possono lo stesso pensare come indirizzi fisici dato che il kernel xv6 fa direct mapping.

      Senza direct mapping si sarebbe dovuto creare temporaneamente una nuova entry nella page table del kernel
      con cui mappare l'indirizzo fisico recuperato con walkaddr(). Se no, l'indirizzo fisico sarebbe stato considerando 
      come virtuale dall'HW e sottoposto ad una traduzione automatica durante la dereferenziazione. Di conseguenza, si sarebbe
      acceduto ad una locazione sbagliata della memoria fisica (in caso di entry presente), o nel migliore dei casi, si
      sarebbe ottenuta una page fault.  
    */
    char *p = (char *) (pa0 + (srcva - va0));
    while(n > 0){
      if(*p == '\0'){
        *dst = '\0';
        got_null = 1;
        break;
      } else {
        *dst = *p;
      }
      --n;
      --max;
      p++;
      dst++;
    }

    srcva = va0 + PGSIZE;
  }
  if(got_null){
    return 0;
  } else {
    return -1;
  }
}
