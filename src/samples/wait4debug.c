#include <aegis.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sigint_watchdog(int signo) {
    printf("\nCanceled.\n");
    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);
    printf("*** Waiting for debug attachment (pid=%d)...\n", getpid());
    while (!aegis_has_debugger()) {
        usleep(1);
    }
    printf("*** Debugger is attached.\n");
    return 0;
}
