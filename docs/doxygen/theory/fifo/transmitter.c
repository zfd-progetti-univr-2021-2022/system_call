#define LEN 250

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(){
    // salvo il percorso del file che gestisce la FIFO
    // > stesso file usato dal ricevitore
    char *fname = "/tmp/myfifo";

    // Apri la FIFO in scrittura
    int fd = open(fname, O_WRONLY);

    if (fd == -1) {
        printf("Non sono riuscito ad aprire in scrittura la FIFO\n");
        exit(1);
    }

    // Leggi da terminale una stringa da mandare al ricevitore
    char buffer[LEN];
    printf("Give me a string: ");
    fgets(buffer, LEN, stdin);

    // scrivi la stringa sulla FIFO
    ssize_t written_bytes = write(fd, buffer, strlen(buffer));

    if (written_bytes == -1) {
        printf("Non sono riuscito a scrivere i byte sulla FIFO\n");
        exit(1);
    }

    // chiudi la FIFO
    if (close(fd) == -1) {
        printf("Non sono riuscito a chiudere la FIFO\n");
        exit(1);
    }
}
