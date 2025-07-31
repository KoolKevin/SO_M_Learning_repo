### Instructions and memory ordering
It is natural to think of programs executing in the order in which source code statements appear. That’s a reasonable mental model for single-threaded code, but is **incorrect when multiple threads interact through shared memory**.
- One reason is that **compilers emit load and store instructions in orders different** from those implied by the source code, and may entirely **omit them** (for example by caching data in registers).
- Another reason is that the **CPU may execute instructions out of order** to increase performance. For example, a CPU may notice that in a serial sequence of instructions A and B are not dependent on each other. The CPU may start instruction B first, either because its inputs are ready before A’s inputs, or in order to overlap execution of A and B.


The good news is that **compilers and CPUs help concurrent programmers by following a set of rules called the MEMORY MODEL**, and by providing some **primitives** to help programmers control re-ordering.

To tell the hardware and compiler not to re-order, xv6 uses **__sync_synchronize()** in both acquire and release. __sync_synchronize() is a **memory barrier**: it tells the compiler and CPU to not reorder loads or stores across the barrier. 

The barriers in xv6’s acquire and release **force order** in almost all cases where it matters, since xv6 uses locks around accesses to shared data.



...


It is a potential memory ordering problem (see Chapter 6), since **without a memory barrier there’s no reason to expect one CPU to see another CPU’s writes**