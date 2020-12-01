/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

struct aegis_set_gorgon_exec_ctx {
    pthread_t thread;
    aegis_gorgon_exit_func should_exit;
    void *args;
};

static struct aegis_set_gorgon_exec_ctx g_aegis_gorgon = { 0, NULL, NULL };

static void *aegis_gorgon_routine(void *args);

int aegis_has_debugger(void) {
    pid_t pid = getpid(), cpid;
    int pidinfo_args[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
    struct kinfo_proc kp;
    size_t kp_len = sizeof(kp);
    int is = 0;

    if ((cpid = fork()) == 0) {
        while (!is) {
            if (sysctl(pidinfo_args, nitems(pidinfo_args),
                       &kp, &kp_len, NULL, 0) == 0) {
                is = (kp.ki_stat == SSTOP);
            }
            usleep(1);
        }
        exit(is);
    } else {
        waitpid(cpid, &is, 0);
    }
    return is;
}

int aegis_set_gorgon(aegis_gorgon_exit_func func, void *args) {
    int err;
    pthread_attr_t gorgon_attr;

    if ((err = pthread_attr_init(&gorgon_attr)) == 0) {
        g_aegis_gorgon.should_exit = func;
        g_aegis_gorgon.args = args;
        err = pthread_create(&g_aegis_gorgon.thread, &gorgon_attr,
                             aegis_gorgon_routine, &g_aegis_gorgon);
    }

    return err;
}

static void *aegis_gorgon_routine(void *args) {
    int stop = 0;
    struct aegis_set_gorgon_exec_ctx *gorgon = (struct aegis_set_gorgon_exec_ctx *)args;
    aegis_gorgon_exit_func should_exit = gorgon->should_exit;
    void *exit_args = gorgon->args;

    while (!stop) {
        if (aegis_has_debugger()) {
            exit(1);
        }
        if (should_exit != NULL) {
            stop = should_exit(exit_args);
        }
    }

    return NULL;
}
