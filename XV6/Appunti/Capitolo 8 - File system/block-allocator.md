Xv6’s block allocator maintains a **free bitmap on disk**, with one bit per block.
- A zero bit indicates that the corresponding block is free;
- a one bit indicates that it is in use.

The program **_mkfs_** sets the bits corresponding to the boot sector, superblock, log blocks, inode blocks, and bitmap blocks.
- praticamente tutti tranne i data blocks

Il resto dei blocchi viene gestito da balloc() e bfree()