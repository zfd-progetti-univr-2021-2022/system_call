/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>
#include <sys/ipc.h>

/// Buffer usato da getcwd()
#define BUFFER_SZ 255

extern char EXECUTABLE_DIR[BUFFER_SZ];

/// Percorso file FIFO 1
#define FIFO1_PATH "/tmp/fifo1_file.txt"
/// Percorso file FIFO 2
#define FIFO2_PATH "/tmp/fifo2_file.txt"
/// mtype messaggio che contiene numero di file "sendme_"
#define CONTAINS_N 1
/// mtype messaggio che contiene prima parte del contenuto del file "sendme_"
#define CONTAINS_FIFO1_FILE_PART 2
/// mtype messaggio che contiene seconda parte del contenuto del file "sendme_"
#define CONTAINS_FIFO2_FILE_PART 3
/// mtype messaggio che contiene terza parte del contenuto del file "sendme_"
#define CONTAINS_MSGQUEUE_FILE_PART 4
/// mtype messaggio che contiene quarta parte del contenuto del file "sendme_"
#define CONTAINS_SHM_FILE_PART 5
/// mtype messaggio che contiene il messaggio di fine proveniente dal server
#define CONTAINS_DONE 6

// -- Macro suddivisione messaggi

// > 4 KB -> 4096 byte -> 1024 caratteri da 1 byte ciascuno per le 4 parti dei messaggi

/// dimensione massima del messaggio/file da inviare
#define MSG_MAX_SZ 4096
/// numero parti in cui suddividere il messaggio
#define MSG_PARTS_NUM 4
/// dimensione massima di una porzione di messaggio
#define MSG_BUFFER_SZ (MSG_MAX_SZ / MSG_PARTS_NUM)

/**
 * Rappresenta un messaggio contenente
 * una porzione del contenuto di un file,
 * il numero di file inviati dal client
 * oppure il messaggio "ok" quando il server ha ricevuto il numero di file.
*/
typedef struct msg_t {

    /// tipo del messaggio: campo usato dalla coda dei messaggi
    long mtype;

    /// PID del mittente del messaggio
    pid_t sender_pid;

    /// Percorso file
    char file_path[BUFFER_SZ+1];

    /// Contenuto messaggio
    char msg_body[MSG_BUFFER_SZ+1];

} msg_t;


/**
 * Restituisce la chiave IPC
 * ottenuta con ftok sull'eseguibile del server.
 *
 * @return key_t Chiave IPC
*/
key_t get_ipc_key();


/**
 * @brief Restituisce vero se l'array contiene tutti true
 *
 * @param arr array di booleani
 * @param len lunghezza array
 * @return true arr contiene tutti true
 * @return false arr contiene almeno un false
*/
bool arrayContainsAllTrue(bool arr[], int len);


/**
 * @brief Rende bloccante oppure non bloccante un file descriptor.
 *
 * @param fd file descriptor
 * @param blocking 0: non bloccante, 1: bloccante
 * @return int Vale 0 se fallisce
 */
int blockFD(int fd, int blocking);
