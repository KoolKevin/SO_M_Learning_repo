The buffer cache has two jobs:
1. **synchronize access to disk blocks** to ensure that only one copy of a block is in memory and that only one kernel thread at a time uses that copy; 
2. **cache popular blocks** so that they don’t need to be RE-read (almeno una volta bisogna leggerli) from the slow disk.

**The buffer cache is a linked list of _buf_ structures** holding cached copies of disk block contents. 
- Caching disk blocks in memory reduces the number of disk reads 
- and also provides a synchronization point for disk blocks used by multiple processes.


The main interface exported by the buffer cache consists of **bread and bwrite**

A kernel thread must release a buffer by calling **brelse** when it is done with it.

The buffer cache uses a **per-buffer sleep-lock** to ensure that only one thread at a time uses each buffer (and thus each disk block);
- bread returns a locked buffer, and brelse releases the lock.


The buffer cache has a fixed number of buffers to hold disk blocks, which means that if the file system asks for a block that is not already in the cache, the buffer cache must **recycle a buffer currently holding some other block**. The buffer cache recycles the **least recently used buffer** for the new block. The assumption is that the least recently used buffer is the one least likely to be used again soon.

**A buffer has two state fields** associated with it.
- _valid_: indicates that the buffer contains a copy of the block.
- _disk_: indicates that the buffer content has been handed to the **disk, which may change the buffer** (e.g., write data from the disk into data).


### Code
The buffer cache is a doubly-linked list of buffers. The function _binit()_ initializes the list with the NBUF buffers in the static array _buf_.

The linked list of all buffers is **sorted by how recently the buffer was used** so it can easily evict the contents of the last cached block if needed (vedi sopra).
- head.next is most recent, head.prev is least.

After _binit()_, **all other access to the buffer cache refer to the linked list via _bcache.head_, not the buf array.**


Once _bread_ has read the disk (if needed) and returned  the buffer to its caller, **the caller has exclusive use of the buffer** and can **read or write the its data bytes**. 

If the caller does modify the buffer, it must call **_bwrite_** to write the changed data to disk before releasing the buffer. _bwrite_ (kernel/bio.c:107) calls virtio_disk_rw to talk to the disk hardware


When the caller is done with a buffer, it must call **_brelse_** to release it. _brelse_ releases the sleep-lock and moves the buffer to the front of the linked list.