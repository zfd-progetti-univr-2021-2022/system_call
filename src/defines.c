/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <sys/ipc.h>

#include "defines.h"
#include "err_exit.h"


key_t get_ipc_key() {
    key_t key = ftok(".", 'b');

    if (key == -1)
        ErrExit("ftok failed");

    return key;
}
