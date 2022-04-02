# Teoria: generare chiavi per le IPC

Le chiavi sono dei numeri interi che servono ad alcune system call per creare una IPC (se non esiste gia') e restituire il suo identificatore che permette poi di utilizzarla per comunicare con altri processi.
> Le IPC sono meccanismi di comunicazione tra processi. Puo' essere la memoria condivisa, la coda FIFO, la PIPE, i segnali, i semafori, le code di messaggi.

Per fare un'analogia con i file di testo e' possibile vedere le chiavi come il percorso del file e l'identificatore come il file descriptor:
* I primi servono per creare i secondi
* I secondi permettono di gestire il file/la IPC

I processi che conoscono la chiave possono comunicare fra di loro creando ed utilizzando le IPC.

Per creare le chiavi esistono piu' modi diversi.

## Macro IPC_PRIVATE

La macro ```IPC_PRIVATE``` permette di generare una chiave che sara' sicuramente univoca perche' il kernel ce lo garantira'.

Esempio di creazione di un semaforo utilizzando la macro:
```c
id = semget(IPC_PRIVATE, 10, S_IRUSR | S_IWUSR);
```

E' comoda quando un processo padre crea la IPC prima di creare i processi figli con cui comunichera' in seguito.

E' difficile utilizzarla quando i processi che devono comunicare non sono imparentati.

## Funzione file to key

La funzione ```ftok()``` (file to key) converte il percorso di un file ed un identificativo di progetto in una chiave.

```c
#include <sys/ipc.h>
// Returns integer key on succcess, or -1 on error (check errno)
key_t ftok(char *pathname, int proj_id);
```
> Vengono usati solo gli ultimi 8 bit di proj_id

Il file nel percorso pathname deve essere esistente e accessibile (bisogna avere i permessi giusti)

```c
Esempio:
key_t key = ftok("/mydir/myfile", 'a');
if (key == -1)
    errExit("ftok failed");

int id = semget(key, 10, S_IRUSR | S_IWUSR);

if (id == -1)
    errExit("semget failed");
```

Questa opzione e' comoda per far comunicare processi non imparentati mantenendo un punto di riferimento fisso.

## Inserimento manuale

Per progetti semplici posso scegliere di fissare un numero per gestire le IPC.

Questa opzione e' comoda per far comunicare processi non imparentati in progetti piccoli.
