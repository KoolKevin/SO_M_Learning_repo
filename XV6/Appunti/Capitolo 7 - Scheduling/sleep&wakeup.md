Scheduling and locks help conceal the actions of one thread from another, but **we also need abstractions that help threads intentionally interact.**

For example
- the reader of a pipe in xv6 may need to wait for a writing process to produce data
- a parent’s call to wait may need to wait for a child to exit
- and a process reading the disk needs to wait for the disk hardware to finish the read

The xv6 kernel uses a mechanism called sleep and wakeup in these situations.
- Sleep allows a kernel thread to wait for a specific event;
- another thread can call wakeup to indicate that threads waiting for a specified event should resume.

Sleep and wakeup provide a relatively low-level synchronization interface. You can use them to build a higher-level synchronization mechanism called a semaphore (se ti vuoi cimentare, guardare come vengono create le pipe è una buona idea).
- sleep(chan, condition_lock) waits for an event designated by the value of chan, called the wait channel. sleep puts the calling process to sleep, releasing the CPU for other work. 
    - The condition_lock is a lock used to access the sleeping/wakeup condition data safely. It also prevents **the lost wakeup problem**: if the condition to go to sleep is met but before the call to sleep a wakeup happens, the wakeup is lost and the sleeping process goes to sleep never to return (unless another wakeup happens)  
    - The root of this problem is that the invariant that P sleeps only when s->count == 0 is violated by V running at just the wrong moment. This means that we need a **condition lock** to make the checking of the condition and the going to sleep atomic
    - The going-to-sleep process then needs to release the condition lock before suspending to avoid deadlocks (if the lock remained held the waker process would wait indefinetly)
    - after the wakeup the condition lock needs to be reacquired to modify the condition data
- wakeup(chan) wakes all processes that are in calls to sleep with the same chan (if any), causing their sleep calls to return. If no processes are waiting on chan, wakeup does nothing. 


### Example | Semaphore

```C
struct semaphore {
    struct spinlock lock;
    int count;
};

void V(struct semaphore *s) {
    acquire(&s->lock);

    s->count += 1;
    wakeup(s);
    
    release(&s->lock);
}

void P(struct semaphore *s) {
    acquire(&s->lock);
    
    while(s->count == 0)
        sleep(s, &s->lock);
    s->count -= 1;
    
    release(&s->lock);
}
```

The fact that P holds s->lock prevents V from trying to wake it up between P’s check of s->count and its call to sleep. However, **sleep must release s->lock and put the consuming process to sleep in a way that’s atomic from the point of view of wakeup**, in order to avoid lost wakeups (usiamo il lock del processo).



### Implementazioni di sleep & wakeup
- sleep mark the current process as SLEEPING and then call sched to release the CPU;
- wakeup looks for a process sleeping on the given wait channel and marks it as RUNNABLE. 

NOTA: Callers of sleep and wakeup can use any mutually convenient number as the channel. Xv6 often uses the address of a kernel data structure involved in the waiting.

**NB**: sleep acquires p->lock and only then releases lk (condition lock). As we’ll see, **the fact that sleep holds one or the other of these locks at all times is what prevents a concurrent wakeup (which must acquire and hold both) from acting**

#### Why do the locking rules for sleep and wakeup ensure that a process that’s going to sleep won’t miss a concurrent wakeup?
The going-to-sleep process holds either the condition lock or its own p->lock or both from before it checks the condition **until after it is marked SLEEPING**. The process calling wakeup holds both locks in wakeup’s loop.

Thus the waker:
- either makes the condition true before the consuming thread checks the condition;
- or the waker’s wakeup examines the sleeping thread strictly after it has been marked SLEEPING. Then wakeup will see the sleeping process and wake it up (unless something else wakes it up first).

Riassumendo:
- il condition lock mi sequenzializza le chiamate a sleep()/wakeup()
- il process lock mi permette di sospendere completamente il processo prima di eseguire una wakup concorrente evitando così un lost wakeup
- **la combinazione dei due lock mi permette di rendere il controllo della condizione di sospensione, la sospensione vera e propria, e il risveglio di un processo thread-safe**




### Nota finale sui semafori
Semaphores are often used for synchronization. **Using an explicit count as part of the abstraction avoids the “lost wakeup” problem**: there is an explicit count of the number of wakeups that have occurred. The count also avoids the spurious wakeup (while che mi fa risospendere) and thundering herd (risveglio tutti ma può procedere solo uno) problems.