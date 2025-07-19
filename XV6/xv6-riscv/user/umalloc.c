#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;  // the alignment of the most restrictive type (long)


/*
  A free block contains a pointer to the next block, a record of the size of the block, and then the free space itself;
  - the control information at the beginning is called the header.
  - the actual memory is next (+1 with pointer arithmetic) to the header

  To simplify alignment, all blocks are multiples of the header size, and the header is aligned properly by means of a union.
  The align field is never used; it just forces each header to be aligned on a worst-case boundary.
*/
union header {
  struct {
    union header *ptr; // next block on freelist
    uint size;         // size of this block
  } s;
  Align x;             // force alignment of blocks
};

typedef union header Header; // nodo nella mia freelist

static Header base;     // empty list to get started
static Header *freep;   // start of the free list

void
free(void *ap)
{
  Header *bp, *p;

  // scan the freelist to look for a place to insert the freed block in the freelist;
  // this is either between two existing block or at one end of the list. 
  bp = (Header*)ap - 1; // punto all'header e non alla memoria
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) // questo mi controlla se il blocco liberato è in mezzo a due blocchi
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))  // questo mi controlla se il blocco liberato è ad una estremita della lista 
      break;

  // da qua in poi p punta al blocco precedente in cui mettere il freeblock
  // 
  // if the block being freed is adjacent to either neighbour, the adjacent
  // block are combined

  // join to upper neighbour
  if(bp + bp->s.size == p->s.ptr){
    // unifico dopo aver spezzato
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;

  // join to lower neighbour
  if(p + p->s.size == bp){
    // unifico dopo aver spezzato
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;

  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  // alloco minimo una pagina (non voglia fare una syscall ad ogni allocazione)
  // la memoria allocata verra poi spezzettata da malloc
  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;

  hp = (Header*)p;
  hp->s.size = nu;
  // mi adeguo all'interfaccia di free che vuole la memoria e non l'header 
  // (al suo interno fa comunque il passo indietro)
  free((void*)(hp + 1)); 
  return freep;
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;
  // the requested size is rounded up to the proper number of header sized units for alignement purposes
  // the extra +1 accounts for the header itself
  // this is the value recorded in the size filed of the header
  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;

  // if the freelist freep is null (first call of malloc) then a degenerate freelist is created 
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base; // free list points at base which points at itself
    base.s.size = 0;
  }
  
  // search the list for a free block of adequate size
  //
  // parti dal blocco successivo rispetto a quello in cui ti sei 
  // fermato la volta precedente (prevp->s.ptr);
  // this strategy helps keep the free list homogeneous
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    // blocco sufficientemente grande trovato
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        // the block is too large, return only the tail-end so
        // only the size field in the header needs to be adjusted
        p->s.size -= nunits; // calcolo di quanto mi devo spostare
        // spezzo il blocco corrente troppo grosso in due 
        // (il blocco precedente mantiene un riferimento grazie al
        //  puntatore dentro a prevp)
        p += p->s.size;     
        p->s.size = nunits; // TODO: superfluo?
      }

      // faccio partire la lista dal blocco precedente rispetto 
      // a quello che ho allocato adesso
      freep = prevp;
      // the pointer returned by malloc points at the free space not at the header
      return (void*)(p + 1);
    }

    // ho fatto il giro, alloco nuova memoria
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}
