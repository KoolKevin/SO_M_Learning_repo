#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int global_var = 10;

int main() {
    int stack_var = 20;

    if(fork_cow() == 0) {
        printf("sono il figlio e scrivo sul mio stack!\n");
        stack_var++;
    } 
    else {
        wait((void *)0);
        printf("sono il padre e scrivo nella mia area dati!\n");
        global_var++;
    }
    return 0;
}