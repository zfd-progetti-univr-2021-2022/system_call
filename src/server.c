/// @file server.c
/// @brief Contiene l'implementazione del server.

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

/// Percorso cartella eseguibile
char EXECUTABLE_DIR[BUFFER_SZ];

int fifo1_fd;
int fifo2_fd;
int msqid;
int shmid;
int semid;
msg_t **matriceFile;//è una matrice che per ogni riga contiene le 4 parti di un file

msg_t * shm_ptr;

int shm_check_id;
int * shm_check_ptr;

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
        if(matriceFile[i][0].mtype==666 && matriceFile[i][1].mtype==666 && matriceFile[i][2].mtype==666 && matriceFile[i][3].mtype==666){
            matriceFile[i][a.mtype-2]=a;
            aggiunto=true;
        }
    }         
}
//costruisce la stringa da scrivere nel file di output
void costruisciStringa(msg_t a, char *stringa,int parte){
	char buffer[20];//serve per convertire il pid
	sprintf(buffer, "%d", a.sender_pid);
	stringa=(char *)malloc((strlen(a.msg_body)+strlen(a.file_path)+strlen(buffer)+61)*sizeof(char));
	strcpy(stringa,"[Parte ");
	if(parte==0)
		strcat(stringa,"1, del file ");
	if(parte==1)
		strcat(stringa,"2, del file ");
	if(parte==2)
		strcat(stringa,"3, del file ");
	if(parte==3)
		strcat(stringa,"4, del file ");
		
	strcat(stringa,a.file_path);
	strcat(stringa," spedita dal processo ");	
	strcat(stringa,buffer);
	strcat(stringa," tramite ");
	if(a.mtype==2)
        strcat(stringa,"FIFO1]\n");
	if(a.mtype==3)
		strcat(stringa,"FIFO2]\n%");
	if(a.mtype==4)
		strcat(stringa,"MsgQueue]\n");
	if(a.mtype==5)
		strcat(stringa,"ShdMem]\n");
		
	strcat(stringa,a.msg_body);
	printf("SASSA%s",stringa);
	//elimino il terminatore di stringa e lo sostituisco con \n
	for(int i=0; i<strlen(stringa); i++)
		if(stringa[i]=='\0')
			stringa[i]='\n';
	
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

    shmid = alloc_shared_memory(get_ipc_key(), 53 * sizeof(msg_t));
    shm_ptr = (msg_t *) get_shared_memory(shmid, IPC_CREAT | S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Memoria condivisa: allocata e connessa\n");

    shm_check_id = alloc_shared_memory(get_ipc_key()+1, 53 * sizeof(int));
    shm_check_ptr = (int *) get_shared_memory(shm_check_id, S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Memoria condivisa flag: allocata e connessa\n");

    semid = createSemaphores(get_ipc_key(), 53);
    short unsigned int semValues[53] = {1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    semSetAll(semid, semValues);
    DEBUG_PRINT("Semafori: creati e inizializzati\n");

    fifo1_fd = create_new_fifo(FIFO1_PATH, 'r');
    DEBUG_PRINT("Mi sono collegato alla FIFO 1\n");

    fifo2_fd = create_fifo(FIFO2_PATH, 'r');  // collegamento a fifo2
    DEBUG_PRINT("Mi sono collegato alla FIFO 2\n");

    msqid = msgget(get_ipc_key(), IPC_CREAT | S_IRUSR | S_IWUSR);
    DEBUG_PRINT("Mi sono collegato alla coda dei messaggi\n");


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
        vuoto.mtype = 666;//Stefano, io sono il diavolo!!!
        //riempio la matrice con una struttura che mi dice se le celle sono vuote
        for(int i=0;i<n;i++)
            for(int j=0; j<4;j++)
                matriceFile[i][j]=vuoto;

        DEBUG_PRINT("Tradotto in numero e' %d (teoricamente lo stesso valore su terminale)\n", n);

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
        blockFD(fifo1_fd, 0);
        blockFD(fifo2_fd, 0);
        semSignal(semid, 2);
        semWait(semid, 1);
        DEBUG_PRINT("Rese fifo non bloccanti\n");

        // si mette in ricezione ciclicamente su ciascuno dei quattro canali
        int arrived_parts_counter = 0;
        int n_tries = 0;
        while (arrived_parts_counter < n * 4) {

            // memorizza il PID del processo mittente, il nome del file con percorso completo ed il pezzo
            // di file trasmesso
            msg_t supporto1,supporto2,supporto3,supporto4;

            //leggo da fifo1 la prima parte del file
            if (read(fifo1_fd,&supporto1,sizeof(supporto1)) != -1) {
                printf("[Parte1, del file %s spedita dal processo %d tramite FIFO1]\n%s\n",supporto1.file_path,supporto1.sender_pid,supporto1.msg_body);
                aggiungiAMatrice(supporto1,n);
                arrived_parts_counter++;
            }

            //leggo da fifo2 la seconda parte del file
            if (read(fifo2_fd,&supporto2,sizeof(supporto2)) != -1) {
                printf("[Parte2,del file %s spedita dal processo %d tramite FIFO2]\n%s\n",supporto2.file_path,supporto2.sender_pid,supporto2.msg_body);
                aggiungiAMatrice(supporto2,n);
                arrived_parts_counter++;
            }

            //leggo dalla coda di messaggi la terza parte del file

            if (msgrcv(msqid,&supporto3,sizeof(struct msg_t)-sizeof(long),CONTAINS_MSGQUEUE_FILE_PART, IPC_NOWAIT) != -1) {
                printf("[Parte3,del file %s spedita dal processo %d tramite MsgQueue]\n%s\n",supporto3.file_path,supporto3.sender_pid,supporto3.msg_body);
                aggiungiAMatrice(supporto3,n);
                arrived_parts_counter++;
            }

            // leggi dalla memoria condivisa
            semWait(semid, 3);
            for (int i = 0; i < 50; i++) {
                if (shm_check_ptr[i] == 1) {
                    DEBUG_PRINT("Trovata posizione da leggere %d, messaggio: '%s'\n", i, shm_ptr[i].msg_body);
                    shm_check_ptr[i] = 0;
                    aggiungiAMatrice(shm_ptr[i],n);
                    arrived_parts_counter++;
                }
            }
            semSignal(semid, 3);

            // una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e
            // salva le 4 parti in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
            // bianca (carattere newline) ed ha l’intestazione
            // > Il file verrà chiamato con lo stesso nome e percorso del file originale ma con l'aggiunta del postfisso "_out"


            if (n_tries % 5000 == 0) {
		DEBUG_PRINT("Ancora un altro tentativo... Counter = %d\n", arrived_parts_counter);
            }
            n_tries++;
        }
        
        int file;
        char *temp;//supporto che identifica il percorso dei file da creare
        char *stampa;//supporto che andrà scritto sul file
        for(int i=0;i<n;i++){
            temp=(char *)malloc((strlen(matriceFile[i][0].file_path)+4)*sizeof(char));//aggiungo lo spazio per _out
            strcpy(temp,matriceFile[i][0].file_path);
            strcat(temp,"_out");//aggiungo -out
            file=open(temp,O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
            for(int j=0; j<4;j++){
                costruisciStringa(matriceFile[i][j],stampa,j);
                write(file,stampa,sizeof(stampa));
            }
            close(file);
            free(temp);
            free(stampa);   
        }

        // quando ha ricevuto e salvato tutti i file invia un messaggio di terminazione sulla coda di
        // messaggi, in modo che possa essere riconosciuto da Client_0 come messaggio
        DEBUG_PRINT("Invio messaggio di fine al client\n");
        msg_t end_msg = {.msg_body = "DONE", .mtype = CONTAINS_DONE, .sender_pid=getpid()};
        msgsnd(msqid, &end_msg, sizeof(struct msg_t)-sizeof(long), 0);
        DEBUG_PRINT("Inviato messaggio di fine al client\n");

        // rendi fifo bloccanti
        DEBUG_PRINT("Rendi fifo bloccanti\n");
        blockFD(fifo1_fd, 1);
        blockFD(fifo2_fd, 1);
        semSignal(semid, 2);
        semWait(semid, 1);
        DEBUG_PRINT("Rese fifo bloccanti\n");

        // si rimette in attesa su FIFO 1 di un nuovo valore n tornando all'inizio del ciclo
        DEBUG_PRINT("\n");
        DEBUG_PRINT("==========================================================\n");
        DEBUG_PRINT("\n");
    }

    return 0;
}
