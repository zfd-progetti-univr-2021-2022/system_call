#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>

#include "common.h"

union semun {
    // usato se si lavora su un singolo semaforo.
    // usato dalla operazione SETVAL
    int val;
    // usato per lavorare sullo stato globale del semaforo.
    // usato dalle operazioni IPC_STAT e IPC_SET
    struct semid_ds * buf;
    // per eseguire operazioni su tutti i semafori.
    // usato dalle operazioni GETALL e SETALL
    unsigned short * array;
};


void signalHandler(int sig){
    printf("done\n");


    semDelete(semid);
    shmDetachAndDelete(shmid, shm_pointer);

    exit(0);
}


void errExit(char * msg){
    perror(msg);
    exit(1);
}


key_t getKey() {
    key_t key = ftok("process_a", 'a');
    if (key == -1) {
        errExit("[shared_memory.c:get_shdmem_key] ftok failed");
    }

    return key;
}


void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1) {
        errExit("semop failed");
    }
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
        errExit("semctl SETVAL");
    }
}


void semSetAll(int semid, short unsigned int values[]) {
    union semun arg;
    arg.array = values;

    // Inizializza il set di semafori
    if (semctl(semid, 0/*semnum: ignored*/, SETALL, arg) == -1)
        errExit("semctl SETALL");
}


void semDelete(int semid) {
    if (semctl(semid, 0/*semnum: ignored*/, IPC_RMID, 0/*arg: ignored*/) == -1) {
        errExit("semctl failed");
    }
}


struct semid_ds semGetStats(int semid){
    struct semid_ds arg;

    if (semctl(semid, 0 /* semnum: ignored */, IPC_STAT, arg) == -1){
        errExit("semctl failed");
    }

    return arg;
}


void semSetPerm(int semid, struct semid_ds arg) {
    if (semctl(semid, 0 /*semnum: ignored*/, IPC_SET, arg) == -1)
        errExit("semctl IPC_SET failed");
}


int getShmId() {
    key_t key = getKey();

    int shmid = shmget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

    if (shmid == -1) {
        errExit("[shared_memory.c:get_shdmem_id] semget failed");
    }

    return shmid;
}


void shmDetach(int * shm_pointer) {
    if (shmdt(shm_pointer) == -1)
        errExit("shmdt failed");
}

void shmDelete(int shmid){
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl failed");
}

void shmDetachAndDelete(int shmid, int * shm_pointer){
    shmDetach(shm_pointer);
    shmDelete(shmid);
}
