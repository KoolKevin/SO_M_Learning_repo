The term **inode** can have one of **two related meanings**:
- It might refer to the **on-disk data structure** containing a file’s size and list of data block numbers.
- Or “inode” might refer to an **in-memory inode**, which contains a **copy of the on-disk inode** as well as **extra information** needed within the kernel.

The on-disk inodes are **packed into a contiguous area of disk** called the inode blocks. **Every inode is the same size**, so it is easy, given a number n, to find the nth inode on the disk. In fact, this number n, called the inode number or **i-number, is how inodes are identified and indexed** in the implementation.

You allocate inodes when creating a file or directory. An inode is just metadata, the data associated with the inode is stored in the data blocks. 

The kernel keeps the set of **active inodes in memory** in a table called **_itable_**; _struct inode_ is the in-memory copy of a _struct dinode_ on disk.

The kernel stores an inode in memory only if there are C pointers referring to that inode (refcounted).
- The **iget** and **iput** functions acquire and release pointers to an inode, modifying the reference count.
    - iget() provides non-exclusive access to an inode, so that **there can be many pointers to the same inode**.
- Pointers to an inode can come from file descriptors, current working directories, and transient kernel code such as exec.

The inode table only stores inodes to which kernel code or data structures hold C pointers. **Its main job is synchronizing access by multiple processes**.

The struct inode that iget returns may not have any useful content. In order to ensure it holds a copy of the on-disk inode, **code must call ilock**.
- This locks the inode so that the locking process has exlusive access to the inode
- and reads the inode from the disk, if it has not already been read.
- **iunlock** releases the lock on the inode.

Separating acquisition of inode pointers from locking **helps avoid deadlock** in some situations, for example during directory lookup. Multiple processes can hold a C pointer to an inode returned by iget, but only one process can lock the inode at a time.

Code that **modifies an in-memory inode writes it to disk with _iupdate()_**.

### inode-content



### funzioni principali
- ialloc()
    - alloca un inode sul disco e lo aggiunge alla tabella in degli inode attivi (usa iget per questo)
    - l'inode allocato è nuovo e vuoto (viene zerod-out). Non punta ad alcun contenuto e non ha links
    - restituisce un puntatore all'inode allocato
- iget()
    - incrementa il refcount di un inode se già presente nella tabella
    - se l'inode non è già presente, ricicla una entry non attiva inserendo il nuovo inode nella tabella
        - in questo caso imposta valid=0 per segnalare che l'inode non è ancora stato letto da disco ma solo allocato nella tabella
- iput()
    - contropoarte di iget(). Releases a C pointer to an inode by decrementing the reference count. 
    - if this is the last reference, the inode’s slot in the inode table is now free and can be re-used for a different inode by iget().
    - if refcount e nlink scendono a zero, libera anche l'inode su disco e i relativi data blocks chiamando itrunc() e iupdate()
    - **NB**: iput() can write to the disk (it frees stuff, vedi sopra). This means that any system call that uses the file system may write to the disk, because the system call may be the last one having a reference to the file.
    - Even calls like read() that appear to be read-only, may end up calling iput() and may write stuff to disk (pensa a file temporanei senza links da cui stai leggendo). This, in turn, means that **even read-only system calls must be wrapped in transactions if they use the file system**.
- itrunc()
    - truncates the file to zero bytes, freeing the data blocks (both direct and indirect)
- iupdate()
    - copia il contenuto di un in-memory inode, su disco
- ilock()
    - impegna il lock dell'inode
    - se non è valido, aggiorna il contenuto in memoria leggendo da disco
    - Code must lock the inode using ilock before reading or writing its metadata or content
- iunlock()
    - rilascia il lock impegnato

