# system_call
Elaborato System Call per il Corso di Sistemi Operativi (2021-2022)

---

Work In Progress.

## Documentazione Doxygen

E' possibile raggiungere la [documentazione generata da Doxygen cliccando qui](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/index.html).

Navigando il menu' a tendina a sinistra e' possibile vedere:
* Commenti delle funzioni e dei parametri con grafici interattivi delle chiamate ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/files.html))
* Problemi o incomprensioni delle specifiche da risolvere ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/warning.html))
* Lista delle cose da implementare ([link diretto](https://zfd-progetti-univr-2021-2022.github.io/system_call/doxygen/html/todo.html))

## Comandi utili

Compilazione:
```bash
make
```
> Gli eseguibili finiranno nella cartella ```dist```

> NOTA: a volte non vengono rilevate le modifiche e quindi occorre cancellare gli eseguibili dalla cartella src

Terminazione del client:
1. Aprire un altro terminale
2. Eseguire il comando:

    ```bash
    kill -s SIGUSR1 $(pgrep client_0)
    ```

    > Prende l'output del comando pgrep, che restituisce il PID del processo con nome client_0, e usa il PID restituito per mandargli il segnale SIGUSR1 di terminazione.
