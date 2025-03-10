#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

/* 
    Eseguire con un solo core!
    Altrimenti gli altri scheduler(che hanno interrupt attivi 
    dato che hanno CSR separati) cominciano ad eseguire 
    immediatamente i figli
 */
int main() {
    interrupt_off(); // disabilito per evitare la preemption

    set_child_priority(3);
    if(fork() == 0) {
        printf("sono il figlio 3\n");
        procdump(0);
        return 0;
    } 

    set_child_priority(2);
    if(fork() == 0) {
        printf("sono il figlio 2\n");
        procdump(0);
        return 0;
    } 

    set_child_priority(1);
    if(fork() == 0) {
        printf("sono il figlio 1\n");
        procdump(0);
        return 0;
    } 

    printf("sono il padre\n");
    procdump(0);
    interrupt_on();
    
    return 0;
}