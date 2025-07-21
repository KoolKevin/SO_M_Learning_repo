### Definizione coincisa di concorrenza
The word concurrency refers to situations in which multiple instruction streams are interleaved, due to:
- multiprocessor parallelism
- thread switching
- interrupts


## Locking
The downside of locks is that they can limit performance, because they serialize concurrent operations. We say that multiple processes conflict if they want the same lock at the same time, or that the lock experiences contention. A major challenge in kernel design is avoidance of lock contention in pursuit of parallelism. 

Sophisticated kernels organize data structures and algorithms specifically to avoid lock contention. In the list example, a kernel may maintain a separate free list per CPU and only touch another CPU’s free list if the current CPU’s list is empty and it must steal memory from another CPU. Other use cases may require more complicated designs.
- tecnica che ha insegnato Mattoccia!



### Invariants
When we say that a lock protects data, we really mean that **the lock protects some collection of invariants that apply to the data**. 

**Invariants are properties of data structures that are maintained across operations.** Typically, an operation’s **correct behavior** depends on the **invariants being true when the operation begins**. The operation may temporarily violate the invariants but must reestablish them before finishing.
- operation intesa come un qualcosa di più di un'istruzione singola

For example, in the linked list case, the invariant is that list points at the first element in the list and that each element’s next field points at the next element. The implementation of push violates this invariant temporarily: in line 17, l points to the next list element, but list does not point at l yet (reestablished at line 18).

**The race we examined above happened because a second CPU executed code that depended on the list invariants while they were (temporarily) violated**. Proper use of a lock ensures that only one CPU at a time can operate on the data structure in the critical section, so that no CPU will execute a data structure operation when the data structure’s invariants do not hold.



### Deadlock and lock ordering
If a code path through the kernel must hold several locks at the same time, it is important that **all code paths acquire those locks in the same order**. If they don’t, there is a **risk of deadlock**.

Let’s say two code paths in xv6 need locks A and B, but code path 1 acquires locks in the order A then B, and the other path acquires them in the order B then A. Suppose thread T1 executes code path 1 and acquires lock A, and thread T2 executes code path 2 and acquires lock B. Next T1 will try to acquire lock B, and T2 will try to acquire lock A. Both acquires will block indefinitely, because in both cases the other thread holds the needed lock, and won’t release it until its acquire returns. To avoid such deadlocks, all code paths must acquire locks in the same order.
- TL;DR: uno impegna il lock di cui l'altro ha bisogno -> deadlock

**NB**: The need for a **global lock acquisition order** means that **locks are effectively part of each function’s specification**: callers must invoke functions in a way that causes locks to be acquired in the agreed-on order.


### Locks and interrupt handlers
Some xv6 spinlocks protect **data that is used by both threads and interrupt handlers**. 

The interaction of spinlocks and interrupts raises a potential danger. Suppose sys_sleep holds tickslock, and its CPU is interrupted by a **timer interrupt**. clockintr would try to acquire tickslock, see it was held, and wait for it to be released. In this situation, tickslock will never be released: only sys_sleep can release it, but sys_sleep will not continue running **until clockintr returns** (but device interrupts return only when they complete). So the CPU will deadlock, and any code that needs either lock will also freeze.

To avoid this situation, **if a spinlock is used by an interrupt handler**, a CPU must never hold that lock with interrupts enabled. Xv6 is more conservative: **when a CPU acquires any lock, xv6 always disables interrupts on that CPU**. Interrupts may still occur on other CPUs, so an interrupt’s acquire can wait for a thread to release a spinlock; just not on the same CPU.



### Instructions and memory ordering
It is natural to think of programs executing in the order in which source code statements appear. That’s a reasonable mental model for single-threaded code, but is **incorrect when multiple threads interact through shared memory**.
- One reason is that **compilers emit load and store instructions in orders different** from those implied by the source code, and may entirely **omit them** (for example by caching data in registers).
- Another reason is that the **CPU may execute instructions out of order** to increase performance. For example, a CPU may notice that in a serial sequence of instructions A and B are not dependent on each other. The CPU may start instruction B first, either because its inputs are ready before A’s inputs, or in order to overlap execution of A and B.


The good news is that **compilers and CPUs help concurrent programmers by following a set of rules called the MEMORY MODEL**, and by providing some **primitives** to help programmers control re-ordering.

To tell the hardware and compiler not to re-order, xv6 uses **__sync_synchronize()** in both acquire and release. __sync_synchronize() is a **memory barrier**: it tells the compiler and CPU to not reorder loads or stores across the barrier. 

The barriers in xv6’s acquire and release **force order** in almost all cases where it matters, since xv6 uses locks around accesses to shared data.








### Spinlocks
The important field in the structure is locked, a word that is zero when the lock is available and non-zero when it is held.

Due CPU potrebbero controllare se il lock è libero contemporaneamente e successivamente acquisire quest'ultimo entrambe...
What we need is a way to make lines 25 and 26 execute as an atomic (i.e., indivisible) step.

Because locks are widely used, multi-core processors usually provide instructions that implement an atomic version of lines 25 and 26. On the RISC-V this instruction is amoswap r, a.

amoswap:
- reads the value at the memory address a
- writes the contents of register r to that address,
- and puts the value it read into r.

That is, it **swaps the contents of the register and the memory address**. It performs this sequence atomically, using special hardware to prevent any other CPU from using the memory address between the read and the write.

Xv6’s acquire uses the portable C library call **__sync_lock_test_and_set**, which boils down to the amoswap instruction; the return value is the old (swapped) contents of lk->locked.

The function release is the opposite of acquire: it clears the lk->cpu field and then releases the lock.

Conceptually, the release just requires assigning zero to lk->locked. The C standard allows compilers to implement an assignment with multiple store instructions, so a C assignment might be non-atomic with respect to concurrent code. 

Instead, release uses the C library function **__sync_lock_release** that performs an atomic assignment. This function also boils down to a RISC-V amoswap instruction.




### Sleeplocks
Sometimes xv6 needs to hold a lock for a long time. For example, the file system (Chapter 8) keeps a file locked while reading and writing its content on the disk, and these disk operations can take tens of milliseconds. Holding a spinlock that long would lead to waste if another process wanted to acquire it, since the acquiring process would waste CPU for a long time while spinning. Another drawback of spinlocks is that a process cannot yield the CPU while retaining a spinlock; we’d like to do this so that other processes can use the CPU while the process with the lock waits for the disk.
1. spreco cpu-time di altre cpu che aspettano che il lock venga rilasciato
2. spreco la cpu corrente non facendole fare nulla intanto che l'io-operation termina


Yielding while holding a spinlock is illegal because it might lead to deadlock if a second thread then tried to acquire the spinlock: the second thread’s spinning might prevent the first thread from running and releasing the lock.

Yielding while holding a lock would also violate the requirement that interrupts must be off while a spinlock is held.


We’d like a type of lock:
1. that yields the CPU while waiting to acquire
2. and allows yields (and interrupts (minimo dal timer per la preemption)) while the lock is held




NB:
- Because sleep-locks leave interrupts enabled, they cannot be used in interrupt handlers.
    - rischio di deadlock in quanto gli interrupt handler è facile che disabilitano gli interrupt
- Because acquiresleep may yield the CPU, sleep-locks cannot be used inside spinlock critical sections (though spinlocks can be used inside sleep-lock critical sections).
    - vedi motivi sopra; inoltre, spinlock critical sections disable interrupt e scasinano con tutti il meccanismo di sospensione e cambio di contesto


### Che lock usare?
- Spin-locks are best suited to short critical sections, since waiting for them wastes CPU time;
- sleep-locks work well for lengthy operations.