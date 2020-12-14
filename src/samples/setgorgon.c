/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int bye = 0;

void sigint_watchdog(int signo) {
    bye = 1;
}

int disable_gorgon(void *args) {
    return (*(int *)args);
}

void on_debugger(void *args) {
    fprintf(stdout, "\ninfo: debugger detected.\n");
    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);
    if (aegis_set_gorgon(disable_gorgon, &bye, on_debugger, NULL) != 0) {
        fprintf(stderr, "error: unable to set gorgon.\n");
        exit(1);
    }

    fprintf(stdout, "info: process started (pid=%d)...\n", getpid());
    while (!bye) {
        usleep(2);
    }

    fprintf(stdout, "\ninfo: gracefully exiting, no debugger was detected.\n");

    return 0;
}
