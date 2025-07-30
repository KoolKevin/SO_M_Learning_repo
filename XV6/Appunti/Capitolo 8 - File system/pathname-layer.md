Path name lookup involves a **succession of calls to dirlookup**, one for each path component.


### Funzioni principali
- namei()
    - evaluates path and returns the corresponding inode.

- nameiparent()
    - is a variant of namei()
    - it stops before the last element, returning the inode of the parent directory and copying the final element into name