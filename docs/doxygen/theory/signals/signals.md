# Teoria: segnali

## Introduzione

Un segnale e' una notifica che indica ad un processo che un certo evento e' avvenuto.
> E quindi puo' essere ricevuto in qualsiasi momento.

I segnali interrompono il normale flusso del programma e vengono gestiti da una (o piu') funzione specifica ("signal handler").
Al termine della esecuzione della funzione il programma inizia ad eseguire la istruzione su cui era stato interrotto in precedenza dal segnale.

Un segnale che e' stato generato ma che non e' ancora arrivato al processo si dice pendente ("pending").

Un processo puo' gestire i segnali con un handler di default (che segue dei comportamenti predefiniti), puo' gestirli con un handler personalizzato oppure puo' ignorarli.
> NON e' possibile ignorare i segnali ```SIGKILL``` (termina il processo forzatamente) e ```SIGSTOP``` (mette in pausa il processo per farne eseguire un altro).

## Tipi di segnali

Segnali che terminano un processo:
* ```SIGTERM```: indica al processo di terminare in modo sicuro (dopo aver salvato e chiuso tutte le risorse).

* ```SIGINT```: termina forzatamente un processo.
> E' inviato al processo quando l'utente preme sulla tastiera ```Ctrl+C```

* ```SIGQUIT```: termina il processo facendogli produrre un file core dump utile per il debugging

* ```SIGKILL```: termina forzatamente un processo e non puo' essere gestito in nessun modo.
> Non si possono creare handler personalizzati per gestirlo

Segnali che mettono in pausa e fanno ripartire processi:
* ```SIGSTOP```: ferma/mette in pausa un processo. Non puo' essere gestito in nessun modo.
> Non si possono creare handler personalizzati per gestirlo

* ```SIGCONT```: fa ripartire un processo in pausa.

Altri segnali:
* ```SIGPIPE```: generato da un processo che cerca di scrivere su una PIPE
* ```SIGALRM```: segnale generato dalla system call alarm. La system call fa partire un timer e al suo termine genera il segnale.
* ```SIGUSR1```: e' un segnale utilizzabile dal programmatore per i suoi scopi e che quindi non ha un significato standard.
> Ad esempio puo' essere usato per sincronizzare dei processi.
* ```SIGSTOP```SIGUSR2```: e' un segnale utilizzabile dal programmatore per i suoi scopi e che quindi non ha un significato standard.
> Ad esempio puo' essere usato per sincronizzare dei processi.

## Signal handler

E' una funzione che viene eseguita quando il processo riceve un certo segnale.
> il kernel si occupa di richiamarla

Tutti i signal handler hanno la seguente struttura:
```c
void <nome_funzione>(int sig){
    // codice gestione segnale/i
}
```

Dove:
* ```<nome_funzione>```: e' il nome della funzione, non e' importante.
* ```sig```: e' un intero che indica quale tipo di segnale ha fatto eseguire
> e' utile quando si usa lo stesso handler per gestire piu' tipi di segnali.

> personalmente utilizzerei un unico handler per piu' segnali solo se la maggior parte (o tutte) delle operazioni da eseguire sono identiche.
* Non ha valori di ritorno

Dopo aver eseguito l'handler, il programma prosegue la sua esecuzione da dove era stato interrotto.

## Impostare un signal handler personalizzato

La system call ```signal()``` permette di impostare un signal handler personalizzato:
```c
#include <signal.h>
typedef void (*sighandler_t)(int);

signal(int signum, sighandler_t handler);
```

Dove:
* ```signum```: e' il segnale da gestire con il signal handler "handler"
* ```handler```. Puo' essere:
    * l'indirizzo del nuovo signal handler
    * ```SIG_DFL```: costante che permette di resettare il comportamento di default
    * ```SIG_IGN```: costante che permette di ignorare il segnale

Esempio:
```c
void sigHandler(int sig) {
    printf("Ho ricevuto un SIGINT!\n");
}


int main (int argc, char *argv[]) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        printf("Non sono riuscito ad impostare il nuovo signal handler!\n");
        exit(1);
    }
}
```

Per impostare lo stesso signal handler per piu' segnali oppure per impostare signal handler diversi a certi segnali e' sufficiente richiamare ```signal()``` come descritto sopra.

Esempio:
```c
// dopo aver definito due signal handler chiamati
// sigHandlerperSIGINT e sigHandlerperSIGUSR1

// ...

if (signal(SIGINT, sigHandlerperSIGINT) == SIG_ERR ||
    signal(SIGUSR1, sigHandlerperSIGUSR1) == SIG_ERR) {
    printf("Non sono riuscito ad impostare i nuovi signal handler!\n");
    exit(1);
}
```

Per resettare il comportamento di default:
```c
if (signal(SIGINT, SIG_DFL) == SIG_ERR){
    printf("Non sono riuscito a resettare il comportamento da eseguire quando ricevero' il segnale SIGINT\n");
    exit(1);
}
```

Per ignorare un segnale:
```c
if (signal(SIGINT, SIG_IGN) == SIG_ERR){
    printf("Non sono riuscito a memorizzare il fatto che dovro' ignorare il segnale SIGINT\n");
    exit(1);
}
```

## Informazioni utili

* Non e' possibile bloccare i segnali ```SIGKILL``` e ```SIGSTOP```
* Non e' possibile prevedere quando arrivera' un segnale
* Il flusso di esecuzione del programma viene interrotto quando viene ricevuto un segnale non bloccato per eseguire il signal handler. Quando il signal handler termina il programma torna ad eseguire il flusso "originale".
* Se arrivano piu' segnali di un tipo ignorato nel momento in cui si sblocca il segnale arrivera' al processo una volta sola.
* Anche il signal handler puo' essere interrotto da un segnale
* Se un processo padre e' configurato per gestire in un certo modo dei segnali, i figli erediteranno quella configurazione.

## Attendere un segnale per un tempo indefinito

Richiamando ```pause()``` si sospende l'esecuzione di un processo che riprendera' solo quando verra' ricevuto un segnale.

Quel segnale fara' eseguire il signal handler o terminera' il processo.

```
#include <unistd.h>

int pause();
```

La funzione restituisce sempre -1 e imposta errno a EINTR.

## Attendere un segnale per una certa quantita' di tempo

Richiamando ```sleep()``` e' possibile sospendere il processo che riprendera' solo quando verra' ricevuto un segnale oppure quando sara' scaduto il tempo.

```c
#include <unistd.h>

unsigned int sleep(unsigned int seconds);
```

Dove:
* ```seconds```: numero di secondi massimo per cui sospendere il processo
* Restituisce:
    * 0 se il tempo di sospensione e' scaduto
    * un numero maggiore di 0 se il processo viene svegliato da un segnale.

        Quel numero indica quanti secondi mancavano per far scadere il tempo.

        In termini matematici:
        $$seconds - tempo \ sospensione$$

Esempio:

```c
void sigHandler(int sig) {
    printf("\nMi hai svegliato! volevo dormire ancora...\n");
}

int main (int argc, char *argv[]) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        printf("Non sono riuscito ad impostare il nuovo signal handler\n");
        exit(1);
    }

    int time = 5;
    printf("Dormo per %d secondi!\n", time);

    int rem_time = sleep(time); // mi sospendo per al massimo 5 secondi

    if (rem_time == 0){
        printf("Ho dormito per tutti i %d secondi senza interruzioni\n", time);
    }
    else {
        printf("Volevo dormire altri %d secondi...\n", rem_time);
    }

    return 0;
}
```

## Mandare un segnale ad un altro processo

La system call usata per mandare un segnale e':

```c
#include <signal.h>

int kill(pid_t pid, int sig);
```
> Contrariamente a quello che fa pensare il nome, questa system call puo' mandare tutti i segnali: non solo SIGKILL!

Dove:
* ```pid```: in genere e' il PID del processo a cui mandare il segnale.

    In realta' puo' essere:
    * maggiore di zero: e' il PID del processo a cui mandare il segnale
    * uguale a zero: il segnale e' mandato a tutti i processi appartenenti allo stesso gruppo del processo che manda il messaggio, incluso se stesso
    > E' un broadcast che comprende il mittente stesso

    * uguale a -1: il segnale viene mandato a tutti i processi a cui il mittente ha i permessi di mandare messaggi.
    > Ad esempio non puo' mandarlo a init e a se stesso

    * minore di -1:  il segnale e' mandato a tutti i processi che appartengono al gruppo con pid uguale al valore assoluto di ```pid```

* ```sig```: e' il segnale da mandare al processo
* Restituisce: -1 in caso di errore, 0 altrimenti

Esempio:
```c
int main (int argc, char *argv[]) {
    pid_t child = fork();
    if (child == -1) {
        errExit("fork");
    }
    else if (child == 0) {
        // il processo figlio e' bloccato qui
        while(1);
    }
    else {
        // processo padre
        sleep(10); // aspetta 10 secondi
        kill(child, SIGKILL); // manda segnale SIGKILL al processo figlio
    }
    return 0;
}
```

## Mandare un segnale a se stessi dopo una certa quantita' di tempo

La system call alarm permette di impostare un timer dopo cui si ricevera' una "notifica di promemoria":

```c
#include <signal.h>

unsigned int alarm(unsigned int seconds);
```
> Non puo' mai fallire.

Dopo ```seconds``` secondi il timer scadra' e il processo ricevera' la notifica.

La notifica e' un segnale chiamato ```SIGALRM```.

Restituisce:
* il numero di secondi rimanente da un timer impostato precedentemente
* 0 se e' stato impostato un unico timer

Impostare un timer con ```alarm``` sovrascrive i timer impostati precedentemente.

## Filtrare segnali

Esistono diverse funzioni che permettono di creare e impostare oppure di eliminare filtri che servono per bloccare (o sbloccare) determinati segnali.
> In gergo tecnico questi filtri vengono chiamati "maschera"

Per prima cosa bisogna creare un insieme di segnali da bloccare (o sbloccare).
```c
#include <signal.h>
typedef unsigned long sigset_t;

int sigemptyset(sigset_t *set);
```
> Restituisce 0 in caso di successo o -1 in caso di errore
Inizializza il filtro ```set``` per essere vuoto.

```c
#include <signal.h>
typedef unsigned long sigset_t;

int sigfillset(sigset_t *set);
```
> Restituisce 0 in caso di successo o -1 in caso di errore
Inizializza il filtro ```set``` per contenere tutti i segnali.

Poi bisogna aggiungere o rimuovere segnali dal filtro:
```c
#include <signal.h>

int sigaddset(sigset_t *set, int sig);
```
Aggiunge all'insieme ```set``` il segnale ```sig```.

```c
#include <signal.h>

int sigdelset(sigset_t *set, int sig);
```
Elimina dall'insieme ```set``` il segnale ```sig```.

Se si vuole si puo' verificare se un segnale e' contenuto nel filtro:
```c
#include <signal.h>

int sigismember(const sigset_t *set, int sig);
```
Restituisce 1 se il segnale ```sig``` e' contenuto nell'insieme ```set```.

Dopo aver creato il filtro adatto per le nostre esigenze bisogna impostarlo con:
```c
#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```
> Restituisce 0 in caso di successo o -1 in caso di errore

Dove:
* ```how``` indica come usare il filtro.

    Puo' essere:
    * ```SIG_BLOCK```: continua a bloccare i segnali gia' bloccati e in piu' blocca quelli contenuti in ```set```

    * ```SIG_UNBLOCK```: sblocca i segnali in ```set```.
    > Se i segnali in ```set``` non sono bloccati non succede niente: non ci sono errori.

    * ```SIG_SETMASK```: imposta come segnali bloccati SOLO i segnali contenuti in ```set```.
    > Quindi se ```set``` NON contiene segnali bloccati precedentemente, questi verranno sbloccati

* ```set```:
    * filtro di segnali da bloccare/sbloccare
    * NULL se si desidera recuperare la maschera precedente senza modificarla
    > ```oldset``` deve essere un puntatore ad una variabile in cui memorizzarla. ```how``` viene ignorato.

* ```oldset```:
    * se e' NULL la maschera precedente viene persa
    * se e' una variabile che puo' ospitare una maschera, questa memorizzera' la maschera precedente

Esempio:
```c
int main (int argc, char *argv[]) {
    sigset_t mySet, prevSet;

    // inizializza mySet con tutti i segnali
    sigfillset(&mySet);

    // rimuovi SIGTERM e SIGINT da mySet
    sigdelset(&mySet, SIGTERM);
    sigdelset(&mySet, SIGINT);

    // blocca tutti i segnali tranne SIGTERM e SIGINT
    sigprocmask(SIG_SETMASK, &mySet, &prevSet);

    // il codice qui puo' solo essere interrotto da SIGTERM, SIGINT
    // e dai segnali non bloccabili (SIGKILL e SIGSTOP)

    // reset the signal mask of the process
    sigprocmask(SIG_SETMASK, &prevSet, NULL);
    // if SIGTERM is pending, only at this point it is
    // delivered to the process
    return 0;
}
```
