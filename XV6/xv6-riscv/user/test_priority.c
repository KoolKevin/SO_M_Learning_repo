#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int main() {
    interrupt_off(); // disabilito per evitare la preemption

    set_child_priority(1);
    
    if(fork() == 0) {
        printf("sono il figlio\n");
    } 
    else {
        printf("sono il padre\n");
        procdump();
        interrupt_on();
    }

    return 0;
}