/// @file server.c
/// @brief Contiene l'implementazione del server.

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/msg.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "debug.h"

/// Percorso cartella eseguibile
char EXECUTABLE_DIR[BUFFER_SZ];

int fifo1_fd;
int fifo2_fd;
int msqid;
int shmid;
int semid;

msg_t * shm_ptr;


/**
 * @brief Chiude tutte le IPC e termina
 *
 * @param sig
 */
void SIGINTSignalHandler(int sig) {

    // chiudi FIFO1
    if (close(fifo1_fd) == -1) {
        ErrExit("[server.c:SIGINTSignalHandler] close failed");
    }

    if (unlink(FIFO1_PATH) == -1) {
        ErrExit("[server.c:SIGINTSignalHandler] unlink failed");
    }

    // chiudi e dealloca memoria condivisa
    free_shared_memory(shm_ptr);
    remove_shared_memory(shmid);

    // chiudi semafori
    semDelete(semid);

    exit(0);
}


int string_to_int(char * string) {

    uintmax_t num = strtoumax(string, NULL, 10);
    if (num == UINTMAX_MAX && errno == ERANGE) {
        ErrExit("strtoumax failed");
    }
    return num;
}


/**
 * ANNOTAZIONE: Probabilmente bisogna fare un ciclo per aspettare ogni file. Per ogni file bisogna attendere le 4 parti e poi scriverle su file in ordine.
 *
 * terminazione effettuata con SIGINT: Al termine chiudi tutte le IPC.
 *
 * @todo La ricezione dei messaggi dai vari canali dovra' essere asincrona.
*/
int main(int argc, char * argv[]) {

    // memorizza il percorso dell'eseguibile per ftok()
    if (getcwd(EXECUTABLE_DIR, sizeof(EXECUTABLE_DIR)) == NULL) {
        ErrExit("getcwd");
    }

    // imposta signal handler per gestire la chiusura dei canali di comunicazione

    if (signal(SIGINT, SIGINTSignalHandler) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    DEBUG_PRINT("Ho impostato l'handler di segnali\n");

    // -- APERTURA CANALI DI COMUNICAZIONE

    // genera due FIFO (FIFO1 e FIFO2), una coda di messaggi (MsgQueue), un
    // segmento di memoria condivisa (ShdMem) ed un set di semafori per gestire la concorrenza su
    // alcuni di questi strumenti di comunicazione.

    DEBUG_PRINT("Recuperata la chiave IPC: %x\n", get_ipc_key());

    shmid = alloc_shared_memory(get_ipc_key(), 50 * sizeof(msg_t));
    shm_ptr = (msg_t *) get_shared_memory(shmid, IPC_CREAT | S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Memoria condivisa: allocata e connessa\n");

    semid = createSemaphores(get_ipc_key(), 50);
    short unsigned int semValues[50] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    semSetAll(semid, semValues);
    DEBUG_PRINT("Semafori: creati e inizializzati\n");

    fifo1_fd = create_new_fifo(FIFO1_PATH, 'r');
    DEBUG_PRINT("Mi sono collegato alla FIFO 1\n");

    fifo2_fd = create_fifo(FIFO2_PATH, 'r');  // collegamento a fifo2
    msqid = msgget(get_ipc_key(), IPC_CREAT | S_IRUSR | S_IWUSR);


    while (true) {
        // Attendo il valore <n> dal Client_0 su FIFO1 e lo memorizzo
        msg_t n_msg;
        if (read(fifo1_fd, &n_msg, sizeof(msg_t)) == -1) {
            ErrExit("read failed");
        }

        DEBUG_PRINT("Il client mi ha inviato un messaggio che dice che ci sono %s file da ricevere\n", n_msg.msg_body);
        int n = string_to_int(n_msg.msg_body);
        DEBUG_PRINT("Tradotto in numero e' %d (teoricamente lo stesso valore su terminale)\n", n);

        // scrive un messaggio di conferma su ShdMem
        msg_t received_msg = {.msg_body = "OK", .mtype = CONTAINS_N, .sender_pid = getpid()};

        semWait(semid, 0);
        // zona mutex
        shm_ptr[0] = received_msg;
        // fine zona mutex
        semSignal(semid, 0);
        DEBUG_PRINT("Ho mandato al client il messaggio di conferma.\n");


        // si mette in ricezione ciclicamente su ciascuno dei quattro canali
        int finished_files = 0;

        // NOTA: non so come gestire questa parte di sincronizzazione e attesa di tutti i file e per ogni file di ogni parte...
        // > bisogna trovare il modo di riconoscere e memorizzare quali messaggi corrispondono a quali file
        while (finished_files < n) {
            //while (true) {
                // memorizza il PID del processo mittente, il nome del file con percorso completo ed il pezzo
                // di file trasmesso
                msg_t supporto1,supporto2,supporto3,supporto4;
                //leggo da fifo1 la prima parte del file
                read(fifo1_fd,&supporto1,sizeof(supporto1));
                printf("[Parte1, del file %s spedita dal processo %d tramite FIFO1]\n%s\n",supporto1.file_path,supporto1.sender_pid,supporto1.msg_body);
                //leggo da fifo2 la seconda parte del file
                read(fifo2_fd,&supporto2,sizeof(supporto2));
                printf("[Parte2, del file %s spedita dal processo %d tramite FIFO2]\n%s\n",supporto2.file_path,supporto2.sender_pid,supporto2.msg_body);
                //leggo dalla coda di messaggi la terza parte del file
                msgrcv(msqid,&supporto3,sizeof(struct msg_t)-sizeof(long),CONTAINS_MSGQUEUE_FILE_PART,0);
		        printf("[Parte3, del file %s spedita dal processo %d tramite MsgQueue]\n%s\n",supporto3.file_path,supporto3.sender_pid,supporto3.msg_body);
		        //leggo dalla memoria condivisa
		        for(int i=0; i<50; i++){
			        if(shm_ptr[i].sender_pid==supporto1.sender_pid){//se trovo corrispondenza
				    supporto4=shm_ptr[i];
				    break;
			        }							
		        }
    		    printf("[Parte4, del file %s spedita dal processo %d tramite ShdMem]\n%s\n",supporto4.file_path,supporto4.sender_pid,supporto4.msg_body);

                // una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e l

                // salva le 4 parti in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
                // bianca (carattere newline) ed ha l’intestazione
                // > Il file verrà chiamato con lo stesso nome e percorso del file originale ma con l'aggiunta del postfisso "_out"
            //}

            finished_files++;
        }

        // quando ha ricevuto e salvato tutti i file invia un messaggio di terminazione sulla coda di
        // messaggi, in modo che possa essere riconosciuto da Client_0 come messaggio


        // si rimette in attesa su FIFO 1 di un nuovo valore n tornando all'inizio del ciclo
        DEBUG_PRINT("\n");
        DEBUG_PRINT("==========================================================\n");
        DEBUG_PRINT("\n");
    }

    return 0;
}
