/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <debugapi.h>
#include <intrin.h>
#include <winternl.h>
#include <processthreadsapi.h>
#include <synchapi.h>

struct aegis_gorgon_exec_ctx {
    HANDLE thread;
    aegis_gorgon_exit_test_func should_exit;
    void *should_exit_args;
    aegis_gorgon_on_debugger_func on_debugger;
    void *on_debugger_args;
};

static struct aegis_gorgon_exec_ctx g_aegis_gorgon = { 0, NULL, NULL, NULL, NULL };

static DWORD WINAPI aegis_gorgon_routine(LPVOID args);

int aegis_has_debugger(void) {
	int has = (IsDebuggerPresent() != FALSE);
	PPEB peb = NULL;
	DWORD nt_global_flag = 0;
	if (!has) {
#if defined(_WIN64)
		peb = (PPEB) __readgsqword(0x60);
#elif defined(_WIN32)
		peb = (PPEB) __readfsdword(0x30);
#else
# error Some code wanted.
#endif
		has = peb->BeingDebugged;
		
		if (!has) {
#if defined(_WIN64)
			nt_global_flag = *(PDWORD)((PBYTE)peb + 0xBC);
#elif defined(_WIN32)
			nt_global_flag = *(PDWORD)((PBYTE)peb + 0x68);
#else
# error Some code wanted.
#endif
			has = ((nt_global_flag & 0x70) != 0);
		}
	}
	return has;
}

int aegis_set_gorgon(aegis_gorgon_exit_test_func exit_test, void *exit_test_args,
                     aegis_gorgon_on_debugger_func on_debugger, void *on_debugger_args) {
    int err = 1;
    g_aegis_gorgon.should_exit = exit_test;
    g_aegis_gorgon.should_exit_args = exit_test_args;
    g_aegis_gorgon.on_debugger = (on_debugger != NULL) ? on_debugger : aegis_default_on_debugger;
    g_aegis_gorgon.on_debugger_args = on_debugger_args;
    g_aegis_gorgon.thread = CreateThread(NULL, 0, aegis_gorgon_routine, &g_aegis_gorgon, 0, NULL);
	if (g_aegis_gorgon.thread != NULL) {
		err = 0;
	}
    return err;
}

static DWORD WINAPI aegis_gorgon_routine(LPVOID args) {
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
        Sleep(1);
    }
    return 0;
}
