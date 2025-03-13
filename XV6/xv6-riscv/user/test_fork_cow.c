#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

// int global_var = 10;

int main() {
    int free_pages = freemem();
    int free_mem = (free_pages*4096)/(1024*1024); // la CPU corrente non supporta float
    printf("attualmente ci sono %d pagine libere, corrispondenti a %d MiB\n", free_pages, free_mem);

    printf("divento enorme allocando 1 milione di interi (4MiB)!!!");
    int* array = (int*)sbrk(1000000*sizeof(int));
    array[500] = 2;
    coredump();

    free_pages = freemem();
    free_mem = (free_pages*4096)/(1024*1024);
    printf("attualmente ci sono %d pagine libere, corrispondenti a %d MiB\n", free_pages, free_mem);



    int status, pid, start, elapsed;



    printf("\n----- FORK -----\n");
    start = uptime();
    pid=fork();
    
    if(pid == 0) {
        sleep(2);
        exit(0);
    } 
    else {
        elapsed = uptime() - start;

        free_pages = freemem();
        free_mem = (free_pages*4096)/(1024*1024);
        printf("attualmente ci sono %d pagine libere, corrispondenti a %d MiB\n", free_pages, free_mem);
        printf("la fork ci ha messo %d tick a ritornare\n", elapsed);

        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
    }





    printf("\n----- FORK_COW -----\n");
    start = uptime();
    pid=fork_cow();
    
    if(pid == 0) {
        sleep(2);
        exit(0);
    } 
    else {
        elapsed = uptime() - start;

        free_pages = freemem();
        free_mem = (free_pages*4096)/(1024*1024);
        printf("attualmente ci sono %d pagine libere, corrispondenti a %d MiB\n", free_pages, free_mem);
        printf("la fork_cow ci ha messo %d tick a ritornare\n", elapsed);

        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
    }

    return 0;
}