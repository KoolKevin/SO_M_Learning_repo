#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

// int global_var = 10;

void print_free_pages() {
    int free_pages = freemem();
    int free_mem = (free_pages*4096)/(1024*1024); // la CPU corrente non supporta float
    printf("\tattualmente ci sono %d pagine libere, corrispondenti a %d MiB\n", free_pages, free_mem);
}

int main() {
    int status, pid, start, elapsed;

    print_free_pages();

    printf("alloco 1000 pagine (4MiB)!\n");
    int* array = (int*)sbrk(1000*4096);
    array[500] = 2;
    // coredump();
    print_free_pages();

    printf("\n----- FORK -----\n");
    start = uptime();
    pid=fork();
    // elapsed = uptime() - start;
    if(pid == 0) {
        sleep(20); // ~2 secondi
        char *argv[] = { "echo", "[FIGLIO] ho eseguito exec! Della memoria di mio padre non me ne faccio niente!\n", 0 };
        exec("echo", argv);
        exit(-1);
    } 
    else {
        elapsed = uptime() - start;
        print_free_pages();
        printf("la fork ci ha messo %d tick a ritornare\n", elapsed);

        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
        print_free_pages();
    }





    printf("\n----- FORK_COW -----\n");
    start = uptime();
    pid=fork_cow();
    // elapsed = uptime() - start;
    if(pid == 0) {
        sleep(20); // 2 secondi
        char *argv[] = { "echo", "[FIGLIO] ho eseguito exec! Della memoria di mio padre non me ne faccio niente!\n", 0 };
        exec("echo", argv);
        exit(-1);
    } 
    else {
        elapsed = uptime() - start;
        print_free_pages();
        printf("la fork_cow ci ha messo %d tick a ritornare\n", elapsed);

        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
        print_free_pages();
    }

    printf("\n");

    return 0;
}