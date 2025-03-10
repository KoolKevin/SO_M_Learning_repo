#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int main() {
    printf("Hello world! Sono riuscito a compilare ed eseguire il mio primo programma dentro a xv6\n");

    hello(); // la mia syscall

    interrupt_off();
    if(fork() != 0) {
        procdump(1);
        interrupt_on();
    }

    return 0;
}