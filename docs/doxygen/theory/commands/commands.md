# Teoria: comandi utili

## Comandi debugger GDB

Per prima cosa bisogna compilare i file sorgente con le informazioni di debugging con la flag ```-g```.

Esempio:
```bash
# Usa -g per permettere il debugging
gcc -g <nome>.c -o <output>
```

Comandi GDB:
* ```break <nome funzione>```: imposta un breakpoint su una funzione
* ```break <riga codice>```: imposta un breakpoint su una certa riga del codice
* ```clear <riga codice>```: toglie il breakpoint su quella riga di codice
* ```watch <nome variabile>```: aggiunge un watchpoint che blocca il codice quando la variabile cambia valore
* ```run```: esegue il programma
* ```print <variabile>```: visualizza il valore della variabile
	> E' possibile anche scrivere print var_array[x] per visualizzare l'elemento di un array
* ```step```: esegue la riga di codice successiva seguendo il codice nelle funzioni
* ```next```: esegue la riga di codice successiva eseguendo le funzioni in un colpo solo
* ```continue```: esegue il codice finche' non viene raggiunto un breakpoint o watchpoint
* ```list```: visualizza la riga
* ```quit```: esce da gdb
* ```signal <segnale>```: invia al processo il segnale ```<segnale>```
    > Esempio: ```signal SIGINT```, utile per fare debugging del client del progetto

## Estensioni dell'interfaccia GDB e interfacce grafiche

### Interfacce CLI

[gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard) estende l'interfaccia del terminale.

![](https://raw.githubusercontent.com/wiki/cyrus-and/gdb-dashboard/Screenshot.png)

E' possibile scaricando il file ```.gdbinit``` nella cartella home:
```bash
wget -P ~ https://git.io/.gdbinit
```

Per aggiungere il syntax highlight bisogna installare Pygments (libreria Python):
```bash
pip install pygments
```

### Interfacce GUI

[gdbgui](https://github.com/cs01/gdbgui) e' una interfaccia grafica basata su tecnologie web.

![](https://raw.githubusercontent.com/cs01/gdbgui/799d34051419653cfda7d068806f853007d46fac/screenshots/gdbgui_animation.gif)

## Ricerca memory leak

Valgrind e' uno strumento che permette di analizzare la memoria usata dai processi
per trovare memory leak oppure in generale accessi errati della memoria.

Testare il server:
```
valgrind --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind-server-out.txt \
    ./server
```

Testare il client:
```
valgrind --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind-client-out.txt \
    ./client_0 <percorso cartella file>
```
> Sostituire ```<percorso cartella file>``` con l'input corretto

## Visualizza informazioni IPC

Il comando ```ipcs``` permette di vedere una lista di:
* code dei messaggi
* segmenti di memoria condivisa
* set di semafori

Esempio di output:
```
------ Message Queues --------
key     msqid  owner    perms  used-bytes  messages
0x1235  26     student  620    12          20

------ Shared Memory Segments --------
key     shmid  owner      perms  bytes  nattch  status
0x1234  0      professor  600    8192   2

------ Semaphore Arrays --------
key     semid  owner      perms  nsems
0x1111  102    professor  330    20
```

## Rimuovi IPC dal sistema:

Il comando ```ipcrm``` permette di rimuovere le IPC.

Rimuovere una coda dei messaggi:
```bash
# 0x1235 e' la chiave della coda
ipcrm -Q 0x1235

# 25 e' l'identificatore della coda
ipcrm -q 26
```

Rimuovere un segmento di memoria condivisa:
```bash
# 0x1234 e' la chiave della memoria condivisa
ipcrm -M 0x1234

# 0 e' l'identificatore della memoria condivisa
ipcrm -m 0
```

Rimuovere un set di semafori:
```bash
# 0x1111 e' la chiave del set di semafori
ipcrm -S 0x1111

# 102 e' l'identificatore del set di semafori
ipcrm -s 102
```
