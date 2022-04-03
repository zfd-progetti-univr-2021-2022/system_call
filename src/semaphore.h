/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

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
 * @brief Funzione di supporto per manipolare i valori di un set di semafori.
 *
 * @param semid Identificatore del set di semagori
 * @param sem_num Indice di un semaforo nel set
 * @param sem_op Operazione eseguita sul semaforo sem_num
*/
void semOp (int semid, unsigned short sem_num, short sem_op);
