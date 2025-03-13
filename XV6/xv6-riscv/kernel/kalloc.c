// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"


void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;





// tipi aggiunti per tenere traccia di quanti riferimenti
// attivi ho ad una determinata pagina fisica (fork_cow())
struct page_ref_t {
  uint64 pa;
  int count;
};

struct page_ref_table_t {
  struct page_ref_t page_refs[(PHYSTOP-KERNBASE)/PGSIZE]; // sto largo con la dimensione per semplicità
  int dim;
  struct spinlock lock;
};

struct page_ref_table_t page_ref_table;

void page_ref_table_init() {
  page_ref_table.dim=0;

  char *p = (char*)PGROUNDUP((uint64)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE) {
    page_ref_table.page_refs[page_ref_table.dim].pa = (uint64)p;
    page_ref_table.page_refs[page_ref_table.dim].count = 0;
    page_ref_table.dim++;
  }
}

int get_physical_page_refs(uint64 pa) {
  pa = PGROUNDDOWN(pa);
  int index = (pa - PGROUNDUP((uint64)end)) / PGSIZE;

  return page_ref_table.page_refs[index].count;
}

void increase_physical_page_refs(uint64 pa) {
  pa = PGROUNDDOWN(pa);
  int index = (pa - PGROUNDUP((uint64)end)) / PGSIZE;

  // lock per evitare di perdere un incremento durante una corsa critica
  acquire(&page_ref_table.lock);
  page_ref_table.page_refs[index].count++;
  release(&page_ref_table.lock);

  // #ifdef DEBUG_COW
  // printf("incremento a %d i riferimenti alla pagina: 0x%lx\n", page_ref_table.page_refs[index].count, (uint64)kmem.freelist);
  // #endif  
}

void decrease_physical_page_refs(uint64 pa) {
  pa = PGROUNDDOWN(pa);
  int index = (pa - PGROUNDUP((uint64)end)) / PGSIZE;

  // lock per evitare di perdere un decremento durante una corsa critica
  acquire(&page_ref_table.lock);
  page_ref_table.page_refs[index].count--;
  release(&page_ref_table.lock);

  // #ifdef DEBUG_COW
  // printf("decremento a %d i riferimenti alla pagina: 0x%lx\n", page_ref_table.page_refs[index].count, (uint64)pa);
  // #endif  
}


int get_freemem() {
  int num_pagine = 0;
  
  // lock necessario per evitare situazioni in
  // cui recupero il nodo in cima alla freelist
  // appena prima di una kalloc (con successiva
  // modifica al puntatore next) che mi porterebbe 
  // ad accedere memoria casuale
  acquire(&kmem.lock);  
  struct run *r = kmem.freelist;

  while (r != NULL) {
    num_pagine++;
    r = r->next;
  }
  release(&kmem.lock);

  return num_pagine;
}







void
kinit()
{
  initlock(&kmem.lock, "kmem");
  page_ref_table_init(); // inizializzo anche questo
  freerange(end, (void*)PHYSTOP);
}

// la vecchia kfree diventa kfree_init dato che
// devo distinguere quando sto costruendo la freelist
// da quando sto effettivamente liberando della memoria 
// precedentemente allocata (necessità di aggiornare 
// la tabella dei riferimenti)
void kfree_init(void *pa) {
 struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree_init");

  r = (struct run*)pa;

  acquire(&kmem.lock);
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree_init(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  r = (struct run*)pa;

  int index = ((uint64)pa - PGROUNDUP((uint64)end)) / PGSIZE;
  acquire(&kmem.lock);

  // decremento i riferimenti della pagina
  if(page_ref_table.page_refs[index].pa == (uint64)pa) {
    decrease_physical_page_refs((uint64)pa);
    
    // se la pagina non è più riferita, la posso liberare    
    if(get_physical_page_refs((uint64)pa) == 0) {
      // Fill with junk to catch dangling refs.
      memset(pa, 1, PGSIZE);
      r->next = kmem.freelist;
      kmem.freelist = r;
    }
  }
  else {
    printf("non ho trovato pa=0x%lx nel posto in cui mi aspettavo nella tabella\n", (uint64)pa);
    panic("panico");
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  // incremento i riferimenti alla pagina
  int index = ((uint64)kmem.freelist - PGROUNDUP((uint64)end)) / PGSIZE;
  
  if(page_ref_table.page_refs[index].pa == (uint64)kmem.freelist) {
    increase_physical_page_refs((uint64)kmem.freelist);
    
    if(get_physical_page_refs((uint64)kmem.freelist) != 1) {
      printf("kalloc: pagina appena allocata con più di un riferimento?!\n");
      panic("panico");
    }
  } else {
    printf("non ho trovato pa=0x%lx nel posto in cui mi aspettavo nella tabella\n", (uint64)kmem.freelist);
    panic("panico");
  }

  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
