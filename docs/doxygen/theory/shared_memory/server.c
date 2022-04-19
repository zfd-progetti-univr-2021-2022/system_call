#include <stdbool.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"


int main(){

    int shmid = alloc_shared_memory(57, NUM*sizeof(char));
    int shm_check_id = alloc_shared_memory(58, NUM*sizeof(int));

    char * shm_ptr = get_shared_memory(shmid, IPC_CREAT);
    int * shm_check_ptr = get_shared_memory(shm_check_id, IPC_CREAT);

    int semid = createSemaphores(57, 1);
    unsigned short int vals[NUM]={1};
    semSetAll(semid, vals);

    char char_val = 'a';

    while (true) {
        printf("entro zona critica\n");
        semWait(semid, 0);
        for (int i = 0; i < NUM; i++) {

            if (shm_check_ptr[i] == 1) {
                printf("Nuovo valore da leggere in posizione %d: '%c'\n", i, shm_ptr[i]);
                shm_check_ptr[i] = 0;
            }
            else {
                printf("Posizione %d contiene carattere gia' letto...\n", i);
            }
        }
        semSignal(semid, 0);
        printf("esco zona critica\n");

        sleep(5);
    }

    free_shared_memory(shm_ptr);
    free_shared_memory(shm_check_ptr);
    remove_shared_memory(shm_check_id);
    remove_shared_memory(shmid);
}
