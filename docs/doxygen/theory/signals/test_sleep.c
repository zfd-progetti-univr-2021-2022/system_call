#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>


void sigHandler(int sig) {
    printf("\nMi hai svegliato!\n");
}


int main (int argc, char *argv[]) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        printf("Non sono riuscito ad impostare il nuovo signal handler\n");
        exit(1);
    }

    int time = 5;
    printf("Dormo per %d secondi!\n", time);

    int rem_time = sleep(time); // mi sospendo per al massimo 5 secondi

    if (rem_time == 0){
        printf("Ho dormito per tutti i %d secondi senza interruzioni\n", time);
    }
    else {
        printf("Volevo dormire altri %d secondi...\n", rem_time);
    }

    return 0;
}
