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
};

struct page_ref_table_t page_ref_table;

void page_ref_table_init() {
  char *p = (char*)PGROUNDUP((uint64)end);
  
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE) {
    page_ref_table.page_refs[page_ref_table.dim].pa = (uint64)p;
    page_ref_table.page_refs[page_ref_table.dim].count = 0;
    page_ref_table.dim++;
  }
}

// bruttura per non 
int inizializzazione;

void
kinit()
{
  inizializzazione = 1;
  initlock(&kmem.lock, "kmem");
  page_ref_table.dim=0;
  freerange(end, (void*)PHYSTOP);
  inizializzazione = 0;
}


void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
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

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  /*
      KKoltraka
      Le 5 righe qua sotto, sono una maniera un po' contorta per creare una lista linkata delle pagine libere
      che però parte dall'ultima pagina in memoria. La variabile che rappresenta questa lista è kmem.freelist.
        - Alla prima chiamata di kfree() abbiamo che kmem.freelist punta all'unica pagina che abbiamo appena liberato 
          mentre kmem.freelist.next punta a NULL
        - Alle invocazioni successive kmem.freelist puntarà alla pagina "più alta" libera, mentre kmem.freelist.next
          punterà alla pagina immediatamente sotto fino a raggiungere di nuovo NULL

      NB: i puntatori alle pagina puntano al primo indirizzo della pagina, sono presenti quindi altri 4095 indirizzi 
      NB_2: è interessante anche il fatto che in un certo senso la memoria considerata "libera" in realtà stia venendo
      utilizzata come una struttura dati! Attraverso il cast a (struct run*) diventa possibile assegnare ai primi 8 byte
      di ogni pagina libera l'indirizzo della prossima pagina libera. In sostanza ogni pagina libera diventa un nodo della
      lista linkata delle pagine libere. NON C'È STATO BISOGNO DI USARE DELLA MEMORIA A PARTE PER TENERE TRACCIA DELLE
       PAGINE LIBERE.  
  */
  r = (struct run*)pa;

  int found = 0;
  int i=0;
  acquire(&kmem.lock);
  // decremento i riferimenti della pagina (se non sto venendo chiamato da kinit())
  if(!inizializzazione) {
    while(i < page_ref_table.dim && !found) {
      if(page_ref_table.page_refs[i].pa == (uint64)pa) {
        found = 1;
        page_ref_table.page_refs[i].count--;
        printf("decremento a %d i riferimenti alla pagina: 0x%lx\n", page_ref_table.page_refs[i].count, (uint64)pa);

        // se la pagina non è più riferita, la posso liberare    
        if(page_ref_table.page_refs[i].count == 0) {
          r->next = kmem.freelist;
          kmem.freelist = r;
        }
      }

      i++;
    }

    if(!found) {
      printf("kfree: nella tabella dei riferimenti alle pagine non ho trovato pa=0x%lx\n", (uint64)pa);
      panic("panico");
    }
  } else {
    r->next = kmem.freelist;
    kmem.freelist = r;
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
  int found = 0;
  int i=page_ref_table.dim-1; // nell'allocazione ha senso partire dal fondo
  printf("ultimo elemento nella tabella: 0x%lx\n", (uint64)page_ref_table.page_refs[i].pa);
  while(i < page_ref_table.dim && !found) {
    if(page_ref_table.page_refs[i].pa == (uint64)kmem.freelist) {
      found = 1;
      page_ref_table.page_refs[i].count++;
      printf("incremento a %d i riferimenti alla pagina: 0x%lx\n", page_ref_table.page_refs[i].count, (uint64)kmem.freelist);
      if(page_ref_table.page_refs[i].count != 1) {
        printf("kalloc: pagina appena allocata con più di un riferimento?!\n");
        panic("panico");
      }
    }

    i++;
  }

  if(!found) {
    printf("kalloc: nella tabella dei riferimenti alle pagine non ho trovato pa=0x%lx\n", (uint64)kmem.freelist);
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
