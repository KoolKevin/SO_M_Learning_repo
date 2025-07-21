Xv6 multiplexes by switching each CPU from one process to another in two situations:
1. First, xv6’s sleep and wakeup mechanism switches when a process makes a **system call that blocks** (has to wait for an event)
    - typically in read, wait, or sleep.
2. Second, xv6 periodically forces a switch to cope with processes that compute for long periods without blocking

Xv6 has separate threads (saved registers and stacks) in which to execute the scheduler because it is not safe for the scheduler to execute on any process’s kernel stack: some other CPU might wake the process up and run it, and it would be a disaster to use the same stack on two different CPUs
- si sarebbe potuto avere come alternativa uno stato aggiuntivo del tipo SWITCHING?




**The scheduler exists in the form of a special thread per CPU**, each running the scheduler function.


A process that wants to give up the CPU must **acquire its own process lock p->lock** so it can update its own state (p->state), and release any other locks it is holding so other. Lasly, it needs to call sched() to switch to the scheduler thread.



We just saw that xv6 holds p->lock across calls to swtch: the caller of swtch must already hold the lock, and control of the lock passes to the switched-to code. This arrangement is unusual: it’s more common for the thread that acquires a lock to also release it. 

Xv6’s context switching must break this convention because p->lock protects invariants on the process’s state and context fields that are not true while executing in swtch. For example, if p->lock were not held during swtch, a different CPU might decide to run the process after yield had set its state to RUNNABLE, but before swtch caused it to stop using its own kernel stack. The result would be two CPUs running on the same stack, which would cause chaos. 

Once yield has started to modify a running process’s state to make it RUNNABLE, p->lock must remain held until the invariants are restored: the earliest correct release point is after scheduler (running on its own stack) clears c->proc.

Similarly, once scheduler starts to convert a RUNNABLE process to RUNNING, the lock cannot be released until the process’s kernel thread is completely running (after the swtch, for example in yield).





The only place a kernel thread gives up its CPU is in sched, and it always switches to the same location in scheduler, which (almost) always switches to some kernel thread that previously called sched.
- l'unica eccezione è al primo scheduling di un processo che, per forza di cose, non ha ceduto la CPU con sched 

There is one case when the scheduler’s call to swtch does not end up in sched(). allocproc() sets the context ra register of a new process to forkret() (kernel/proc.c:524), so that its first swtch “returns” to the start of that function. forkret exists to release the p->lock; otherwise, since the new process needs to return to user space as if returning from fork, it could instead start at usertrapret.