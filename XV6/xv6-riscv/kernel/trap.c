#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

extern struct spinlock wait_lock;
//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts (dopo che vengono riabilitati) and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc(); // kkoltraka: popolato dall'hardware
  
  if(r_scause() == 8){
    // system call

    if(killed(p))
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on();

    syscall();
  } else if((which_dev = devintr()) != 0){
    // ok
  }
  /* kkoltraka: aggiunto da me */
  else if(r_scause() == 0xf) { // 0xf == store page fault (fork_cow) 
    pte_t *pte;
    uint64 page_pa;
    char* new_page_pa;
    uint64 faulty_va = PGROUNDDOWN(r_stval());
    uint flags;

    acquire(&wait_lock);

    #ifdef DEBUG_COW
    printf("gestisco una store page fault per (pid=%d; stval=0x%lx)\n", p->pid, faulty_va);
    #endif

    if((pte = walk(p->pagetable, faulty_va, 0)) == 0)
      panic("usertrap::store page fault: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("usertrap::store page fault: page not present");

    // recupero indirizzo fisico e flags dal PTE
    page_pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    flags |= PTE_W; // ora posso scrivere!
    flags &= ~PTE_COW; 
    // recupero il numero di processi che stanno riferendo questa pagina
    int refs = get_physical_page_refs(page_pa);

    // alloco la pagina di copia e aggiorno il vecchio   
    // mapping che puntava alla pagina condivisa
    if(refs > 1) {
      if((new_page_pa = kalloc()) == 0) {
        printf("usertrap::store page fault: non ho abbastanza memoria per allocare una nuova pagina");
        setkilled(p);
      }
      memmove(new_page_pa, (char*)page_pa, PGSIZE);
      
      decrease_physical_page_refs(page_pa);
      uvmunmap(p->pagetable, faulty_va, 1, 0); // non c'è bisogno di controllo di errore
      mappages(p->pagetable, faulty_va, PGSIZE, (uint64)new_page_pa, flags);
    }
    // se ho solo un riferimento mi basta aggiornare il PTE
    else {
      #ifdef DEBUG_COW
      printf("\tsono l'unico che sta riferendo la pagina pa=0x%lx e quindi la rendo semplicemente scrivibile!\n", (uint64)page_pa);
      #endif
      
      uvmunmap(p->pagetable, faulty_va, 1, 0); 
      mappages(p->pagetable, faulty_va, PGSIZE, (uint64)page_pa, flags);
    }
    
    release(&wait_lock);
    
    // if(remap_page(p->pagetable, faulty_va, (uint64)new_page_pa, flags) != 0){
    //   printf("usertrap::store page fault: questo non dovrebbe succedere mai dato che sto aggiornando solo un PTE");
    //   kfree(new_page_pa);
    //   setkilled(p);
    // }
  }
  else {
    printf("usertrap(): unexpected scause 0x%lx pid=%d\n", r_scause(), p->pid);
    printf("            sepc=0x%lx stval=0x%lx\n", r_sepc(), r_stval());
    setkilled(p);
  }

  if(killed(p))
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2)
    yield();

  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  /*
    TRAMPOLINE                        = indirizzo virtuale
    trampoline e uservec              = indirizzi fisici (direct mapping) 
    uservec-trampoline                = offset del codice handler all'interno del codice del trampolino
    TRAMPOLINE + (uservec-trampoline) = indirizzo virtuale effettivo dell'handler per le trap in userspace
  */
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to userret in trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc(); //kkoltraka: sepc popolato dall'hardware
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)  // kkoltraka: ricorda che l'HW resetta questo bit automaticamente ad ogni trap
    panic("kerneltrap: interrupts enabled");

  // kkoltraka: gestisco solo gli interrupt dato che dal
  // - non ha senso invocare system call
  // - si assume che non vengano generate eccezioni
  if((which_dev = devintr()) == 0){
    // interrupt or trap from an unknown source
    printf("scause=0x%lx sepc=0x%lx stval=0x%lx\n", scause, r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0)
    yield();

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  if(cpuid() == 0){
    acquire(&tickslock);
    ticks++;
    // kkoltraka: wakes up any processes waiting in the sleep system call
    wakeup(&ticks);
    release(&tickslock);
  }

  // ask for the next timer interrupt. this also clears
  // the interrupt request. 1000000 is about a tenth
  // of a second.
  w_stimecmp(r_time() + 1000000);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if(scause == 0x8000000000000009L){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000005L){
    // timer interrupt.
    clockintr();
    return 2;
  } else {
    return 0;
  }
}

