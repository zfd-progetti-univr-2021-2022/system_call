/**
 * @file client.c
 * @brief Contiene l'implementazione del client.
 *
 * @todo Completare l'implementazione delle funzioni.
 * @todo Rimuovere printf di debug o gestirle tramite flag di compilazione.
 * @todo Spostare la funzione dividi() ? (una opzione e' metterla in files.c)
*/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "defines.h"
#include "err_exit.h"
#include "signals.h"
#include "files.h"
#include "client.h"

/// contiene percorso passato come parametro
char * searchPath = NULL;


void SIGINTSignalHandler(int sig) {
    // blocca tutti i segnali (compresi SIGUSR1 e SIGINT) modificando la maschera
    block_all_signals();

    // imposta la sua directory corrente ad un path passato da linea di comando all’avvio del programma
    if (chdir(searchPath) == -1) {
        ErrExit("chdir failed");
    }

    // saluta l'utente stampando un messaggio a video
    char *USER = getenv("USER");
    if (USER == NULL)
        USER = "unknown";

    char CURRDIR[BUFFER_SZ];
    if (getcwd(CURRDIR, sizeof(CURRDIR)) == NULL) {
        ErrExit("getcwd");
    }

    printf("Ciao %s, ora inizio l'invio dei file contenuti in %s\n", USER, CURRDIR);

    // ricerca in CURRDIR e nelle sotto-directory tutti i file che iniziano con "sendme_"
    // e la dimensione e' inferiore a 4KByte e memorizzali
    files_list * sendme_files = NULL;
    sendme_files = find_sendme_files(searchPath, sendme_files);
    printf("trovati i seguenti file:\n");
    print_list(sendme_files);

    // determina il numero <n> di questi file
    int n = count_files(sendme_files);
    printf("ci sono %d file 'sendme_'\n", n);

    // invia il numero di file tramite FIFO1 al server

    // si mette in attesa di conferma dal server su ShrMem

    // genera <n> processi figlio Client_i (uno per ogni file "sendme_")
    files_list * sendme_file = sendme_files;
    while (sendme_file != NULL) {
        printf("Creo figlio e gli ordino di gestire il file: %s\n", sendme_file->path);
        pid_t pid = fork();
        if (pid == -1) {
            ErrExit("fork failed");
        }
        else if (pid == 0) {
            operazioni_figlio(sendme_file->path);
            exit(0);
        }

        // codice eseguito da client_0 (i figli terminano con l'exit(0))
        sendme_file = sendme_file->next;
    }

    // si mette in attesa sulla MsgQueue di un messaggio da parte del server che lo
    // informa che tutti i file di output sono stati creati dal server stesso e che il server ha concluso le sue operazioni.

    // libera lista dei file
    free_list(sendme_files);

    // sblocca i segnali SIGINT e SIGUSR1
    block_sig_no_SIGINT_SIGUSR1();
}


void SIGUSR1SignalHandler(int sig) {
    // TODO: chiudi tutto ?
    // > se gestiti dalle altre funzioni forse non occorre fare nulla

    // termina
    exit(0);
}


void dividi(int fd, char *buf, size_t count, char *filePath, int parte) {
    ssize_t bR = read(fd, buf, count);
    if (bR > 0) {
        // add the character '\0' to let printf know where a string ends
        buf[bR] = '\0';
        printf("Parte 1 file %s: '%s'\n", filePath, buf);
    }
    else {
        printf("Non sono riuscito a leggere la parte %d\n",parte);
    }
}


void operazioni_figlio(char * filePath){
    printf("Sono il figlio %d e sto lavorando sul file %s\n", getpid(), filePath);

    // apre il file
    int fd = open(filePath, O_RDONLY);

    // controllo che non ci siano errori
    if (fd == -1) {
	    ErrExit("open failed");
    }

    // determina il numero di caratteri totali
    long numChar;
    numChar = lseek(fd, 0, SEEK_END);

    printf("Il file %s contiene %ld caratteri (dimensione %ld)\n", filePath, numChar, getFileSize(filePath));

    // divide il file in quattro parti contenenti lo stesso numero di caratteri (l'ultimo file puo' avere meno caratteri)
    long msg_lengths[MSG_PARTS_NUM];

    for (int i = 0; i < MSG_PARTS_NUM; i++) {
        msg_lengths[i] = numChar / MSG_PARTS_NUM;
    }

    for (int i = 0; i < (numChar % MSG_PARTS_NUM) && i < MSG_PARTS_NUM; i++) {
        msg_lengths[i] += 1;
    }

    printf("Il file %s contiene verra' diviso in parti con questi caratteri: %ld %ld %ld %ld\n", filePath, msg_lengths[0], msg_lengths[1], msg_lengths[2], msg_lengths[3]);

    // prepara i quattro messaggi (4 porzioni del contenuto del file) per l’invio
    // > NOTA: questo codice sarebbe da sistemare: forse si puo' creare una matrice 4 x (MSG_BUFFER_SZ+1) e usare un for?
    lseek(fd, 0, SEEK_SET);
    char msg_buffer[MSG_PARTS_NUM][MSG_BUFFER_SZ + 1];

    for (int i = 0; i < MSG_PARTS_NUM; i++){
        dividi(fd, msg_buffer[i], msg_lengths[i], filePath, i+1);
    }

    // si blocca su un semaforo fino a quando tutti i client sono arrivati a questo punto
    // > Attesa con semop() finche' non arriva a zero.

    // -- INVIO 4 MESSAGGI
    // > NOTA: servono meccanismi di sincronizzazione tra i client. Il server gestira' il riordino dei messaggi.

    // invia il primo messaggio a FIFO1
    // > invia anche il proprio PID ed il nome del file "sendme_" (con percorso completo)

    // invia il secondo messaggio a FIFO2
    // > invia anche il proprio PID ed il nome del file "sendme_" (con percorso completo)

    // invia il terzo a MsgQueue (coda dei messaggi)
    // > invia anche il proprio PID ed il nome del file "sendme_" (con percorso completo)

    // invia il quarto a ShdMem (memoria condivisa)
    // > invia anche il proprio PID ed il nome del file "sendme_" (con percorso completo)

    // chiude il file
    if (close(fd) == -1) {
        ErrExit("close failed");
    }

    // termina
}


int main(int argc, char * argv[]) {

    // assicurati che sia stato passato un percorso come parametro e memorizzalo
    if (argc != 2) {
        printf("Please, execute this program passing the directory with the send_me files as a parameter\n");
        exit(1);
    }

    searchPath = argv[1];

    // crea una maschera che gli consente di ricevere solo i segnali SIGINT e SIGUSR1
    block_sig_no_SIGINT_SIGUSR1();

    // imposta due signal handler: uno per SIGINT per eseguire le operazioni
    // principali di Client 0 e uno per SIGUSR1 per terminare il programma
    if (signal(SIGINT, SIGINTSignalHandler) == SIG_ERR || signal(SIGUSR1, SIGUSR1SignalHandler) == SIG_ERR) {
        ErrExit("change signal handler failed");
    }

    // sospendi processo fino all'esecuzione di un signal handler.
    // dopo la gestione del segnale, torna in sospensione.
    // > Il processo terminera' con il segnale SIGUSR1
    while (true)
        pause();

    return 0;
}
