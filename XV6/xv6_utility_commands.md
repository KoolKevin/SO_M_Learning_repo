Avvio del container _svkv/riscv-tools:v1.0_ che contiene tutta la toolchain riscv per la compilazione e linking di xv6

L'opzione -v specifica un volume da montare dall'host dentro al container; in questo caso il volume contiene il codice sorgente xv6

    $ docker run --name my_xv6 -it -v /home/kevin/Scrivania/Git_learning_repos/SO_M_Learning_repo/XV6/xv6-riscv:/home/os-iitm/xv6-riscv svkv/riscv-tools:v1.0

Build del sistema operativo (dentro la cartella xv6-riscv del container); effettua la compilazione e il linking (da rieseguire ad ogni modifica):

    $ make qemu

Per uscire da xv6 bisogna terminare qemu, lo si fa con:

    $ ctrl+a, x

Per far ripartire il container una volta usciti:

    $ docker start my_xv6
    $ docker attach my_xv6