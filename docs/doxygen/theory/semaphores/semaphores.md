# Teoria: semafori

I semafori vengono usati per sincronizzare piu' processi.

La sincronizzazione puo' servire per gestire la comunicazione tra processi oppure per coordinarli.
> NOTA: non tutti i mezzi di comunicazione necessitano di semafori. Un mezzo che li richiede e' la memoria condivisa.

Le system call per i semafori lavorano sempre su insiemi (set) di semafori e NON su semafori singoli.

## Creazione di un insieme di semafori

```c
#include <sys/sem.h>

// Returns semaphore set identifier on success, or -1 error
int semget(key_t key, int nsems, int semflg);
```

Dove.
* ```key```: e' la chiave IPC
* ```nsems```: e' il numero di semafori da creare e deve essere quindi maggiore di 0.
* ```semflg```: specifica i permessi
    * Sono validi tutti i permessi del parametro ```mode``` della system call ```open()```
    * ```IPC_CREAT```: se non esiste un set di semafori legato alla chiave, lo crea
    * ```IPC_EXCL```: se usato con ```IPC_CREAT```, fa fallire la system call se esiste gia' un set di semafori legato alla chiave

Per vedere come generare la chiave andare nella <a href="md_theory_generate_keys_generate_keys.html">sezione relativa qui</a>

Esempi:
```c
int semid;
ket_t key = // ... (genera la chiave in qualche modo)

// A) Delega al kernel la scelta della chiave
semid = semget(IPC_PRIVATE, 10, S_IRUSR | S_IWUSR);

// B) Se non esiste crea un set di 10 semafori con la chiave key
semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// C) crea un set di 10 semafori con la chiave key.
//    se esiste gia' fallisce
semid = semget(key, 10, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
```

## Operazioni di controllo

La system call ```semctl()``` permette di eseguire operazioni su un set di semafori oppure su un semaforo appartenente al set.

```c
#include <sys/sem.h>
// Returns nonnegative integer on success, or -1 error
int semctl(int semid, int semnum, int cmd, ... /* union semun arg */);
```

Dove:
* ```semid```: identificativo ottenuto usando la chiave IPC
* ```semnum```
* ```cmd```: operazione da eseguire
* (opzionale) ```arg```: argomento usato solo per certe operazioni

Dove ```arg``` e' la seguente union:

```c
#include <sys/sem.h>

union semun {
    // usato se si lavora su un singolo semaforo.
    // usato dalla operazione SETVAL
    int val;

    // usato per lavorare sullo stato globale del semaforo.
    // usato dalle operazioni IPC_STAT e IPC_SET
    struct semid_ds * buf;

    // per eseguire operazioni su tutti i semafori.
    // usato dalle operazioni GETALL e SETALL
    unsigned short * array;
};
```
> Deve essere dichiarata prima di usare ```semctl()```.

### Cancellare un set di semafori

```c
int semctl(semid, 0 /* semnum: ignored */, cmd, 0 /* arg: ignored */);
```

Se il parametro ```cmd``` vale ```IPC_RMID``` la system call rimuove immediatamente un set di semafori.
> I processi bloccati vengono svegliati con errore ```EIDRM```.

Esempio:
```c
if (semctl(semid, 0/*ignored*/, IPC_RMID, 0/*ignored*/) == -1)
    errExit("semctl failed");
else
    printf("semaphore set removed successfully\n");
```

### Reciperare statistiche

```c
int semctl(semid, 0 /* semnum: ignored */, cmd, arg);
```

Se ```cmd``` vale ```IPC_STAT``` la system call memorizza le statistiche del set di semafori in ```arg.buf```.

Dove ```arg.buf``` ha la seguente struttura:

```c
struct semid_ds {
    struct ipc_perm sem_perm; // proprietario e permessi
    time_t sem_otime; // Tempo dell'ultimo semop()
    time_t sem_ctime; // Tempo dell'ultima modifica
    unsigned long sem_nsems; // Numero di semafori nel set
};
```

### Cambiare i permessi

```c
int semctl(semid, 0 /* semnum: ignored */, cmd, arg);
```

Se ```cmd``` vale ```IPC_SET``` la system call utilizza il campo ```arg.buf``` per impostare proprieta' sul set di semafori.

Dove ```arg.buf``` ha la seguente struttura:
```c
struct semid_ds {
    struct ipc_perm sem_perm; // proprietario e permessi
    time_t sem_otime; // Tempo dell'ultimo semop()
    time_t sem_ctime; // Tempo dell'ultima modifica
    unsigned long sem_nsems; // Numero di semafori nel set
};
```

Gli unici valori modificabili sono i permessi contenuti in ```sem_perm```: ```uid```, ```gid``` e ```mode```.

Esempio:
```c
ket_t key = // ... (genera la chiave IPC)

// Crea o ottieni un set di 10 semafori legato alla chiave
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// istanzia una struct semid_ds
struct semid_ds ds;

// instanzia una union semun
// (che deve essere stata definita precedentemente nel codice)
union semun arg;
arg.buf = &ds;  // si vuole una semid_ds

// ottieni una copia di semid_ds dal kernel
if (semctl(semid, 0 /*ignored*/, IPC_STAT, arg) == -1)
    errExit("semctl IPC_STAT failed");

// modifica i permessi sulla copia aggiungendo
// i permessi di lettura al gruppo
arg.buf->sem_perms.mode |= S_IRGRP;

// aggiorna la struttura semid_ds del kernel applicando le modifiche
if (semctl(semid, 0 /*ignored*/, IPC_SET, arg) == -1)
    errExit("semctl IPC_SET failed");
```

### Inizializzare i semafori

Per inizializzare i semafori e' possibile inizializzare l'intero set oppure un singolo semaforo.

Il valore dei semafori DEVE ESSERE SEMPRE INIZIALIZZATO prima di utilizzarli.

Per inizializzare un semaforo:
```c
int semctl(semid, semnum, SETVAL, arg);
```
> Dove ```cmd``` = ```SETVAL```

Imposta il valore arg.val al semaforo semnum-esimo.

Esempio:
```c
ket_t key = //... (genera la chiave IPC)

// ottieni o crea il set di 10 semafori
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// imposta il valore desiderato del semaforo a zero
union semun arg;
arg.val = 0;

// inizializza il semaforo in posizione 5 a zero
if (semctl(semid, 5, SETVAL, arg) == -1)
    errExit("semctl SETVAL");
```

Per inizializzare l'intero set:
```c
int semctl(semid, 0 /* semnum: ignored*/, cmd, arg);
```
> Dove ```cmd``` = ```SETALL```. ```semnum``` e' ignorato perche' si inizializzano tutti i semafori, non solo uno.

Imposta i valori di arg.array al set di semafori.

Esempio:
```c
ket_t key = //... (genera la chiave IPC)

// crea o ottieni il set di 10 semafori
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// Imposta 5 semafori a 1 e gli altri 5 a 0
int values[] = {1,1,1,1,1,0,0,0,0,0};
union semun arg;
arg.array = values;

// Inizializza il set di semafori
if (semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
    errExit("semctl SETALL");
```

### Recuperare il valore dei semafori

Per recuperare il valore dei semafori e' possibile leggere i valori dell'intero set oppure il valore di un singolo semaforo.

> ATTENZIONE: subito dopo aver letto il valore dei semafori questi potrebbero cambiare e quindi non bisogna dare per scontato che siano aggiornati.
>
> Il valore e' quindi attendibile solo all'inizio e dopo la fine dell'uso dei semafori, quando c'e' un solo processo e gli altri sono terminati.

Per leggere il valore di un semaforo:
```c
int semctl(semid, semnum, GETVAL, 0 /*arg: ignored*/);
```
> Dove ```cmd``` = ```GETVAL```

Restituisce il valore del semaforo semnum-esimo.

Esempio:
```c
ket_t key = //... (genera la chiave IPC)

// ottieni o crea il set di 10 semafori
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// ottieno lo stato attuale del semaforo in posizione 5
int value = semctl(semid, 5, GETVAL, 0/*ignored*/);
if (value == -1)
    errExit("semctl GETVAL");
```

Per leggere il valore dell'intero set:
```c
int semctl(semid, 0 /*semnum:ignored*/, GETALL, arg);
```
> Dove ```cmd``` = ```GETALL```

Imposta i valori dell'array ```arg.array``` con i valori dei semafori del set.

Esempio:
```c
ket_t key = //... (genera la chiave IPC)

// ottieni o crea il set di 10 semafori
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// dichiara un array sufficientemente grande
// per memorizzare i valori dei semafori
int values[10];
union semun arg;
arg.array = values;

// ottieni lo stato attuale di tutti i semafori
if (semctl(semid, 0/*ignored*/, GETALL, arg) == -1)
    errExit("semctl GETALL");
```

### Ottenere informazioni dai singoli semafori

```c
int semctl(semid, semnum, cmd, 0);
```

Dove ```cmd``` puo' essere:
* ```GETPID```: restituisce il PID dell'ultimo processo che ha eseguito la system call ```semop``` sul semaforo ```semnum```-esimo
* ```GETNCNT```: restituisce il numero di processi attualmente in attesa che il valore del semaforo ```semnum```-esimo diventi maggiore di 0
* ```GETZCNT```: restituisce il numero di processi attualmente in attesa che il valore del semaforo ```semnum```-esimo diventi uguale a 0.

Esempio:
```c
ket_t key = //... (genera la chiave IPC)

// ottieni o crea il set di 10 semafori
int semid = semget(key, 10, IPC_CREAT | S_IRUSR | S_IWUSR);

// ...

// ottieni informazioni sul primo semaforo del set
printf(
    "Sem:%d getpid:%d getncnt:%d getzcnt:%d\n",
    semid,
    semctl(semid, 0, GETPID, NULL),
    semctl(semid, 0, GETNCNT, NULL),
    semctl(semid, 0, GETZCNT, NULL)
);
```

### Operazioni wait e signal

La system call permette di eseguire una o piu' operazioni (wait o signal) sui semafori.

```c
#include <sys/sem.h>

// Returns 0 on success, or -1 on error
int semop(int semid, struct sembuf *sops, unsigned int nsops);
```

Dove:
* ```semid```: identificativo ottenuto utilizzando la chiave IPC
* ```sops```: puntatore ad un array che contiene una sequenza di operazioni da eseguire in modo atomico.
> Le operazioni vengono eseguite in ordine da sinistra verso destra
* ```nsops```: numero di operazioni contenute nell'array ```sops```

La struttura ```sembuf``` usata nell'array ```sops``` ha la seguente forma:
```c
struct sembuf {
    unsigned short sem_num; // numero del semaforo
    short sem_op;  // operazione da eseguire
    short sem_flg; // flag della operazione
};
```

Dove:
* ```sem_num```: identifica il semaforo su cui eseguire l'operazione
* ```sem_op```: operazione da eseguire.

    * se ```sem_op``` e' maggiore di 0: il valore ```sem_op``` viene aggiunto al valore del ```sem_num```-esimo semaforo
    * se```sem_op``` e' uguale a 0: viene verificato se il semaforo ```sem_num```-esimo ha il valore 0.

        Se non e' 0, il processo viene bloccato.

        Verra' sbloccato quanto il valore del semaforo torna maggiore di 0.
    * se ```sem_op``` e' minore di 0: decrementa il valore del semaforo ```sem_num```-esimo di ```sem_op```.

        Blocca il processo.

        Il processo verra' sbloccato quando il valore del sesemaforo tornera' sufficientemente grande per permettere di eseguire l'operazione senza ottenere un risultato negativo.

In generale le chiamate di ```semop()``` sono bloccanti. Il processo si sblocca quando:
* un altro processo modifica il valore del semaforo per permettere l'esecuzione della operazione richiesta
* un segnale interrompe la chiamata. La system call fallira' con l'errore EINTR.
* un altro processo cancella il semaforo. La system call fallira' con l'errore EIDRM.

Se si vuole non avere operazioni bloccanti si puo' specificare la flag ```IPC_NOWAIT``` in ```sem_flg```.
> Se la chiamata fosse bloccante senza ```IPC_NOWAIT``` la system call fallira' con errore ```EAGAIN```

Esempio:
```c
struct sembuf sops[3];

// sottrai 1 dal semaforo 0
sops[0].sem_num = 0;
sops[0].sem_op = -1;
sops[0].sem_flg = 0;

// aggiungi 2 al semaforo 1
sops[1].sem_num = 1;
sops[1].sem_op = 2;
sops[1].sem_flg = 0;

// aspetta che il semaforo 2 sia 0
// ma non bloccare immediatamente il processo
// se l'operazione non puo' essere svolta immediatamente
sops[2].sem_num = 2;
sops[2].sem_op = 0;
sops[2].sem_flg = IPC_NOWAIT;

if (semop(semid, sops, 3) == -1) {
    if (errno == EAGAIN)
        // il semaforo 2 avrebbe bloccato il processo
        printf("Operation would have blocked\n");
    else
        errExit("semop"); // Some other error
}
```
