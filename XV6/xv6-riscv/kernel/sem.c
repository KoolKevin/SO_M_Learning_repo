#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"


struct SemTable {
  struct semaphore sems[NUM_SEM];
  uint8 status[NUM_SEM]; // 0 -> free ; != 0 -> busy
  struct spinlock lock;
};

struct SemTable sem_table;


void seminit() {
  initlock(&sem_table.lock, "sem_table");

  for(int i=0; i<NUM_SEM; i++) {
    initlock(&sem_table.sems[i].lock, "sem");
    // i semafori partono rossi perchè ho deciso io così :)
    sem_table.sems[i].count = 0; 
    sem_table.status[i] = 0;
  }
}


int acquire_sem(void) {
  for(int i=0; i<NUM_SEM; i++) {
    if (sem_table.status[i] == 0) {
      acquire(&sem_table.lock);
      sem_table.status[i] = 1;  // occupo il semaforo
      release(&sem_table.lock);

      return i;    
    }
  }

  return -1;  // nessun semaforo libero
}

void release_sem(int sem_num) {
  acquire(&sem_table.lock);
  sem_table.status[sem_num] = 0;  // libero il semaforo
  release(&sem_table.lock);
}

// libero il semaforo
// - ritorno -1 in caso di fallimento
// - 0 altrimenti
int v(int sem_num) {
  // TODO: se faccio solo questo controllo, proc A 
  // può impegnare il semaforo di proc B...
  // non ha molto senso ma me lo faccio andare bene
  if(sem_table.status[sem_num] == 0) {
    printf("chiamato V sul semaforo libero: %d\n", sem_num);
    return -1;
  }

  struct semaphore *s = &sem_table.sems[sem_num];

  acquire(&s->lock);
  s->count += 1;
  wakeup(s);
  release(&s->lock);

  return 0;
}

// impegno il semaforo
// - ritorno -1 in caso di fallimento
// - 0 altrimenti
int p(int sem_num) {
  // TODO: se faccio solo questo controllo, proc A 
  // può impegnare il semaforo di proc B...
  // non ha molto senso ma me lo faccio andare bene
  if(sem_table.status[sem_num] == 0) {
    printf("chiamato P sul semaforo libero: %d\n", sem_num);
    return -1;
  }

  struct semaphore *s = &sem_table.sems[sem_num];

  acquire(&s->lock);
  while(s->count == 0)
      sleep(s, &s->lock);
  s->count -= 1;
  release(&s->lock);

  return 0;
}