The xv6 file system provides __data files__, which contain __uninterpreted byte arrays__, and directories, which contain named references to data files and other directories.

### mknod
mknod creates a special file that refers to a __device__. Associated with a device file are the major and minor device numbers (the two arguments to mknod), which uniquely identify a kernel device. When a process later opens a device file, the kernel diverts read and write system calls to the kernel device implementation instead of passing them to the file system.

### links
__A file’s name is distinct from the file itself__; the same underlying file, called an __inode__, can have multiple names, called links. Each link consists of an entry in a directory; the entry contains a file name and a reference to an inode. An inode holds metadata about a file, including its type (file or directory or device), its length, the location of the file’s content on disk, and the number of links to a file. Each inode is identified
by a unique __inode number__.

### idiomatic way to create a temporary file

    fd = open("/tmp/xyz", O_CREATE|O_RDWR);
    unlink("/tmp/xyz");

This is an idiomatic (pratica comune ritenuta corretta) way to create a temporary inode with no name that will be __cleaned up when the process closes fd or exits__. Siccome il file non ha nome, nessun che non abbia accesso al fd puo accedere al file