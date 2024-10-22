A file descriptor is a small integer representing a kernel-managed object that a process may read from or write to. A process may obtain a file descriptor by opening a file, directory, or __device__, or by creating a __pipe__, or by duplicating an existing descriptor. For simplicity we’ll often refer to the object a file descriptor refers to as a “file”.

    The file descriptor interface abstracts away the differences between files, pipes, and devices, making them all look like streams of bytes.

Internally, the xv6 kernel uses the file descriptor as an index into a __per-process table__, so that every process has a private space of file descriptors starting at zero.

The read and write system calls read bytes from and write bytes to open files (ricorda che con file si intendono anche diversi tipi di entità) named by file descriptors.

### Read
The call read(fd, buf, n) reads at most n bytes from the file descriptor fd, copies them into buf, and returns the number of bytes read. __Each file descriptor that refers to a file has an offset associated with it__. read reads data from the current file offset and then advances that offset by the number of bytes read: a subsequent read will return the bytes following the ones returned by the first read. When there are no more bytes to read, read returns zero to indicate the end of the file.

### Write
The call write(fd, buf, n) writes n bytes from buf to the file descriptor fd and returns the number of bytes written. Fewer than n bytes are written only when an error occurs. Like read, write writes data at the current file offset and then advances that offset by the number of bytes written: each write picks up where the previous one left off.

### Close
The close system call __releases a file descriptor__, making it free for reuse by a future open, pipe, or dup system call. 

### Redirezione
File descriptors and fork interact to make I/O redirection easy to implement. __fork copies the parent’s file descriptor table along with its memory__, so that the child starts with exactly the same open files as the parent. The system call exec replaces the calling process’s memory but preserves its file table. This behavior allows the shell to implement I/O redirection by __forking, reopening chosen file descriptors in the child, and then calling exec__ to run the new program. The parent process’s file descriptors are not changed by this sequence, since it modifies only the child’s descriptors.

Now it should be clear why it is helpful that fork and exec are separate calls: between the two, the shell has a chance to redirect the child’s I/O without disturbing the I/O setup of the main shell.

__NB__: Although fork copies the file descriptor table, each underlying __file offset is shared between parent and child__ (ricorda che c'è anche una tabella di sistema nella memoria del kernel che memorizza gli I/O pointer). More generally, two file descriptors share an offset if they were derived from the same original file descriptor by a sequence of fork and dup calls. Otherwise file descriptors do not share offsets, even if they resulted from open calls for the same file.