#include "kernel/types.h"   // contiene definizioni di tipo usate dentra a user.h 
#include "user/user.h"      // contiene le dichiarazione dei wrapper alle system call 

int main() {
    int sem_num = acquire_sem();
    printf("impegnato il semaforo numero: %d\n", sem_num);

    set_child_priority(3);
    if(fork_cow() == 0) {
        p(sem_num);
        printf("\nSONO IL FIGLIO CON PRIORITà 3!");
        procdump(0);
        sleep(20);
        v(sem_num);

        return 0;
    } 

    set_child_priority(2);
    if(fork_cow() == 0) {
        p(sem_num);
        printf("\nSONO IL FIGLIO CON PRIORITà 2!");
        procdump(0);
        sleep(20);
        v(sem_num);

        return 0;
    } 

    set_child_priority(1);
    if(fork_cow() == 0) {
        p(sem_num);
        printf("\nSONO IL FIGLIO CON PRIORITà 1!");
        procdump(0);
        sleep(20);
        v(sem_num);

        return 0;
    } 

    if(fork_cow() == 0) {
        p(sem_num);
        printf("\nSONO IL SECONDO FIGLIO CON PRIORITà 1!");
        procdump(0);
        sleep(20);
        v(sem_num);

        return 0;
    } 

    printf("[PADRE] stato iniziale");
    procdump(0);
    
    // aspetto due secondi e poi faccio partire tutti
    sleep(20);
    v(sem_num);

    int pid, status;
    for(int i=0; i<4; i++) {
        pid = wait(&status);
        p(sem_num);
        printf("[PADRE]: ho aspettato %d -> status: %d\n", pid, status);
        v(sem_num);
    }
    

    release_sem(sem_num);

    return 0;
}