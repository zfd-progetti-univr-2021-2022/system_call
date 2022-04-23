#include <stdbool.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

int main(){

    printf("Creo id per dati\n");
    int shmid = alloc_shared_memory(57, NUM*sizeof(char));
    printf("Creo id per controllo\n");
    int shm_check_id = alloc_shared_memory(58, NUM*sizeof(int));

    printf("Creo puntatore per dati\n");
    char * shm_ptr = get_shared_memory(shmid, 0);
    printf("Creo puntatore per controllo\n");
    int * shm_check_ptr = get_shared_memory(shm_check_id, 0);

    printf("Creo semaforo\n");
    int semid = createSemaphores(57, 1);

    char char_val = 'a';

    printf("Inizio ciclo\n");
    while (true) {
        printf("entro nella zona critica\n");
        semWait(semid, 0);
        for (int i = 0; i < NUM; i++) {

            if (shm_check_ptr[i] == 0) {
                printf("Trovata posizione libera: scrivo in posizione %d il carattere '%c'\n", i, char_val);
                shm_ptr[i] = char_val;
                shm_check_ptr[i] = 1;
                char_val++;
                if (char_val > 'z') {
                    char_val = 'a';
                }
                break;
            }
            else {
                printf("Posizione %d occupata, non posso scriverla\n", i);
            }
        }
        semSignal(semid, 0);
        printf("esco dalla zona critica\n");
        sleep(2);
    }
}
