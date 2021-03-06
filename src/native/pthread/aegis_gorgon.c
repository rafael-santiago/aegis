/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <unistd.h>
#include <pthread.h>

#if defined(CGO)
# error You are compiling it from Cgo. Aegis uses native concurrency stuff from go. Do not complicate stuff buddy.
#endif

struct aegis_gorgon_exec_ctx {
    pthread_t thread;
    aegis_gorgon_exit_test_func should_exit;
    void *should_exit_args;
    aegis_gorgon_on_debugger_func on_debugger;
    void *on_debugger_args;
};

static struct aegis_gorgon_exec_ctx g_aegis_gorgon = { 0, NULL, NULL, NULL, NULL };

static void *aegis_gorgon_routine(void *args);

int aegis_set_gorgon(aegis_gorgon_exit_test_func exit_test, void *exit_test_args,
                     aegis_gorgon_on_debugger_func on_debugger, void *on_debugger_args) {
    pthread_attr_t gorgon_attr;
    int err = 1;
    if ((err = pthread_attr_init(&gorgon_attr)) == 0) {
        g_aegis_gorgon.should_exit = exit_test;
        g_aegis_gorgon.should_exit_args = exit_test_args;
        g_aegis_gorgon.on_debugger = (on_debugger != NULL) ? on_debugger : aegis_default_on_debugger;
        g_aegis_gorgon.on_debugger_args = on_debugger_args;
        err = pthread_create(&g_aegis_gorgon.thread, &gorgon_attr, aegis_gorgon_routine, &g_aegis_gorgon);
    }
    return err;
}

static void *aegis_gorgon_routine(void *args) {
    int stop = 0;
    struct aegis_gorgon_exec_ctx *exec = (struct aegis_gorgon_exec_ctx *)args;
    aegis_gorgon_exit_test_func should_exit = exec->should_exit;
    void *exit_args = exec->should_exit_args;
    aegis_gorgon_on_debugger_func on_debugger = exec->on_debugger;
    void *on_debugger_args = exec->on_debugger_args;
    while (!stop) {
        if (aegis_has_debugger()) {
            on_debugger(on_debugger_args);
        }
        if (should_exit != NULL) {
            stop = should_exit(exit_args);
        }
        usleep(1);
    }
    return NULL;
}
