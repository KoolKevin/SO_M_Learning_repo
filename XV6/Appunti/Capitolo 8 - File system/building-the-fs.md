The superblock is filled in by a separate program, called mkfs, which builds an initial file system.

The program mkfs sets the bits corresponding to the boot sector, superblock, log blocks, inode blocks, and bitmap blocks.




### cos'è fs.img
fs.img è un file come altri, ma con un **contenuto speciale**: è un'immagine binaria che rappresenta un intero file system(**\***) (come se fosse un disco formattato).
 - **NB**: file system **termine overloaded**. Qua intendiamo che fs.img contiene i dati del file system:
    - La struttura ad albero: directory, sottodirectory, file.
    - I dati veri e propri: contenuti dei file binari (init, sh, ecc.).
    - Le metainformazioni: inode, dimensioni file, permessi, ecc.
    - La bitmap dei blocchi occupati/liberi.
    - Tutto salvato in un formato binario preciso, compatibile con le strutture dati del file system xv6 (come definite nei vari header file).

In pratica, fs.img è un disco virtuale che xv6 monterà come sua root directory (/) al boot.

Puoi pensare a fs.img come a:
- un .iso (immagine di un CD)
- un .img di una scheda SD su Raspberry Pi
- o una chiavetta USB formattata e piena di file, ma salvata in un singolo file

### Cosa c'è dentro fs.img?
Grazie al programma mkfs, il file fs.img contiene:

- Un superblock (descrizione generale del file system)
- Una bitmap dei blocchi liberi
- Una tabella di inode (file, directory, metadati)
- I blocchi dei dati veri e propri (binari come /sh, /init, ecc.)
- I file dichiarati in $(UPROGS) e il file README, copiati nella root /

### Come viene usato fs.img?
Durante l’avvio, xv6 (in QEMU) **legge fs.img come se fosse il suo disco principale**.

Ecco il comando QEMU nella Makefile:

```Make
    QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
    QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
```

Questo dice a QEMU: "**Collega fs.img come disco virtuale** al bus virtio, e lascia che il kernel lo usi." A questo punto il kernel vedrà fs.img come se fosse un disco fisico. Il driver:
- legge settori da questo file
- scrive settori da questo file

Un altro esempio si ha in fsinit() quando bisogna leggere il superblock; il kernel legge infatti il blocco numero 1 da questo file (che ricordiamo essere stato scritto da mkfs) 

In sintesi:
- fs.img è un file normale, creato da mkfs/mkfs.c
- Contiene un file system xv6 già popolato con programmi e file iniziali
    - con il significato di fs spiegato sopra
- È montato come disco virtuale in xv6 durante l'avvio
- Può essere esplorato in xv6 con comandi come ls, cat, ecc.



### building fs.img
```Make
    # compila il fs builder
    mkfs/mkfs: mkfs/mkfs.c $K/fs.h $K/param.h
        gcc -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c

    # usa il fs builder per costruire fs.img
    fs.img: mkfs/mkfs README $(UPROGS)
        mkfs/mkfs fs.img README $(UPROGS)
```

These rules defines how to build the xv6 file system image, fs.img.
- Target:
    - fs.img: The file system image that will be used by QEMU as the root disk for xv6.
- Prerequisites (dependencies):
    - mkfs/mkfs – the file system image builder tool.
    - README – this file is added as a user file to the root of the file system. (opzionale)
    - \$(UPROGS) – all user programs that should be pre-installed in the file system.





### mkfs | the fs builder
siccome l'immagine del fs è un file binario come altri, la creazione dell'immagine utilizza le system call del sistema operativo host per popolare il contenuto di quest'ultima.
- un buon esempio sono le funzioni rsect/wsect che leggono/scrivono blocchi dentro al file binario immagine


### What Does mkfs.c Write Into fs.img?
1. Create and zero-fill the image:
    - It writes zeroes to the entire image (FSSIZE blocks) to ensure a clean slate.

2. Initializes and Writes the Superblock:
    - The superblock contains metadata about the file system, like the number of inodes, blocks, log blocks, etc.

3. Create Inodes:
    - The root directory inode (/) is created first.
    - Then it creates inodes for every file passed like /init, /sh, and others.
        - tutti questi file vengono aggiunti come dirent dentro alla root directory

4. Allocate Blocks:
    - It uses a bitmap to mark which blocks are in use.
    - Data blocks for files are written, and their pointers are stored in inode structures.

5. Populate File Entries:
    - It populates the data blocks of the inodes with the corresponding file contents (compiled binaries like init, sh, etc.)

6. Write Bitmap:
    - It writes the free block bitmap at the end to reflect block usage.

### Example of File Installation
You’ll often see this in mkfs.c:

```C
    iappend(&dinodes[inum], buf, size);
```
This appends binary content (read from a real file like init) to the inode in the image. These binaries are usually compiled xv6 user programs, found in the build directory.