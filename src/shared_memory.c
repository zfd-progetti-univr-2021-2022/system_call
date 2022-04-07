/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "shared_memory.h"
#include "shared_memory.h"


int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);

    return shmid;
}


void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    int *ptr_sh = (int *) shmat(shmid, NULL, shmflg);

    return ptr_sh;
}


void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        errExit("shmdt failed");
}


void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl failed");
}
