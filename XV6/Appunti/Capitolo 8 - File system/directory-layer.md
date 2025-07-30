A directory is implemented internally much like a file. **Its inode has type T_DIR** and **its data is a sequence of directory entries**. Each entry is a _struct dirent_, which contains a name and an inode number.
- **The name is at most DIRSIZ (14) characters**; if shorter, it is terminated by a NULL (0) byte.
- Directory entries with inode number zero are free.

In conclusione, una directory è un file come tutti gli altri, ma
- il suo inode ha tipo T_DIR
- il suo contenuto è una serie di coppie (inode, nome file)



### funzioni principali
- dirlookup() 
    - The function dirlookup searches a directory for an entry with the given name.  If it finds one, it returns a pointer to the corresponding inode
        - the inode could be that of a file or a directory
    - it also sets *poff to the byte offset of the entry within the directory, in case the caller wishes to edit it.

- dirlink()
    - Write a new directory entry (name, inum) into the directory dp.