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
        printf("\nSONO IL FIGLIO CON PRIORITà 3!");
        procdump(0);
        return 0;
    } 

    set_child_priority(2);
    if(fork() == 0) {
        printf("\nSONO IL FIGLIO CON PRIORITà 2!");
        procdump(0);
        return 0;
    } 

    set_child_priority(1);
    if(fork() == 0) {
        printf("\nSONO IL FIGLIO CON PRIORITà 1!");
        procdump(0);
        return 0;
    } 


    printf("[PADRE] stato iniziale");
    procdump(0);
    interrupt_on();
    
    int pid, status;
    for(int i=0; i<3; i++) {
        pid = wait(&status);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
    }
    
    
    return 0;
}