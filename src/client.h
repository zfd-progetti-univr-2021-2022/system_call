/// @file client.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del CLIENT.

#pragma once

/**
 * @brief Handler del segnale SIGINT.
 *
 * @todo Gestire la parte di comunicazione con il server.
 *
 * Esegue tutte le funzionalita' principali del client.
 *
 * @param sig Valore intero corrispondente a SIGINT
*/
void SIGINTSignalHandler(int sig);

/**
 * @brief Handler del segnale SIGUSR1.
 *
 * @todo Gestire la chiusura delle risorse. Potrebbe non essere necessario: dipende dalla implementazione delle altre funzioni.
 *
 * Termina il processo del client 0.
 *
 * @param sig Valore intero corrispondente a SIGUSR1
*/
void SIGUSR1SignalHandler(int sig);

/**
 * @brief Divide in 4 parti il contenuto del file da inviare al server.
 *
 * @param fd File descriptor del file da inviare
 * @param buf Buffer in cui memorizzare la porzione del messaggio
 * @param count Dimensione della porzione di messaggio
 * @param filePath Percorso del file da suddividere
 * @param parte Numero identificativo della porzione di messaggio
*/
void dividi(int fd, char *buf, size_t count, char *filePath, int parte);

/**
 * @brief Funzione eseguita da ogni Client i (figli di Client 0) per mandare i file al server.
 *
 * @todo Aggiungere la sincronizzazione con gli altri client
 *
 * @todo Aggiungere la parte di comunicazione delle quattro parti dei messaggi
 *
 * @warning siccome l'ultima parte del messaggio e' l'unica che puo' essere piu' corta per specifica... Cosa bisogna fare in casi in cui non e' possibile garantire questo vincolo?
 *          Esempio: 2 caratteri possono essere divisi in:
 *          - caratteri per parte: 1 1 0 0
 *          - oppure: 2 0 0 0
 *
 * Lo stesso problema si pone per 1, 2, 5, 6, 9, 10, ... caratteri
 *
 * Non posso garantire come ad esempio nel caso di 3 caratteri che sono l'ultimo numero sia inferiore:
 * Esempio di suddivisione di 3 caratteri: 1 1 1 0. L'ultimo, come per specifica, e' l'unico di dimensione inferiore
 *
 * @param filePath Percorso del file che il client deve suddividere e mandare al server.
 *
*/
void operazioni_figlio(char * filePath);

/**
 * Memorizza il percorso passato come parametro,
 * gestisce segnali e handler, attende i segnali SIGINT o SIGUSR1.
 *
 * @param argc Numero argomenti passati da linea di comando (compreso il nome dell'eseguibile)
 * @param argv Array di argomenti passati da linea di comando
 * @return int Exit code dell'intero programma
*/
int main(int argc, char * argv[]);
