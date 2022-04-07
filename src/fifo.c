/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "err_exit.h"
#include "fifo.h"


void make_fifo(char * path) {
    if (mkfifo(path, IPC_CREAT | S_IRUSR | S_IWUSR) == -1) {
        ErrExit("mkfifo failed");
    }
}


int create_fifo(char * path, char mode) {
    int fifo1_fd = -1;

    make_fifo(path);

    if (mode == 'r') {
        fifo1_fd = open(path, O_RDONLY);
        if (fifo1_fd == -1) {
            ErrExit("[fifo.c:create_fifo] open FIFO1 failed (read mode)");
        }
    }
    else if (mode == 'w') {
        fifo1_fd = open(path, O_WRONLY);
        if (fifo1_fd == -1) {
            ErrExit("[fifo.c:create_fifo] open FIFO1 failed (write mode)");
        }
    }
    else {
        printf("[fifo.c:create_fifo] mode should be 'r' or 'w'\n");
        exit(1);
    }

    return fifo1_fd;
}


int create_new_fifo(char * path, char mode) {

    if (unlink(path) == -1) {
        if (errno != ENOENT) {
            ErrExit("[fifo.c:create_new_fifo] unlink failed\n");
        }
    }

    return create_fifo(path, mode);
}
