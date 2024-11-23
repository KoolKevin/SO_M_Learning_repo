#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene i wrapper alle system call e a varie chiamate di funzione utili

int main() {
    printf("Hello world! Sono riuscito a compilare ed eseguire il mio primo programma dentro a xv6\n");
    return 0;
}