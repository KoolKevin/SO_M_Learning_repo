✅ Quando diciamo che fs.img "contiene un file system già popolato"...
📂 Significa solo che contiene i dati del file system:
La struttura ad albero: directory, sottodirectory, file.

I dati veri e propri: contenuti dei file binari (init, sh, ecc.).

Le metainformazioni: inode, dimensioni file, permessi, ecc.

La bitmap dei blocchi occupati/liberi.

Tutto salvato in un formato binario preciso, compatibile con le strutture del file system xv6 (come definite in fs.h, fs.c).

👉 Questo è quello che mkfs.c scrive fisicamente dentro fs.img.

❌ Non contiene il "codice" del file system
Cioè, fs.img non contiene:

Le funzioni C per gestire inode, cache, journaling, lock, ecc.

Nessun comportamento attivo: è solo dati passivi.

In altre parole: non contiene il codice di fs.c, bio.c, log.c, sleeplock.c ecc.

👉 Quel codice viene compilato e incluso nel kernel (kernel), ed è il responsabile di leggere e scrivere dentro fs.img a runtime, simulando un vero file system.

📌 Quindi:
Cosa	Dove si trova
📁 File e directory (root, /sh, /init, ecc.)	Dentro fs.img
⚙️ Funzioni per leggere/scrivere file (es: readi, writei, bwrite, ialloc)	Dentro il kernel (fs.c, bio.c, ecc.)
💬 Comandi ls, cat, sh	Come programmi utente, salvati dentro fs.img
🧠 Inizializzazione del file system	In mkfs.c (build time) e fsinit() (runtime nel kernel)
