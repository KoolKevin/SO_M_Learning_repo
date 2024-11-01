The first question one might ask when encountering an operating system is why have it at all? That is, one could implement the system calls in Figure 1.2 as a library, with which applications link. In this plan, each application could even have its own library tailored to its needs. Applications could
directly interact with hardware resources and use those resources in the best way for the application (e.g., to achieve high or predictable performance). Some operating systems for embedded devices or real-time systems are organized in this way.

The downside of this library approach is that, __if there is more than one application running, the applications must be well-behaved__. For example, each application must periodically give up the CPU so that other applications can run. Such a cooperative time-sharing scheme may be OK if all applications trust each other and have no bugs. It’s more typical for applications to not trust each other, and to have bugs, so __one often wants stronger isolation__(tra le applicazioni) than a cooperative scheme provides.

__To achieve strong isolation it’s helpful to forbid applications from directly accessing sensitive hardware resources, and instead to abstract the resources into services__. For example, Unix applications interact with storage only through the file system’s open, read, write, and close system
calls, instead of reading and writing the disk directly. This provides the application with the convenience of pathnames (abstraction), and it allows the operating system (as the implementer of the interface) to manage the disk (isolation through delega).

Even if isolation is not a concern, programs that interact intentionally (or just wish to keep out of each other’s way) are likely to find a file system a more convenient abstraction than direct use of the disk.

As another example, Unix processes use exec to build up their memory image, instead of directly interacting with physical memory. __This allows the operating system__ to decide where to place a process in memory; if memory is tight, the operating system might even store some of a process’s data on disk. exec also provides users with the convenience of a file system to store executable program images.

    The system-call interface in Figure 1.2 is carefully designed to provide both programmer CONVENIENCE and the possibility of STRONG ISOLATION.
    
The Unix interface is not the only way to abstract resources, but it has proved to be a good one.