/// @file client.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del CLIENT.

#pragma once

/**
 * @brief Handler del segnale SIGINT.
 *
 * @todo Gestire la parte di comunicazione con il server.
 *
 * @todo Potrebbe essere necessario ottimizzare l'uso dello HEAP nel client 0.
 *       Attualmente la dimensione massima occupata dal client e':
 *       $$100 file * dim. path + 100 file * dim. pointer$$.
 *       Assumento dim. path massima = 250 caratteri e dim. pointer 4 byte (32 bit):
 *       $$100 * 250 + 100 * 4 = 25400$$
 *       Quindi solo lato client si occupano 25 KByte.
 *       Altro problema: ogni client i eredita la lista creando al massimo altre 100 liste concatenate?
 *       In quel caso si occuperebbero:
 *       $$25 + 100 * 25 = 2525$$
 *       Ovvero 2525 KB, circa 2.5 MB.
 *
 * @warning Per ottimizzare l'uso dello HEAP nel client 0 si potrebbe
 *          prima cercare e contare quanti file sono presenti senza creare una lista concatenata
 *          e poi ricercare i file e man mano che si trovano file send_me si puo' creare il
 *          processo figlio per inviare il file. Per fare questo BISOGNA sapere se il numero di file
 *          puo' cambiare durante l'esecuzione del programma:
 *          se trovo 3 file e dopo un file viene cancellato cosa succede? <br>
 *          NOTA: questo problema puo' esserci anche nella situazione attuale...
 *
 * @warning Il client 0 deve attendere i processi figlio? La specifica indica solo che bisogna attendere il messaggio di fine dal server...
 *          Probabilmente prima bisogna attendere il messaggio di fine e poi aspettare che tutti i figlio terminino (prima di liberare la lista dei file e).
 *
 * @warning Il percorso passato al client deve essere assoluto o puo' essere relativo? Se si passa un percorso relativo chdir() fallira' alla seconda esecuzione. <br>
 *          SOLUZIONE: si potrebbe usare un altro chdir() a fine funzione per tornare al percorso di esecuzione iniziale anticipando il chdir() successivo.
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
 *          - oppure: 2 0 0 0 <br>
 *          Lo stesso problema si pone per 1, 2, 5, 6, 9, 10, ... caratteri. <br>
 *          Non posso garantire come ad esempio nel caso di 3 caratteri che sono l'ultimo numero sia inferiore:
 *          Esempio di suddivisione di 3 caratteri: 1 1 1 0. L'ultimo, come per specifica, e' l'unico di dimensione inferiore
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
