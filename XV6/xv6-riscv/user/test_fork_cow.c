#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int main() {
    if(fork_cow() == 0) {
        procdump();
        printf("sono il figlio\n");
    } 

    wait(0);
    printf("sono il padre\n");

    return 0;
}