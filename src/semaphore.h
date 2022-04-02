/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

/**
 * @todo Commentare la union semun e i suoi attributi.
*/
union semun {
    int val;
    struct semid_ds * buf;
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
