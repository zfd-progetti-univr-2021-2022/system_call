# Teoria: FIFO

## Introduzione

Una FIFO permette di far scambiare a piu' processi un flusso di byte
> Per farlo viene usato un buffer memorizzato nella memoria del kernel

Una FIFO ha un nome all'interno del file system e quindi e' comoda per far comunicare processi che non sono imparentati.
> Una FIFO puo' essere vista come una PIPE che ha un nome che la identifica.

Una FIFO viene scritta da una parte e letta dall'altra.

I dati vengono letti nello stesso ordine in cui sono stati scritti.

Tipicamente una FIFO e' usata da solo 2 processi: un processo crea e apre una FIFO in lettura e attende che un altro processo apra e scriva sulla FIFO.
> L'attesa e' automatica, l'apertura in lettura e' bloccante. Anche chi scrive attende che ci sia un processo pronto a leggere.

## Creare una FIFO

Una FIFO si crea con la system call:
```c
#include <unistd.h>
#include <sys/stat.h>

int mkfifo(const char *pathname, mode_t mode);
```

Dove:
* pathname: e' il percorso del file della FIFO
* mode: specifica i permessi
* restituisce: -1 in caso di errore, 0 in caso di successo

## Aprire una FIFO

```c
#include <fcntl.h>
int open(const char *pathname, int flags);
```

Dove:
* pathname: e' il percorso del file della FIFO.

    Lo stesso usato da ```mkfifo()```.

* flags: maschera che indica la modalita' di apertura della FIFO.

    Sola lettura (```O_RDONLY```) oppure sola scrittura(```O_WRONLY```).

* restituisce: -1 in caso di errore, oppure il file descriptor della FIFO

## Modalita' di utilizzo tipica

Tipicamente una FIFO viene usata da un processo che riceve e da un processo che trasmette.

In questo esempio bisogna:
1. eseguire il ricevitore
2. eseguire il trasmettitore
3. scrivere sul trasmettitore il messaggio da mandare e premere invio

Il ricevitore visualizzera' a video il messaggio inviato dal trasmettitore.

> NOTA: Nella cartella ```docs/doxygen/theory/fifo``` e' possibile trovare i file sorgente dell'esempio qui sotto.

### Ricevitore

```c
// salvo il percorso del file che gestisce la FIFO
// > stesso file usato dal trasmettitore
char *fname = "/tmp/myfifo";

// creo la FIFO passandogli il percorso e i permessi
int res = mkfifo(fname, S_IRUSR|S_IWUSR);

if (res == -1) {
    // posso usare ErrExit()
    printf("Non sono riuscito a creare la FIFO\n");
    exit(1);
}

// Apro in lettura la FIFO
int fd = open(fname, O_RDONLY);

if (fd == -1) {
    printf("Non sono riuscito ad aprire in lettura la FIFO\n");
    exit(1);
}

// leggi byte dalla FIFO e memorizza nel buffer
char buffer[LEN];
if (read(fd, buffer, LEN) == -1) {
    printf("Non sono riuscito a leggere la FIFO\n");
    exit(1);
}

// Visualizza cio' che e' stato letto
printf("%s\n", buffer);

// chiudi la FIFO
if (close(fd) == -1) {
    printf("Non sono riuscito a chiudere la FIFO\n");
    exit(1);
}

// elimina file della FIFO (o togli symlink se e' un collegamento)
if (unlink(fname)) {
    printf("Elimina file della FIFO\n");
    exit(1);
}
```

### Trasmettitore

```c
// salvo il percorso del file che gestisce la FIFO
// > stesso file usato dal ricevitore
char *fname = "/tmp/myfifo";

// Apri la FIFO in scrittura
int fd = open(fname, O_WRONLY);

if (fd == -1) {
    printf("Non sono riuscito ad aprire in scrittura la FIFO\n");
    exit(1);
}

// Leggi da terminale una stringa da mandare al ricevitore
char buffer[LEN];
printf("Give me a string: ");
fgets(buffer, LEN, stdin);

// scrivi la stringa sulla FIFO
ssize_t written_bytes = write(fd, buffer, strlen(buffer));

if (written_bytes == -1) {
    printf("Non sono riuscito a scrivere i byte sulla FIFO\n");
    exit(1);
}

// chiudi la FIFO
if (close(fd) == -1) {
    printf("Non sono riuscito a chiudere la FIFO\n");
    exit(1);
}
```
