#define LEN 250

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(){
    // salvo il percorso del file che gestisce la FIFO
    // > stesso file usato dal trasmettitore
    char *fname = "/tmp/myfifo";

    // creo la FIFO passandogli il percorso e i permessi
    int res = mkfifo(fname, S_IRUSR|S_IWUSR);

    if (res == -1) {
        // posso usare ErrExit()
        printf("Non sono riuscito a creare la FIFO\n");
        exit(1);
    }

    // Apro in lettura la FIFO
    int fd = open(fname, O_RDONLY);

    if (fd == -1) {
        printf("Non sono riuscito ad aprire in lettura la FIFO\n");
        exit(1);
    }

    // leggi byte dalla FIFO e memorizza nel buffer
    char buffer[LEN];
    if (read(fd, buffer, LEN) == -1) {
        printf("Non sono riuscito a leggere la FIFO\n");
        exit(1);
    }

    // Visualizza cio' che e' stato letto
    printf("%s\n", buffer);

    // chiudi la FIFO
    if (close(fd) == -1) {
        printf("Non sono riuscito a chiudere la FIFO\n");
        exit(1);
    }

    // elimina file della FIFO (o togli symlink se e' un collegamento)
    if (unlink(fname)) {
        printf("Elimina file della FIFO\n");
        exit(1);
    }
}
