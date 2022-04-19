
#pragma once

#define NUM 100

#include <sys/types.h>

/**
 * @brief Ottiene l'id o crea una memoria condivisa.
 *
 * @param shmKey Chiave IPC della memoria condivisa
 * @param size Dimensione della memoria condivisa
 * @return int Identificativo della memoria condivisa
*/
int alloc_shared_memory(key_t shmKey, size_t size);


/**
 * @brief Recupera il puntatore alla memoria condivisa aggiungendola allo spazio degli indirizzi.
 *
 * @param shmid Identificativo della memoria condivisa
 * @param shmflg Flag
 * @return void* Puntatore per accedere alla memoria condivisa
*/
void * get_shared_memory(int shmid, int shmflg);


/**
 * @brief Scollega la memoria condivisa dallo spazio degli indirizzi.
 *
 * @param ptr_sh Puntatore per accedere alla memoria condivisa
*/
void free_shared_memory(void *ptr_sh);


/**
 * @brief Elimina la memoria condivisa
 *
 * @param shmid Identificativo della memoria condivisa
*/
void remove_shared_memory(int shmid);


/**
 * Union usata dalla system call semctl().
*/
union semun {
    /// usato se si lavora su un singolo semaforo.
    /// Usato dalla operazione SETVAL
    int val;

    /// usato per lavorare sullo stato globale del semaforo.
    /// Usato dalle operazioni IPC_STAT e IPC_SET
    struct semid_ds * buf;

    /// per eseguire operazioni su tutti i semafori.
    /// Usato dalle operazioni GETALL e SETALL
    unsigned short * array;
};


/**
 * @brief Ottiene un insieme di semafori gia' creato
 *
 * @param key Chiave IPC
 * @param n_sem Numero semafori da ottenere/creare
*/
int getSemaphores(key_t key, int n_sem);

/**
 * @brief Crea un insieme di semafori
 *
 * @param key Chiave IPC
 * @param n_sem Numero semafori da ottenere/creare
*/
int createSemaphores(key_t key, int n_sem);


/**
 * @brief Funzione di supporto per manipolare i valori di un set di semafori.
 *
 * @param semid Identificatore del set di semafori
 * @param sem_num Indice di un semaforo nel set
 * @param sem_op Operazione eseguita sul semaforo sem_num
*/
void semOp (int semid, unsigned short sem_num, short sem_op);


/**
 * @brief Attende che il semaforo sem_num raggiunga il valore zero.
 *
 * @param semid Identificatore del set di semafori
 * @param sem_num Indice di un semaforo nel set
*/
void semWaitZero(int semid, int sem_num);


/**
 * @brief Esegue la wait sul semaforo sem_num: decrementa il valore di 1 ed eventualmente mette in attesa il processo.
 *
 * @param semid Identificatore del set di semafori
 * @param sem_num Indice di un semaforo nel set
*/
void semWait(int semid, int sem_num);


/**
 * @brief Esegue la signal sul semaforo sem_num: incrementa il valore di 1.
 *
 * @param semid Identificatore del set di semafori
 * @param sem_num Indice di un semaforo nel set
*/
void semSignal(int semid, int sem_num);


/**
 * @brief Inizializza il valore del semaforo sem_num al valore val.
 *
 * @param semid Identificatore del set di semafori
 * @param sem_num Indice di un semaforo nel set
 * @param val Valore a cui impostare il semaforo sem_num
*/
void semSetVal(int semid, int sem_num, int val);


/**
 * @brief Inizializza i valori del set di semafori semid ai valori in values.
 *
 * @param semid Identificatore del set di semafori
 * @param values Array di valori a cui impostare ogni semaforo del set
*/
void semSetAll(int semid, short unsigned int values[]);


/**
 * @brief Cancella il set di semafori svegliando eventuali processi in attesa.
 *
 * @param semid Identificatore del set di semafori
 */
void semDelete(int semid);


/**
 * @brief Recupera statistiche di un set di semafori
 *
 * @param semid Identificatore del set di semafori
 * @return struct semid_ds Statistiche del set di semafori
*/
struct semid_ds semGetStats(int semid);


/**
 * @brief Imposta i permessi su un set di semafori.
 *
 * @param semid Identificatore del set di semafori
 * @param arg Contiene i permessi da impostare
*/
void semSetPerm(int semid, struct semid_ds arg);

void ErrExit(char * msg);
