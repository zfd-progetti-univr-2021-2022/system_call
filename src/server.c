/**
 * @file server.c
 * @brief Contiene l'implementazione del server.
 *
 * @todo Spostare le funzioni non main fuori dal file server.c ?
 * @todo Spostare cio' che non riguarda i segnali in una funzione fuori dal main ?
*/

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <fcntl.h>

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "debug.h"
#include "message_queue.h"

/// Percorso cartella eseguibile
char EXECUTABLE_DIR[BUFFER_SZ] = "";

/// file descriptor della FIFO 1
int fifo1_fd = -1;
/// file descriptor della FIFO 2
int fifo2_fd = -1;
/// identifier della message queue
int msqid = -1;
/// identifier del set di semafori
int semid = -1;
/// identifier della memoria condivisa contenente i messaggi
int shmid = -1;
/// puntatore alla memoria condivisa contenente i messaggi
msg_t * shm_ptr = NULL;
/// identifier della memoria condivisa contenente le flag cella libera/occupata
int shm_check_id = -1;
/// puntatore alla memoria condivisa contenente le flag cella libera/occupata
int * shm_check_ptr = NULL;

/// e' una matrice che per ogni riga contiene le 4 parti di un file
msg_t **matriceFile = NULL;


/**
 * @brief Chiude tutte le IPC e termina
 *
 * @param sig Intero che rappresenta il segnale catturato dalla funzione
 */
void SIGINTSignalHandler(int sig) {

    // chiudi FIFO1
    if (fifo1_fd != -1) {
        if (close(fifo1_fd) == -1) {
            ErrExit("[server.c:SIGINTSignalHandler] close FIFO1 failed");
        }

        if (unlink(FIFO1_PATH) == -1) {
            ErrExit("[server.c:SIGINTSignalHandler] unlink FIFO1 failed");
        }
    }

    // chiudi FIFO2
    if (fifo2_fd != -1) {
        if (close(fifo2_fd) == -1) {
            ErrExit("[server.c:SIGINTSignalHandler] close FIFO2 failed");
        }

        if (unlink(FIFO2_PATH) == -1) {
            ErrExit("[server.c:SIGINTSignalHandler] unlink FIFO2 failed");
        }
    }

    // chiudi e dealloca memorie condivise
    if (shm_ptr != NULL)
        free_shared_memory(shm_ptr);
    if (shmid != -1)
        remove_shared_memory(shmid);

    if (shm_check_ptr != NULL)
        free_shared_memory(shm_check_ptr);
    if (shm_check_id != -1)
        remove_shared_memory(shm_check_id);

    // chiudi coda dei messaggi
    if (msqid != -1) {
        if (msgctl(msqid, IPC_RMID, NULL) == -1){
            ErrExit("[server.c:SIGINTSignalHandler] msgctl failed");
        }
    }

    // chiudi semafori
    if (semid != -1)
        semDelete(semid);

    exit(0);
}


/**
 * Converte stringa in intero.
 *
 * @param string Stringa da convertire in intero
 * @return int Valore intero ottenuto convertendo la stringa in input
*/
int string_to_int(char * string) {

    uintmax_t num = strtoumax(string, NULL, 10);
    if (num == UINTMAX_MAX && errno == ERANGE) {
        ErrExit("strtoumax failed");
    }
    return num;
}


/**
 * Aggiunge un messaggio alla matrice buffer.
 * Il buffer verra' usato per recuperare i pezzi di file
 * quando verra' ricostruito il file di output.
 *
 * @param a Messaggio da inserire nel buffer
 * @param righe Numero di righe nella matrice
*/
void aggiungiAMatrice(msg_t a,int righe){
    bool aggiunto=false;
    for(int i=0; i<righe && aggiunto==false; i++)
        for(int j=0; j<4 && aggiunto==false; j++){
            if(strcmp(matriceFile[i][j].file_path,a.file_path)==0){
                matriceFile[i][a.mtype-2]=a;
                aggiunto=true;
            }
        }

    for(int i=0; i<righe && aggiunto==false; i++){
        //cerco la prima riga vuota
        if(matriceFile[i][0].mtype==INIZIALIZZAZIONE_MTYPE && matriceFile[i][1].mtype==INIZIALIZZAZIONE_MTYPE && matriceFile[i][2].mtype==INIZIALIZZAZIONE_MTYPE && matriceFile[i][3].mtype==INIZIALIZZAZIONE_MTYPE){
            matriceFile[i][a.mtype-2]=a;
            aggiunto=true;
        }
    }
}


/**
 * Costruisce la stringa da scrivere nel file di output.
 *
 * @param a Messaggio contenente il pezzo di file arrivato dal client
 * @return char* Stringa pronta per essere scritta su file
*/
char * costruisciStringa(msg_t a){
	char buffer[20]; // serve per convertire il pid
	sprintf(buffer, "%d", a.sender_pid);

	char * stringa = (char *)malloc((strlen(a.msg_body) + strlen(a.file_path) + strlen(buffer)+61)*sizeof(char));

	strcpy(stringa,"[Parte ");

    switch (a.mtype) {
        case CONTAINS_FIFO1_FILE_PART:
            strcat(stringa,"1, del file ");
            break;

        case CONTAINS_FIFO2_FILE_PART:
            strcat(stringa,"2, del file ");
            break;

        case CONTAINS_MSGQUEUE_FILE_PART:
            strcat(stringa,"3, del file ");
            break;

        case CONTAINS_SHM_FILE_PART:
            strcat(stringa,"4, del file ");
            break;

        default:
            break;
    }

	strcat(stringa,a.file_path);
	strcat(stringa," spedita dal processo ");
	strcat(stringa,buffer);
	strcat(stringa," tramite ");

    switch (a.mtype) {
        case CONTAINS_FIFO1_FILE_PART:
            strcat(stringa, "FIFO1]\n");
            break;
        case CONTAINS_FIFO2_FILE_PART:
            strcat(stringa, "FIFO2]\n");
            break;
        case CONTAINS_MSGQUEUE_FILE_PART:
            strcat(stringa, "MsgQueue]\n");
            break;
        case CONTAINS_SHM_FILE_PART:
            strcat(stringa, "ShdMem]\n");
            break;
        default:
            break;
    }

	strcat(stringa, a.msg_body);
    return stringa;
}


/**
 * Esegue operazioni principali del server.
 *
 * terminazione effettuata con SIGINT: Al termine chiudi tutte le IPC.
 *
 * @warning I file devono essere riuniti appena vengono ricevuti i 4 pezzi oppure va bene riunirli alla fine?
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

    shmid = alloc_shared_memory(get_ipc_key(), 53 * sizeof(msg_t));
    shm_ptr = (msg_t *) get_shared_memory(shmid, IPC_CREAT | S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Memoria condivisa: allocata e connessa\n");

    shm_check_id = alloc_shared_memory(get_ipc_key2(), 53 * sizeof(int));
    shm_check_ptr = (int *) get_shared_memory(shm_check_id, S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Memoria condivisa flag: allocata e connessa\n");

    semid = createSemaphores(get_ipc_key(), 53);
    short unsigned int semValues[53] = {1,0,0,0,0,0,1,  0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    semSetAll(semid, semValues);
    DEBUG_PRINT("Semafori: creati e inizializzati\n");

    fifo1_fd = create_fifo(FIFO1_PATH, 'r');
    DEBUG_PRINT("Mi sono collegato alla FIFO 1\n");//collegamento a fifo1

    fifo2_fd = create_fifo(FIFO2_PATH, 'r');  // collegamento a fifo2
    DEBUG_PRINT("Mi sono collegato alla FIFO 2\n");

    msqid = msgget(get_ipc_key(), IPC_CREAT | S_IRUSR | S_IWUSR);//collegamento alla coda di messaggi
    DEBUG_PRINT("Mi sono collegato alla coda dei messaggi\n");

    //limito la coda
    struct msqid_ds ds = msqGetStats(msqid);
    ds.msg_qbytes = sizeof(msg_t) * 50;
    msqSetStats(msqid, ds);

    while (true) {
        // Attendo il valore <n> dal Client_0 su FIFO1 e lo memorizzo
        msg_t n_msg;
        if (read(fifo1_fd, &n_msg, sizeof(msg_t)) == -1) {
            ErrExit("read failed");
        }

        DEBUG_PRINT("Il client mi ha inviato un messaggio che dice che ci sono '%s' file da ricevere\n", n_msg.msg_body);
        int n = string_to_int(n_msg.msg_body);
        //inizializzo la matrice contenente i pezzi di file
        matriceFile=(msg_t **)malloc(n*sizeof(msg_t *));
        for(int i=0; i<n;i++)
            matriceFile[i]=(msg_t *)malloc(4*sizeof(msg_t));

        msg_t vuoto;
        vuoto.mtype = INIZIALIZZAZIONE_MTYPE;
        //inizializzo i valori del percorso per evitare di fare confronti con null
        for (int i = 0; i < BUFFER_SZ+1; i++)
            vuoto.file_path[i] = '\0';
        //riempio la matrice con una struttura che mi dice se le celle sono vuote
        for(int i=0;i<n;i++)
            for(int j=0; j<4;j++)
                matriceFile[i][j]=vuoto;

        DEBUG_PRINT("Tradotto in numero e' %d (teoricamente lo stesso valore su terminale)\n", n);

        //inizializzazione semaforo dei figli
        for (int i=0; i < 2; i++) {
            semSignal(semid, 1);
            semSignal(semid, 2);
            semSignal(semid, 3);
            semSignal(semid, 4);
        }

        //semaforo per far attendere i figli
        for(int i=0;i<n;i++)
        	semSignalNoBlocc(semid,5);
        //semaforo per limitare la fifo1
        for(int i=0;i<50;i++)
        	semSignalNoBlocc(semid,7);
        //semaforo per limitare la fifo2
        for(int i=0;i<50;i++)
        	semSignalNoBlocc(semid,8);
        //semaforo per limitare la coda di messaggi
        for(int i=0;i<50;i++)
        	semSignalNoBlocc(semid,9);

        // scrive un messaggio di conferma su ShdMem
        msg_t received_msg = {.msg_body = "OK", .mtype = CONTAINS_N, .sender_pid = getpid()};

        semWait(semid, 0);
        // zona mutex
        shm_ptr[0] = received_msg;
        // fine zona mutex
        semSignal(semid, 0);
        DEBUG_PRINT("Ho mandato al client il messaggio di conferma.\n");

        // rendi fifo non bloccanti
        DEBUG_PRINT("Rendi fifo non bloccanti\n");
        semWait(semid, 1);
        semWaitZero(semid, 1);
        blockFD(fifo1_fd, 0);
        blockFD(fifo2_fd, 0);
        semWait(semid, 2);
        semWaitZero(semid, 2);
        DEBUG_PRINT("Rese fifo non bloccanti\n");

        // si mette in ricezione ciclicamente su ciascuno dei quattro canali
        int arrived_parts_counter = 0;
        int n_tries = 0;
        while (arrived_parts_counter < n * 4) {

            // memorizza il PID del processo mittente, il nome del file con percorso completo ed il pezzo
            // di file trasmesso
            msg_t supporto1, supporto2, supporto3; // ,supporto4;

            //leggo da fifo1 la prima parte del file
            if (read(fifo1_fd,&supporto1,sizeof(supporto1)) != -1) {
                DEBUG_PRINT("[Parte1, del file %s spedita dal processo %d tramite FIFO1]\n%s\n",supporto1.file_path,supporto1.sender_pid,supporto1.msg_body);
                semSignalNoBlocc(semid,7);
                aggiungiAMatrice(supporto1,n);
                arrived_parts_counter++;
            }

            //leggo da fifo2 la seconda parte del file
            if (read(fifo2_fd,&supporto2,sizeof(supporto2)) != -1) {
                DEBUG_PRINT("[Parte2,del file %s spedita dal processo %d tramite FIFO2]\n%s\n",supporto2.file_path,supporto2.sender_pid,supporto2.msg_body);
                semSignalNoBlocc(semid,8);
                aggiungiAMatrice(supporto2,n);
                arrived_parts_counter++;
            }

            //leggo dalla coda di messaggi la terza parte del file

            if (msgrcv(msqid,&supporto3,sizeof(struct msg_t)-sizeof(long),CONTAINS_MSGQUEUE_FILE_PART, IPC_NOWAIT) != -1) {
                DEBUG_PRINT("[Parte3,del file %s spedita dal processo %d tramite MsgQueue]\n%s\n",supporto3.file_path,supporto3.sender_pid,supporto3.msg_body);
                semSignalNoBlocc(semid,9);
                aggiungiAMatrice(supporto3,n);
                arrived_parts_counter++;
            }

            // leggi dalla memoria condivisa
            DEBUG_PRINT("Tenta di entrare nella memoria condivisa\n");
            semWait(semid, 6);
            DEBUG_PRINT("Sono entrato nella memoria condivisa\n");
            for (int i = 0; i < 50; i++) {
                if (shm_check_ptr[i] == 1) {
                    DEBUG_PRINT("Trovata posizione da leggere %d, messaggio: '%s'\n", i, shm_ptr[i].msg_body);
                    shm_check_ptr[i] = 0;
                    aggiungiAMatrice(shm_ptr[i],n);
                    arrived_parts_counter++;
                }
            }
            DEBUG_PRINT("Tenta di uscire nella memoria condivisa\n");
            semSignal(semid, 6);
            DEBUG_PRINT("Sono uscito dalla memoria condivisa\n");

            // una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e
            // salva le 4 parti in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
            // bianca (carattere newline) ed ha l’intestazione
            // > Il file verrà chiamato con lo stesso nome e percorso del file originale ma con l'aggiunta del postfisso "_out"


            if (n_tries % 5000 == 0) {
		        DEBUG_PRINT("Ancora un altro tentativo... Counter = %d\n", arrived_parts_counter);
            }
            n_tries++;
        }

        for(int i=0;i<n;i++){
            char *temp = (char *)malloc((strlen(matriceFile[i][0].file_path)+5)*sizeof(char)); // aggiungo lo spazio per _out
            if (temp == NULL){
                printf("[server.c:main] malloc failed\n");
                exit(1);
            }
            strcpy(temp, matriceFile[i][0].file_path);
            strcat(temp, "_out"); // aggiungo -out
            int file = open(temp, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

            if (file == -1) {
                ErrExit("open failed");
            }

            for(int j=0; j<4;j++){
                char * stampa = costruisciStringa(matriceFile[i][j]);
                if (write(file, stampa, strlen(stampa) * sizeof(char)) == -1){
                    ErrExit("write output file failed");
                }

                if (write(file, "\n\n", 2) == -1){
                    ErrExit("write newline to output file failed");
                }
                free(stampa);
            }

            close(file);
            free(temp);
        }

        // quando ha ricevuto e salvato tutti i file invia un messaggio di terminazione sulla coda di
        // messaggi, in modo che possa essere riconosciuto da Client_0 come messaggio
        DEBUG_PRINT("Invio messaggio di fine al client\n");
        msg_t end_msg = {.msg_body = "DONE", .mtype = CONTAINS_DONE, .sender_pid=getpid()};
        msgsnd(msqid, &end_msg, sizeof(struct msg_t)-sizeof(long), 0);
        DEBUG_PRINT("Inviato messaggio di fine al client\n");

        // rendi fifo bloccanti
        DEBUG_PRINT("Rendi fifo bloccanti\n");
        semWait(semid, 3);
        semWaitZero(semid, 3);
        blockFD(fifo1_fd, 1);
        blockFD(fifo2_fd, 1);
        semWait(semid, 4);
        semWaitZero(semid, 4);
        DEBUG_PRINT("Rese fifo bloccanti\n");

        // libera memoria della matrice buffer
        for(int i = 0; i < n; i++){
            free(matriceFile[i]);
        }
        free(matriceFile);

        // si rimette in attesa su FIFO 1 di un nuovo valore n tornando all'inizio del ciclo
        DEBUG_PRINT("\n");
        DEBUG_PRINT("==========================================================\n");
        DEBUG_PRINT("\n");
    }

    return 0;
}
