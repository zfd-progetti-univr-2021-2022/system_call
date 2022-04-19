#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <stdio.h>

#include "common.h"

int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        ErrExit("shmget failed");
    }

    return shmid;
}


void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    int *ptr_sh = (int *) shmat(shmid, NULL, shmflg);

    if (ptr_sh == (int *) -1){
        ErrExit("shmat failed");
    }

    return ptr_sh;
}


void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}


void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("shmctl failed");
}


int createSemaphores(key_t key, int n_sem) {
    int semid = semget(key, n_sem, IPC_CREAT | S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("semget failed");
    return semid;
}

int getSemaphores(key_t key, int n_sem) {
    int semid = semget(key, n_sem, S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("semget failed");
    return semid;
}


void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        ErrExit("semop failed");
}


void semWaitZero(int semid, int sem_num) {
    semOp(semid, sem_num, 0);
}


void semWait(int semid, int sem_num) {
    semOp(semid, sem_num, -1);
}


void semSignal(int semid, int sem_num) {
    semOp(semid, sem_num, 1);
}


void semSetVal(int semid, int sem_num, int val) {
    union semun arg;
    arg.val = val;

    if (semctl(semid, sem_num, SETVAL, arg) == -1) {
        ErrExit("semctl SETVAL");
    }
}


void semSetAll(int semid, short unsigned int values[]) {
    union semun arg;
    arg.array = values;

    // Inizializza il set di semafori
    if (semctl(semid, 0/*semnum: ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL");
}


void semDelete(int semid) {
    if (semctl(semid, 0/*semnum: ignored*/, IPC_RMID, 0/*arg: ignored*/) == -1) {
        ErrExit("semctl failed");
    }
}


struct semid_ds semGetStats(int semid){
    struct semid_ds arg;

    if (semctl(semid, 0 /* semnum: ignored */, IPC_STAT, arg) == -1){
        ErrExit("semctl failed");
    }

    return arg;
}


void semSetPerm(int semid, struct semid_ds arg) {
    if (semctl(semid, 0 /*semnum: ignored*/, IPC_SET, arg) == -1)
        ErrExit("semctl IPC_SET failed");
}

void ErrExit(char * msg){
    perror(msg);
    exit(1);
}
