/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int aegis_has_debugger(void) {
    int has = 0;
    FILE *fp = NULL;
    char proc_filepath[1024], proc_buf[1024];
    char *bp = NULL, *bp_end = NULL;
    ssize_t proc_buf_size = 0;
    pid_t pid = getpid();

    if (fork() == 0) {
        snprintf(proc_filepath, sizeof(proc_filepath) - 2, "/proc/%d/stat", pid);
        if ((fp = fopen(proc_filepath, "r")) != NULL) {
            memset(proc_buf, 0, sizeof(proc_buf));
            proc_buf_size = fread(proc_buf, 1, sizeof(proc_buf) - 2, fp);
            fclose(fp);
            if ((bp = strstr(proc_buf, ")")) != NULL) {
                has = ((bp + 2) < bp_end && bp[2] == 't');
            }
        }
        if (!has) {
            snprintf(proc_filepath, sizeof(proc_filepath) - 2, "/proc/%d/stack", pid);
            if ((fp = fopen(proc_filepath, "r")) != NULL) {
                memset(proc_buf, 0, sizeof(proc_buf));
                proc_buf_size = fread(proc_buf, 1, sizeof(proc_buf) - 2, fp);
                fclose(fp);
                has = (strstr(proc_buf, "ptrace_stop") != NULL);
            }
        }
        exit(has);
    } else {
        wait(&has);
    }

    return (has != 0);
}
