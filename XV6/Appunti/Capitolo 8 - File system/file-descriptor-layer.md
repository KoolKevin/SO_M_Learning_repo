### ripasso FD
A file descriptor **is a small integer** representing a kernel-managed object that a process may read from or write to.
- in xv6 fd's are the indexes to entries in the process open file table 

A process may obtain a file descriptor by opening a file, directory, or device, or by creating a pipe, or by duplicating an existing descriptor.
 
The file descriptor interface abstracts away the differences between files, pipes, and devices, making them all look like streams of bytes.

### Premessa 
questo sembra essere pensato più per i user processes che per il kernel


### fd-layer
A cool aspect of the Unix interface is that most resources in Unix are represented as files, including devices such as the console, pipes, and of course, real files. The file descriptor layer is the layer that achieves this uniformity.

Each open file is represented by a _struct file_, which is a **wrapper around either an inode or a pipe, plus an I/O offset**
- **Each call to open creates a new open file** (a new struct file)
    - xv6 non è molto parsimonioso in questo
- All the open files in the system are kept in a global file table, the ftable.
- if multiple processes open the same file independently, **different instances** will be allocated in the system ftable
    - with indipendent I/O offsets

Xv6 gives each process its own table of open files, or file descriptors.
- the table consists of **pointers** to entries in the system ftable
- a single open file (the same struct file) can appear multiple times in one process’s file table and also in the file tables of multiple processes.
    - This would happen if one process used open to open the file and then created aliases using dup
    - or shared it with a child using fork.
- A reference count tracks the number of references to a particular open file.


### Funzioni principali
The file table has functions to
- allocate a file (filealloc)
- create a duplicate reference (filedup),
- release a reference (fileclose)
    - When a file’s reference count reaches zero, fileclose releases the underlying pipe or inode, according to the type.
- and read and write data (fileread and filewrite).

- The functions filestat, fileread, and filewrite implement the stat, read, and write operations on files