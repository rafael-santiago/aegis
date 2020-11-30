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

static FILE *g_test_proc = NULL;

static FILE *gdb(void);
static void gdb_attach(FILE *gdb, const pid_t pid);
static void gdb_continue(FILE *gdb);
static void gdb_next(FILE *gdb);

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
    FILE *gdb_proc = NULL;
    pid_t pid;
    int ntry = 20;
    char *wait4debug_binarypath =
#if defined(__unix__)
        "../../samples/wait4debug";
#elif defined(_WIN32)
        "../../samples/wait4debug.exe";
#else
# error Some code wanted.
#endif
    pid = system_nowait(wait4debug_binarypath);
    CUTE_ASSERT(is_process_running(pid));
    gdb_proc = gdb();
    CUTE_ASSERT(gdb_proc != NULL);
    gdb_attach(gdb_proc, pid);
    gdb_continue(gdb_proc);
#if defined(__FreeBSD__)
    sleep(1);
#endif
    fclose(gdb_proc);
    CUTE_ASSERT(!is_process_running(pid));

    pid = system_nowait(wait4debug_binarypath);
    CUTE_ASSERT(is_process_running(pid));
    gdb_proc = gdb();
    CUTE_ASSERT(gdb_proc != NULL);
    ntry = 10;
    gdb_attach(gdb_proc, pid);
    do {
        gdb_next(gdb_proc);
        usleep(2);
    } while (ntry-- > 0);
    fclose(gdb_proc);
#if defined(__FreeBSD__)
    sleep(1);
#endif
    CUTE_ASSERT(!is_process_running(pid));
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
