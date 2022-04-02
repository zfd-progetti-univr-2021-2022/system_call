# Teoria: memoria condivisa

E' una porzione di memoria gestita dal kernel e che e' condivisa tra piu' processi.

I dati posso essere scritti e letti immediatamente sulla memoria: e' necessario un sistema di sincronizzazione come, ad esempio, i semafori.

## Creazione memoria condivisa

La system call ```shmget()``` permette di creare una nuova memoria condivisa oppure di ottenere un identificatore di un segmento di memoria gia' esistente.
```c
#include <sys/shm.h>
// Returns a shared memory segment identifier on success, or -1 on error
int shmget(key_t key, size_t size, int shmflg);
```

La memoria creata sara' inizializzata a 0.

I parametri sono:
* ```key```: chiave IPC
* ```size```: dimensione desiderata della memoria condivisa in byte
> La dimensione viene arrottondata al multiplo superiore della dimensione di pagina del sistema. Se non si vuole creare una memoria condivisa esistente deve essere minore o uguale al valore size con cui era stata creata precedentemente.
* ```shmflg```: specificano i permessi.

    Possono essere usate tutte le flag di ```mode``` per i file e in piu' possono essere usate anche:
    * ```IPC_CREAT```: se non esiste un segmento di memoria legato alla chiave attuale, creane uno nuovo
    * ```IPC_EXCL```: se usato con ```IPC_CREAT```, permette al comando di fallire se esiste gia' una memoria condivisa legata alla chiave ```key```

Per vedere come generare la chiave andare nella <a href="md_theory_generate_keys_generate_keys.html">sezione relativa qui</a>

Esempi creazione memoria condivisa:
```c
int shmid;
ket_t key = //... (generazione chiave)

size_t size = //... (dimensione calcolabile)

// A) Crea la memoria condivisa facendo scegliere al kernel la chiave
shmid = shmget(IPC_PRIVATE, size, S_IRUSR | S_IWUSR);

// B) Crea la memoria condivisa utilizzando la chiave key, solo se non esiste
shmid = shmget(key, size, IPC_CREAT | S_IRUSR | S_IWUSR);

// C) Crea la memoria condivisa utilizzando la chiave key,
//    fallisce se la memoria e' gia' esistente
shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
```

## Collegamento alla memoria condivisa

Per usare la memoria condivisa bisogna aggiungerla allo spazio degli indirizzi del processo (in modo virtuale) con la system call:

```c
#include <sys/shm.h>
// Returns address at which shared memory is attached on success
// or (void *)-1 on error
void *shmat(int shmid, const void *shmaddr, int shmflg);
```
> shmat sta per "shared memory attach"

Dove:
* ```shmid```: e' l'identificativo ottenuto dalla chiave
* ```shmaddr```:
    * NULL: il kernel sceglie l'indirizzo in cui "collegare" la memoria condivisa.
    > Ignora le flag.

    > Questa opzione permette di rendere l'applicazione il piu' portatile possibile e riduce il rischio di collegare la memoria in zone gia' occupate
    (evitando cosi' di andare in errore)

    * indirizzo: se ```shmflg``` e' ```SHM_RND``` verra' utilizzato indirizzo in cui "collegare" la memoria condivisa
* ```shmflg```:
    * ```SHM_RND```: permette di usare l'indirizzo ```shmaddr```
    > RND sta per round ed ha a che fare con l'arrotondamento della dimensione delle pagine
    * ```SHM_RDONLY```: imposta la memoria condivisa in sola lettura
    * 0: modalita' lettura e scrittura

I processi figlio ereditano le shared memory del padre.

Esempio:
```c
// ottieni puntatore per gestire la memoria condivisa
// in modalita' lettura/scritura
int *ptr_1 = (int *)shmat(shmid, NULL, 0);

if (prt_1 == -1) {
    ErrExit("shmat failed");
}

// ottieni puntatore per gestire la memoria condivisa in sola lettura
int *ptr_2 = (int *)shmat(shmid, NULL, SHM_RDONLY);

if (prt_2 == -1) {
    ErrExit("shmat failed");
}

// scrivi la memoria condivisa usando il puntatore mod. lettura/scrittura
for (int i = 0; i < 10; ++i)
    ptr_1[i] = i;

// leggi la memoria condivisa con il puntatore in mod. lettura
for (int i = 0; i < 10; ++i)
    printf("integer: %d\n", ptr_2[i]);
```

## Scollegamento della memoria condivisa

Quando un processo non necessita' piu' di una memoria condivisa puo' scollegarla eseguendo:
```c
#include <sys/shm.h>
// Returns 0 on success, or -1 on error
int shmdt(const void *shmaddr);
```

Dove:
* ```shmaddr```: indirizzo della memoria da scollegare
> Valore restituito da ```shmat```

Le memorie condivise vengono scollegate automaticamente quando:
* viene eseguito una exec
* viene terminato il processo

Esempio:
```c
// collega la memoria condivisa in lettura/scrittura
int *ptr_1 = (int *)shmat(shmid, NULL, 0);
if (ptr_1 == (void *)-1)
    errExit("first shmat failed");

// scollega la memoria condivisa
if (shmdt(ptr_1) == -1)
    errExit("shmdt failed");
```

## Operazioni di controllo

La system call ```shmctl``` permette di controllare la memoria condivisa.

```c
#include <sys/msg.h>
// Returns 0 on success, or -1 error
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

Dove:
* ```shmid```: identificativo ottenuto dalla chiave
* ```cmd```: comando da eseguire
    * ```IPC_RMID```: quando tutti i processi si saranno scollegati dalla memoria condivisa, questa verra' cancellata
    * ```IPC_STAT```: memorizza in ```buf``` le statistiche della memoria condivisa
    * ```IPC_SET```: usa ```buf``` per modificare le proprieta' della memoria condivisa
    > L'unico campo modificabile e' ```shm_perm```

* ```buf```: puntatore a struttura dati usata per memorizzare/impostare proprieta' della memoria condivisa

```c
if (shmctl(shmid, IPC_RMID, NULL) == -1)
    errExit("shmctl failed");
else
    printf("shared memory segment removed successfully\n");
```

Struttura shmid_ds buf:
```c
struct shmid_ds {
    struct ipc_perm shm_perm; // permessi e proprietario (kernel)
    size_t shm_segsz; // dimensione in byte
    time_t shm_atime; // tempo dell'ultima shmat()
    time_t shm_dtime; // tempo dell'ultimo shmdt()
    time_t shm_ctime; // tempo dell'ultima modifica
    pid_t shm_cpid; // PID del creatore
    pid_t shm_lpid; // PID dell'ultimo shmat() / shmdt()
    shmatt_t shm_nattch; // Numero di processi attualmente collegati
};
```

L'unico campo modificabile e' ```shm_perm``` per gestire i permessi.
