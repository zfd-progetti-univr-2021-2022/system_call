# Teoria: PIPE

## Introduzione

Una PIPE permette di far scambiare a piu' processi un flusso di byte
> Per farlo viene usato un buffer memorizzato nella memoria del kernel

La PIPE e' utile quando due processi imparentati devono scambiarsi informazioni.
> Quindi, di solito, dopo aver creato la PIPE e' presente una fork.

Una PIPE ha le seguenti caratteristiche:
* e' UNIDIREZIONALE: i dati vanno in una direzione, entrano dalla parte della scrittura ed escono dalla parte della lettura
* i dati viaggiano nella PIPE in modo SEQUENZIALE. I byte vengono letti nello stesso ordine in cui sono stati scritti
* non usano il concetto di messaggi, e quindi non ci sono vincoli su come essi sono formati. La lettura puo' essere fatta di qualunque dimensione, senza necessariamente rispettare i vincoli della dimensione dei dati inseriti in scrittura.
* La lettura di una PIPE vuota e' bloccante.

    Per sbloccarsi deve essere scritto almeno un byte nella PIPE oppure deve arrivare un segnale (che non faccia terminare il processo).
    > errno viene impostato a EINTR

* Se viene chiuso il lato della scrittura il processo che legge vedra' l'end of file dopo aver letto il resto dei dati.
* La scrittura e' bloccante.

    Si sblocchera' se c'e' abbastanza spazio per eseguire la scrittura in modo atomico oppure se arriva un segnale (che non faccia terminare il processo).
    > errno viene impostato a EINTR

    > Su linux una PIPE ha 65536 byte di capacita'.

* Se si scrivono blocchi di dati maggiori di ```PIPE_BUF``` (4096 bytes su linux) i dati potrebbero essere suddivisi in segmenti di dimensione arbitraria (piu' piccola di ```PIPE_BUF```)

## Creare una PIPE

```c
#include <unistd.h>

int pipe(int filedes[2]);
```

Dove:
* filedes: e' un array da due interi. Se la chiamata ha successo gli interi sono i file descriptor della PIPE
    * filedes[0] e' il lato di lettura
    * filedes[1] e' il lato di scrittura
* Restituisce: 0 in caso di successo, -1 in caso di errore

Esempio apertura PIPE:
```c
int fd[2];
// apri e controlla se la apertura ha avuto successo
if (pipe(fd) == -1)
    errExit("PIPE");
```

Tipicamente dopo aver creato una PIPE il processo crea un figlio con la ```fork()```.

## Leggere da una PIPE

Dopo aver creato una PIPE, il processo che si occupa di leggere (o il padre o il figlio) puo' tranquillamente chiudere il lato di scrittura con la system call close:

```c
if (close(fd[1]) == -1)
    // visualizza child perche' in questo esempio il figlio legge
    errExit("close - child");
```
> E' altamente consigliato farlo

E poi puo' iniziare a leggere dalla PIPE con read:
```c
char buf[SIZE];  // buffer in cui vengono letti i dati
ssize_t nBys;    // numero byte letti

// leggi dalla PIPE
nBys = read(fd[0], buf, SIZE);

// 0: end-of-file, -1: errore
if (nBys > 0) {
    buf[nBys] = '\0';
    printf("%s\n", buf);
}
```

## Scrivere su una PIPE

Dopo aver creato una PIPE, il processo che si occupa di scrivere (o il padre o il figlio) puo' chiudere il lato di lettura con la system call close:

```c
if (close(fd[0]) == -1)
    // visualizza parent perche' in questo esempio il padre scrive
    errExit("close - parent");
```
> E' altamente consigliato farlo

E poi puo' iniziare a scrivere nella PIPE con write:
```c
char buf[] = "Ciao Mondo\n";
ssize_t nBys;

// scrivi sulla PIPE
nBys = write(fd[1], buf, strlen(buf));

// verifica se write ha avuto successo
if (nBys != strlen(buf)) {
    errExit("write - parent");
}
```

## Chiudere la PIPE

Per chiudere la PIPE si usa la system call close su entrambe i lati della comunicazione (lettura e scrittura).

In genere il processo che dovra' legge chiude immediatamente il lato di scrittura e viceversa il processo che dovra' scrive chiude il lato di lettura.
> E' altamente consigliato farlo

Per questo motivo a fine comunicazione i due processi si ritroveranno a chiudere soltanto il proprio lato del canale.

Chiudi lato di lettura:
```c
if (close(fd[0]) == -1)
    errExit("close");
```

Chiudi lato di scrittura:
```c
if (close(fd[1]) == -1)
    errExit("close");
```

> NOTA: queste system call sono identiche a quelle usate per chiudere il lato che non verra' usato dal proprio processo.
