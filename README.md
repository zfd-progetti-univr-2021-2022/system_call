# system_call
Elaborato System Call per il Corso di Sistemi Operativi (2021-2022)

## Documentazione Doxygen

E' possibile raggiungere la [documentazione generata da Doxygen cliccando qui](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/index.html).

Navigando il menu' a tendina a sinistra e' possibile vedere:
* Commenti delle funzioni e dei parametri con grafici interattivi delle chiamate ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/files.html))
* Problemi o incomprensioni delle specifiche da risolvere ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/warning.html)) o gia' risolte ([link](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/md_theory_questions_questions.html))
* Lista delle cose da implementare ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/todo.html))
* Comandi utili per il debugging e per l'analisi dei memory leak ([link](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/md_theory_commands_commands.html))
* Riassunti di teoria

## Comandi utili

Compilazione:
```bash
make
```
> Gli eseguibili finiranno nella cartella ```dist```

Terminazione del client:
1. Aprire un altro terminale
2. Eseguire il comando:

    ```bash
    kill -s SIGUSR1 $(pgrep client_0)
    ```

    > Prende l'output del comando pgrep, che restituisce il PID del processo con nome client_0, e usa il PID restituito per mandargli il segnale SIGUSR1 di terminazione.

E' possibile trovare altri [comandi utili sulla documentazione doxygen](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/md_theory_commands_commands.html)

## IPC

Set di semafori ```semid```:

|Identificativo|Valore Iniziale|Descrizione                                                                                                                                                       |
|--------------|---------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|0             |1              |Mutex per leggere/scrivere il messaggio con il numero N di file sulla memoria condivisa                                                                           |
|1             |0              |Incrementato a 2 in runtime per prevedere il ciclo successivo. Aspetta che client e server abbiano terminato operazioni sulle FIFO prima di renderle NON bloccanti|
|2             |0              |Incrementato a 2 in runtime per prevedere il ciclo successivo. Aspetta che client e server abbiano reso le FIFO NON bloccanti                                     |
|3             |0              |Incrementato a 2 in runtime per prevedere il ciclo successivo. Aspetta che client e server abbiano terminato operazioni sulle FIFO prima di renderle bloccanti    |
|4             |0              |Incrementato a 2 in runtime per prevedere il ciclo successivo. Aspetta che client e server abbiano reso le FIFO bloccanti                                         |
|5             |0              |Incrementato in runtime per valere tanto quanto sono il numero di file/figli. Aspetta che tutti i processi figlio di client_0 abbiano suddiviso il proprio file in 4 parti prima di mandarle sulle IPC o FIFO|
|6             |1              |Mutex per leggere/scrivere messaggi con la quarta parte di file sulla memoria condivisa|
|7             |50             |Limita il numero di messaggi memorizzabili contemporaneamente nella FIFO 1 a 50 messaggi|
|8             |50             |Limita il numero di messaggi memorizzabili contemporaneamente nella FIFO 2 a 50 messaggi|
|9             |50             |Limita il numero di messaggi memorizzabili contemporaneamente nella coda dei messaggi a 50 messaggi|
