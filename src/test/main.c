/*
 *                          Copyright (C) 2020 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
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

#define TEST_SLEEP_IN_SECS 5

static FILE *g_test_proc = NULL;

static FILE *gdb(void);
static FILE *lldb(void);

static void dbg_attach(FILE *gdb, const pid_t pid);
static void dbg_continue(FILE *gdb);
static void dbg_next(FILE *gdb);

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
    int ntry = 20;
    int run_gdb_tests = 0, run_lldb_tests = 0;
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
        dbg_attach(gdb_proc, pid);
        dbg_continue(gdb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(gdb_proc);
        CUTE_ASSERT(!is_process_running(pid));

        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        gdb_proc = gdb();
        CUTE_ASSERT(gdb_proc != NULL);
        ntry = 10;
        dbg_attach(gdb_proc, pid);
        do {
            dbg_next(gdb_proc);
            usleep(2);
        } while (ntry-- > 0);
        fclose(gdb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        CUTE_ASSERT(!is_process_running(pid));
    }

    // INFO(Rafael): Testing detection with LLDB.
    if (run_lldb_tests) {
        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        lldb_proc = lldb();
        CUTE_ASSERT(lldb_proc != NULL);
        dbg_attach(lldb_proc, pid);
        dbg_continue(lldb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        fclose(lldb_proc);
        CUTE_ASSERT(!is_process_running(pid));

        pid = system_nowait(wait4debug_binarypath);
        CUTE_ASSERT(is_process_running(pid));
        lldb_proc = lldb();
        CUTE_ASSERT(lldb_proc != NULL);
        ntry = 10;
        dbg_attach(lldb_proc, pid);
        do {
            dbg_next(lldb_proc);
            usleep(2);
        } while (ntry-- > 0);
        fclose(lldb_proc);
#if defined(__FreeBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        CUTE_ASSERT(!is_process_running(pid));
    }

CUTE_TEST_CASE_END

CUTE_TEST_CASE(aegis_set_gorgon_tests)
CUTE_TEST_CASE_END

static FILE *gdb(void) {
    return popen("gdb", "w");
}

static void dbg_attach(FILE *dbg, const pid_t pid) {
    fprintf(dbg, "attach %d\n", pid);
}

static void dbg_continue(FILE *dbg) {
    fprintf(dbg, "continue\n");
}

static void dbg_next(FILE *dbg) {
    fprintf(dbg, "next\n");
}

static FILE *lldb(void) {
    return popen("lldb", "w");
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
    int is = 1;
    if (sysctl(pidinfo, nitems(pidinfo), &kp, &kp_len, NULL, 0) == 0) {
        is = (kp.ki_stat != SZOMB && kp.ki_stat != SSLEEP /*&& kp.ki_stat != SLOCK*/);
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
