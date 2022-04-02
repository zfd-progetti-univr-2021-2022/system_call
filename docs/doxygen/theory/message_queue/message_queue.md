# Teoria: coda dei messaggi

## Creazione della coda dei messaggi

```c
#include <sys/msg.h>
// Returns message queue identifier on success, or -1 error
int msgget(key_t key, int msgflg);
```

Dove:
* ```key```: e' una chiave IPC
* ```msgflg```: descrive i permessi
    * puo' essere una delle flag del parametro ```mode``` di ```open()```
    * ```IPC_CREAT```: se non esiste la message queue legata alla chiave, creala
    * ```IPC_EXCEL```: usata insieme a ```IPC_CREAT```, fa fallire msgget se esiste gia' una coda di messaggi legata alla chiave

Per vedere come generare la chiave andare nella <a href="md_theory_generate_keys_generate_keys.html">sezione relativa qui</a>

Esempi:
```c
int msqid;

ket_t key = //... (generazione chiave)

// A) Fai scegliere la chiave al kernel
msqid = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);

// B) Ottieni identificativo della coda dei messaggi con la chiave key,
//    creala se non esiste
msqid = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);

// C) Ottieni identificativo della coda dei messaggi con la chiave key,
//    fallisce se esiste gia'
msqid = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
```

## Struttura dei messaggi

La coda dei messaggi serve per inviare e ricevere messaggi.

Un messaggio e' una struttura che possiede un campo chiamato mtype di tipo long maggiore di zero e poi i dati da inviare.

Esempio:
```c
struct mymsg {
    long mtype;   /* tipo del messaggio */
    char mtext[]; /* Testo */
};
```

Oppure:
```c
struct mymsg {
    long mtype;   /* tipo del messaggio */
    int a;        /* Intero 1 */
    int b;        /* Intero 2 */
};
```

Oppure:
```c
struct mymsg {
    long mtype;   /* tipo del messaggio */
};
```

## Inviare un messaggio

Per scrivere un messaggio bisogna usare la system call:
```c
#include <sys/msg.h>
// Returns 0 on success, or -1 error
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
```

Dove:
* ```msqid```: identificatore della coda dei messaggi
* ```msgp```: indirizzo che punta alla struttura dei messaggi
* ```msgsz```: specifica il numero di byte contenuti nel corpo del messaggio (quindi senza contare mtype)
* ```msgflg```:
    * 0: se la coda dei messaggi e' piena, questa system call si blocca in attesa di spazio libero
    * ```IPC_NOWAIT```: se la coda dei messaggi e' piena prosegue restituendo l'errore EAGAIN

Esempio:
```c
// Struttura del messaggio
struct mymsg {
    long mtype;
    char mtext[100]; /* stringa */
} m;

// messaggio di tipo 1
m.mtype = 1;

// il messaggio contiene la seguente stringa
char *text = "Ciao mondo!";

// copia la stringa dentro a mtext
memcpy(m.mtext, text, strlen(text) + 1);

// calcolo dimensione del messaggio ignorando mtype
size_t mSize = sizeof(struct mymsg) - sizeof(long);

// invia il messaggio nella coda dei messaggi
if (msgsnd(msqid, &m, mSize, 0) == -1)
    errExit("msgsnd failed");
```

## Ricevere un messaggio

La system call ```msgrcv()``` legge e rimuove il messaggio dalla queue dei messaggi:

```c
#include <sys/msg.h>
// Returns number of bytes copied into msgp on success, or -1 error
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtype, int msgflg);
```

Dove:
* ```msgid```: identificativo della coda dei messaggi
* ```msgp```: buffer dove leggere il messaggio
* ```msgsz```: dimensione del messaggio (senza ```mtype```)
* ```msgtype```: tipo del messaggio
    * maggiore di 0: il primo messaggio della coda che ha il tipo uguale a msgtype viene rimosso e restituito al lettore
    * uguale a 0: viene letto e rimosso il primo messaggio della coda
    * minore di 0: viene letto e rimosso il messagio con mtype piu' piccolo e che ha valore minore o uguale al valore assoluto di ```msgtype```
* ```msgflg```: flag
    * 0: se si cerca di leggere un messaggio di tipo msgtype che non c'e' la chiamata e' bloccante.

        Se la dimensione del messaggio (senza mtype) supera la dimensione definita in msgsize verra' restituito un errore.

    * ```IPC_NOWAIT```: se si cerca di leggere un messaggio di tipo  msgtype che non c'e' la chiamata NON e' bloccante.
    > Verra' restituito l'errore ENOMSG
    * ```MSG_NOERROR```: Se la dimensione del messaggio (senza mtype) supera la dimensione definita in msgsize NON dara' errore.

        Il messaggio verra' cancellato dalla coda dei messaggi e verra' troncato per stare in msgsz bytes.


Esempio:
```c
// struttura dei messaggi
struct mymsg {
    long mtype;
    char mtext[100]; /* corpo del messaggio */
} m;

// Calcola la dimensione di mtext
size_t mSize = sizeof(struct mymsg) - sizeof(long);

// Aspetta un messaggio di tipo 1
if (msgrcv(msqid, &m, mSize, 1, 0) == -1)
    errExit("msgrcv failed");
```

## Operazioni di controllo

```c
#include <sys/msg.h>
// Returns 0 on success, or -1 error
int msgctl(int msqid, int cmd, struct msqid_ds *buf);
```

Dove:
* ```msqid```: identificativo della coda dei messaggi
* ```cmd```: comando da eseguire
    * ```IPC_RMID```: rimuove immediatamente la coda. Cancella tutti i messaggi e sveglia i processi in attesa con errore EIDRM.
    * ```IPC_STAT```: salva in ```buf``` le statistiche della coda
    * ```IPC_SET```: modifica impostazioni della coda utilizzando ```buf```
        > si possono modificare i campi ```msg_perm``` e```msg_qbytes```.
* ```buf```: buffer.

Esempio. Cancella la coda:
```c
if (msgctl(msqid, IPC_RMID, NULL) == -1)
    errExit("msgctl failed");
else
    printf("message queue removed successfully\n");
```

Struttura msqid_ds *buf:
```c
struct msqid_ds {
    struct ipc_perm msg_perm; // proprietario e permessi
    time_t msg_stime; // tempo dell'ultimo last msgsnd()
    time_t msg_rtime; // tempo dell'ultimo msgrcv()
    time_t msg_ctime; // tempo dell'ultima modifica
    unsigned long __msg_cbytes; // numero di byte nella coda
    msgqnum_t msg_qnum; // numero di messaggi nella coda
    msglen_t msg_qbytes; // numero massimo di byte inseribili nella coda
    pid_t msg_lspid; // PID dell'ultimo msgsnd()
    pid_t msg_lrpid; // PID dell'ultimo msgrcv()
};
```
> Con ```IPC_SET``` si possono modificare i campi ```msg_perm``` e```msg_qbytes```.

Esempio. Cambiare quantita' massima di byte memorizzabili nella coda:
```c
struct msqid_ds ds;

// Ottieni la struttura della coda dei messaggi
if (msgctl(msqid, IPC_STAT, &ds) == -1)
    errExit("msgctl");

// Cambia il limite di byte massimi dell'mtext
// per tutti i messaggi a 1 Kbyte
ds.msg_qbytes = 1024;

// Aggiorna la struttura nel kernel in kernel
if (msgctl(msqid, IPC_SET, &ds) == -1)
    errExit("msgctl");
```
