#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>

#include "common.h"

int shmid;
int semid;
int * shm_pointer;

int main(){

    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        errExit("change signal handler failed");
    }

    shmid = getShmId();
    shm_pointer = (int *) shmat(shmid, NULL, 0);
    if (shm_pointer == (int *) -1) {
        errExit("shmat failed");
    }

    key_t key = getKey();

    semid = semget(key, 2, IPC_CREAT | S_IRUSR | S_IWUSR);

    semSetVal(semid, SEM_B, 0);
    semSetVal(semid, SEM_A, 1);

    int start = 1;
    while (1){
        semWait(semid, SEM_A);
        shm_pointer[0]++;
        printf("A: %d\n", shm_pointer[0]);
        semSignal(semid, SEM_B);
    }

    return 0;
}
