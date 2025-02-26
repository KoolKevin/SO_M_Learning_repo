#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int main() {
    printf("Hello world! Sono riuscito a compilare ed eseguire il mio primo programma dentro a xv6\n");

    hello(); // la mia syscall
     
    return 0;
}