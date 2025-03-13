#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int global_var = 10;

int main() {
    int status, pid;
    int stack_var = 20;

    if(fork_cow() == 0) {
        printf("[FIGLIO]: scrivo sul mio stack!\n");
        stack_var++;

        printf("[FIGLIO]: global_var=%d, stack_var=%d\n", global_var, stack_var);
        coredump();
        printf("[FIGLIO]: termino! Le mie pagine ora avranno un riferimento in meno\n");
    } 
    else {
        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status); 
        printf("[PADRE]: scrivo nella mia area dati!\n");
        global_var++;

        printf("[PADRE]: global_var=%d, stack_var=%d\n", global_var, stack_var);
        coredump();
    }

    return 0;
}