/// @file server.c
/// @brief Contiene l'implementazione del server.

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "debug.h"

int fifo1_fd;

/**
 * @brief Chiude tutte le IPC e termina
 *
 * @param sig
 */
void SIGINTSignalHandler(int sig) {
    exit(0);
}


/**
 * ANNOTAZIONE: Probabilmente bisogna fare un ciclo per aspettare ogni file. Per ogni file bisogna attendere le 4 parti e poi scriverle su file in ordine.
 *
 * terminazione effettuata con SIGINT: Al termine chiudi tutte le IPC.
*/
int main(int argc, char * argv[]) {

    // imposta signal handler per gestire la chiusura dei canali di comunicazione

    if (signal(SIGINT, SIGINTSignalHandler) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    DEBUG_PRINT("Ho impostato l'handler di segnali\n");

    // -- APERTURA CANALI DI COMUNICAZIONE

    // genera due FIFO (FIFO1 e FIFO2), una coda di messaggi (MsgQueue), un
    // segmento di memoria condivisa (ShdMem) ed un set di semafori per gestire la concorrenza su
    // alcuni di questi strumenti di comunicazione.

    unlink(FIFO1_PATH);
    make_fifo(FIFO1_PATH);
    fifo1_fd = create_fifo(FIFO1_PATH, 'r');
    DEBUG_PRINT("Mi sono collegato alla FIFO 1\n");

    msg_t n_msg;
    if (read(fifo1_fd, &n_msg, sizeof(msg_t)) == -1) {
        ErrExit("read failed");
    }

    DEBUG_PRINT("Il client mi ha inviato un messaggio che dice che ci sono %s file da ricevere\n", n_msg.msg_body);

    DEBUG_PRINT("Recuperata la chiave IPC: %x\n", get_ipc_key());

    int shmid = alloc_shared_memory(get_ipc_key(), 50 * sizeof(msg_t));
    msg_t * shm_ptr = (msg_t *) get_shared_memory(shmid, IPC_CREAT | S_IRUSR | S_IWUSR);

    int semid = createSemaphores(get_ipc_key(), 50);
    short unsigned int semValues[50] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    semSetAll(semid, semValues);

    msg_t received_msg = {.msg_body = "OK", .mtype = CONTAINS_N, .sender_pid = getpid()};

    semWait(semid, 0);
    // zona mutex
    shm_ptr[0] = received_msg;
    // fine zona mutex
    semSignal(semid, 0);

    DEBUG_PRINT("Ho mandato al client il messaggio di conferma.\n");

    /*
    while (true) {
        // Attendo il valore <n> dal Client_0 su FIFO1 e lo memorizzo

        // scrive un messaggio di conferma su ShdMem

        // si mette in ricezione ciclicamente su ciascuno dei quattro canali
        int finished_files = 0;

        // NOTA: non so come gestire questa parte di sincronizzazione e attesa di tutti i file e per ogni file di ogni parte...
        // > bisogna trovare il modo di riconoscere e memorizzare quali messaggi corrispondono a quali file
        while (finished_files < n) {
            while (true) {
                // memorizza il PID del processo mittente, il nome del file con percorso completo ed il pezzo
                // di file trasmesso

                // una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e l

                // salva le 4 parti in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
                // bianca (carattere newline) ed ha l’intestazione
                // > Il file verrà chiamato con lo stesso nome e percorso del file originale ma con l'aggiunta del postfisso "_out"
            }

            finished_files++;
        }

        // quando ha ricevuto e salvato tutti i file invia un messaggio di terminazione sulla coda di
        // messaggi, in modo che possa essere riconosciuto da Client_0 come messaggio

        // si rimette in attesa su FIFO 1 di un nuovo valore n tornando all'inizio del ciclo
    }
    */

    return 0;
}
