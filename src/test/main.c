/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <cutest.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#if defined(__FreeBSD__)
# include <sys/types.h>
# include <sys/user.h>
# include <sys/sysctl.h>
# include <unistd.h>
# include <sys/wait.h>
#endif

#define TEST_SLEEP_IN_SECS 1

static FILE *g_test_proc = NULL;

static FILE *gdb(void);
static FILE *lldb(void);

static void gdb_attach(FILE *gdb, const pid_t pid);
static void gdb_continue(FILE *gdb);
static void gdb_next(FILE *gdb);

static void lldb_attach(FILE *lldb, const pid_t pid);
static void lldb_continue(FILE *lldb);
static void lldb_next(FILE *lldb);

static int has_gdb(void);
static int has_lldb(void);

static int is_process_running(const pid_t pid);

static pid_t system_nowait(const char *command);

CUTE_DECLARE_TEST_CASE(aegis_tests);

CUTE_MAIN(aegis_tests);

CUTE_DECLARE_TEST_CASE(aegis_has_debugger_tests);
CUTE_DECLARE_TEST_CASE(aegis_set_gorgon_tests);

CUTE_TEST_CASE(aegis_tests)
    CUTE_RUN_TEST(aegis_has_debugger_tests);
    CUTE_RUN_TEST(aegis_set_gorgon_tests);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(aegis_has_debugger_tests)
    FILE *gdb_proc = NULL, *lldb_proc = NULL;
    pid_t pid;
    int ntry = 0;
    int run_gdb_tests = 0, run_lldb_tests = 0;
    int is_running = 0;
    char *wait4debug_binarypath =
#if defined(__unix__)
        "../../samples/wait4debug";
#elif defined(_WIN32)
        "../../samples/wait4debug.exe";
#else
# error Some code wanted.
#endif
    run_gdb_tests = has_gdb();
    run_lldb_tests = has_lldb();
    CUTE_ASSERT(run_gdb_tests || run_lldb_tests);

    // INFO(Rafael): Testing detection with GDB.
    if (run_gdb_tests) {
        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        gdb_proc = gdb();
        CUTE_ASSERT(gdb_proc != NULL);
        fprintf(gdb_proc, "set print inferior-events off\n");
        gdb_attach(gdb_proc, pid);
        gdb_continue(gdb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(gdb_proc);
        is_running = is_process_running(pid);
        if (is_running) {
            kill(pid, SIGKILL);
        }
        CUTE_ASSERT(!is_running);

        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        gdb_proc = gdb();
        CUTE_ASSERT(gdb_proc != NULL);
        fprintf(gdb_proc, "set print inferior-events off\n");
        ntry = 10;
        gdb_attach(gdb_proc, pid);
        do {
            gdb_next(gdb_proc);
            usleep(2);
        } while (ntry-- > 0);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(gdb_proc);
        is_running = is_process_running(pid);
        if (is_running) {
            kill(pid, SIGKILL);
        }
        CUTE_ASSERT(!is_running);
    }

    // INFO(Rafael): Testing detection with LLDB.
    if (run_lldb_tests) {
        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        lldb_proc = lldb();
        CUTE_ASSERT(lldb_proc != NULL);
        lldb_attach(lldb_proc, pid);
        lldb_continue(lldb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(lldb_proc);
        is_running = is_process_running(pid);
        if (is_running) {
            kill(pid, SIGKILL);
        }
        CUTE_ASSERT(!is_running);

        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        lldb_proc = lldb();
        CUTE_ASSERT(lldb_proc != NULL);
        ntry = 10;
        lldb_attach(lldb_proc, pid);
        do {
            lldb_next(lldb_proc);
            usleep(2);
        } while (ntry-- > 0);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(lldb_proc);
        is_running = is_process_running(pid);
        if (is_running) {
            kill(pid, SIGKILL);
        }
        CUTE_ASSERT(!is_running);
    }
    sleep(TEST_SLEEP_IN_SECS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(aegis_set_gorgon_tests)
CUTE_TEST_CASE_END

static FILE *gdb(void) {
    return popen("gdb", "w");
}

static void gdb_attach(FILE *gdb, const pid_t pid) {
    fprintf(gdb, "attach %d\n", pid);
}

static void gdb_continue(FILE *gdb) {
    fprintf(gdb, "continue\n");
}

static void gdb_next(FILE *gdb) {
    fprintf(gdb, "next\n");
}

static FILE *lldb(void) {
    return popen("lldb", "w");
}

static void lldb_attach(FILE *lldb, const pid_t pid) {
    fprintf(lldb, "attach --pid %d\n", pid);
}

static void lldb_continue(FILE *lldb) {
    fprintf(lldb, "continue\n");
}

static void lldb_next(FILE *lldb) {
    fprintf(lldb, "next\n");
}

static int has_gdb(void) {
    return (system("gdb --version > /dev/null 2>&1") == 0);
}

static int has_lldb(void) {
    return (system("lldb --version > /dev/null 2>&1") == 0);
}

static int is_process_running(const pid_t pid) {
#if defined(__linux__)
    char proc_buf[1024], *bp, *bp_end;
    FILE *fp;
    int is = 1;
    snprintf(proc_buf, sizeof(proc_buf) - 2, "/proc/%d/stat", pid);
    fp = fopen(proc_buf, "r");
    is = (fp != NULL);
    if (is) {
        memset(proc_buf, 0, sizeof(proc_buf));
        fread(proc_buf, 1, sizeof(proc_buf), fp);
        bp_end = &proc_buf[0] + strlen(proc_buf);
        bp = strstr(proc_buf, ") ");
        if (bp != NULL && (bp + 2) < bp_end) {
            bp += 2;
            is = (*bp != 'Z');
        }
        fclose(fp);
    }
    return is;
#elif defined(__FreeBSD__)
    int pidinfo[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
    struct kinfo_proc kp;
    size_t kp_len = sizeof(kp);
    int is = (sysctl(pidinfo, nitems(pidinfo), &kp, &kp_len, NULL, 0) == 0);
    if (is) {
        is = (kp.ki_stat == SRUN && (kp.ki_flag & P_TRACED) == 0);
    }
    return is;
#else
# Some code wanted.
#endif
}

static pid_t system_nowait(const char *command) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execl(command, command, NULL);
        exit(1);
    }
    return pid;
}
