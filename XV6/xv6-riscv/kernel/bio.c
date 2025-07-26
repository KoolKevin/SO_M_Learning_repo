// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  // contorto, costruisce la lista in maniera che
  // head.next is most recent, head.prev is least.
  // Siccome stiamo ordinando la lista per ordine di inserimento,
  // - facendo ->next, stiamo andando a indietro nella lista
  // - facendo ->prev, stiamo andando avanti nella lista
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next; // il successore dell'ultimo nodo appena inserito è il secondo nodo più recente (andiamo indietro)
    b->prev = &bcache.head;     // il predecessore dell'ultimo nodo appena inserito è la testa (vedila come recentezza 0)
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b; // il predecessore del penultimo nodo più recente è il nodo più recente (andiamo avanti)
    bcache.head.next = b;       // il successore della testa è il nodo più recente
    
    // nota che ogni volta che inserisco un nuovo nodo i puntatori che sovrascrivo (non appartenti al nuovo nodo)
    // sono solo il prev del penultimo nodo, e il next della head. Questo significa che:
    // - i next di tutti i nodi precedentmente inseriti continuano a puntare al nodo precedente
    // - i prev di tutti i nodi prima del penultimo continuano a puntare al nodo successivo
    // - il prev della head continua a puntare al nodo meno recente!!!
  }
}

// Get a buffer for the given sector (blockno)
// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return LOCKED buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.lock);

  // Is the block already cached?
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      // only one thread at a time can access this cached buffer
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer (head.prev).
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    // controllo che il buffer non sia utilizzato
    // potrebbero esserci molti thread bloccati 
    // nell'acquiresleep() sopra. Se riciclassi il buffer
    // quei thread acquisirebbero un buffer diverso da quello
    // che desideravano
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0; // rileggo dato che non voglio i contenuti precedenti
      b->refcnt = 1;
      release(&bcache.lock);
      // only one thread at a time can access this cached buffer
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  // se il buffer non è stato caricato con i contenuti 
  // del relativo settore nel disco, lo carico adesso
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;    // collego il precedente rispetto a dov'ero con il successivo
    b->prev->next = b->next;    // ...
    b->next = bcache.head.next; // mi posiziono in testa
    b->prev = &bcache.head;     // ...
    bcache.head.next->prev = b; // il prev del penultimo più recente, punta al più recente
    bcache.head.next = b;       // il next della testa punta al più recente 
  }
  
  release(&bcache.lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


