/// @file signals.c
/// @brief Contiene le funzioni specifiche per la gestione dei SEGNALI.
#include <signal.h>

#include "signals.h"


void block_sig_no_SIGINT_SIGUSR1(){
    sigset_t signalSet, prevSignalSet;
    sigfillset(&signalSet);                                // inizializzo signalSet che contiene tutti i segnali
    sigdelset(&signalSet, SIGINT);                         // elimino SIGINT dalla lista dei segnali bloccati
    sigdelset(&signalSet, SIGUSR1);                        // elimino SIGUSR1 dalla lista dei segnali bloccati
    sigprocmask(SIG_SETMASK, &signalSet, &prevSignalSet);  // imposto nuova maschera dei segnali
}


void block_all_signals(){
    sigset_t signalSet, prevSignalSet;
    sigfillset(&signalSet);                                // inizializzo signalSet che contiene tutti i segnali
    sigprocmask(SIG_SETMASK, &signalSet, &prevSignalSet);  // imposto nuova maschera dei segnali
}
