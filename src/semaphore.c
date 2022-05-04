/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <sys/stat.h>
#include <sys/sem.h>

#include "err_exit.h"
#include "semaphore.h"


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

void semOpNoBlocc (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = IPC_NOWAIT};
    semop(semid, &sop, 1);
}

void semWaitZero(int semid, int sem_num) {
    semOp(semid, sem_num, 0);
}

void semWait(int semid, int sem_num) {
    semOp(semid, sem_num, -1);
}

void semWaitNoBlocc(int semid, int sem_num) {
    semOpNoBlocc(semid, sem_num, -1);
}

void semSignal(int semid, int sem_num) {
    semOp(semid, sem_num, 1);
}

void semSignalNoBlocc(int semid, int sem_num) {
    semOpNoBlocc(semid, sem_num, 1);
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
