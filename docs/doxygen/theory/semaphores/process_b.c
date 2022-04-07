#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

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

    semid = semget(key, 2, S_IRUSR | S_IWUSR);

    while (1){
        semWait(semid, SEM_B);
        shm_pointer[0]++;
        printf("B: %d\n", shm_pointer[0]);
        semSignal(semid, SEM_A);
    }

    return 0;
}
