/// @file message_queue.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle CODE DEI MESSAGGI.

#include <sys/msg.h>
#include <errno.h>

#include "err_exit.h"
#include "message_queue.h"
#include "debug.h"

struct msqid_ds msqGetStats(int msqid){
    struct msqid_ds ds;

    if (msgctl(msqid, IPC_STAT, &ds) == -1)
        ErrExit("msgctl STAT");

    return ds;
}


void msqSetStats(int msqid, struct msqid_ds ds){

    if (msgctl(msqid, IPC_SET, &ds) == -1) {
        if(errno != EPERM) {
            ErrExit("msgctl SET");
        }
        else {
            DEBUG_PRINT("Couldn't set new config: not enough permissions. Continuing anyway\n");
        }
    }
}
