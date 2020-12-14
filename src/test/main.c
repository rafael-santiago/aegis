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
#elif defined(__NetBSD__)
# include <sys/types.h>
# include <sys/proc.h>
# include <kvm.h>
# include <sys/sysctl.h>
# include <unistd.h>
# include <sys/wait.h>
#elif defined(_WIN32)
# include <windows.h>
#endif

#define TEST_SLEEP_IN_SECS 1

#if !defined(__OpenBSD__)

static FILE *g_test_proc = NULL;

static FILE *gdb(void);
static FILE *lldb(void);

static void gdb_attach(FILE *gdb, const pid_t pid);
static void gdb_continue(FILE *gdb);
static void gdb_next(FILE *gdb);

static void lldb_attach(FILE *lldb, const pid_t pid);
static void lldb_continue(FILE *lldb);
static void lldb_next(FILE *lldb);

static int is_process_running(const pid_t pid);

static pid_t system_nowait(const char *command);

#endif

static int has_gdb(void);
static int has_lldb(void);

CUTE_DECLARE_TEST_CASE(aegis_tests);

CUTE_MAIN(aegis_tests);

CUTE_DECLARE_TEST_CASE(aegis_has_debugger_tests);
CUTE_DECLARE_TEST_CASE(aegis_set_gorgon_tests);

CUTE_TEST_CASE(aegis_tests)
    CUTE_RUN_TEST(aegis_has_debugger_tests);
    CUTE_RUN_TEST(aegis_set_gorgon_tests);
CUTE_TEST_CASE_END

#if defined(_WIN32)

#define SIGKILL 9

void kill(pid_t pid, int dummy) {
    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (proc != NULL) {
        TerminateProcess(proc, 0);
        CloseHandle(proc);
    }
}
#endif

#if !defined(__OpenBSD__)

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
#if defined(__FreeBSD__) || defined(__NetBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        pclose(gdb_proc);
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
#if defined(__FreeBSD__) || defined(__NetBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        pclose(gdb_proc);
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
#if defined(__FreeBSD__) || defined(__NetBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        pclose(lldb_proc);
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
#if defined(__FreeBSD__) || defined(__NetBSD__)
        sleep(TEST_SLEEP_IN_SECS);
#endif
        pclose(lldb_proc);
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
#elif defined(__NetBSD__)
    struct kinfo_proc2 kp;
    size_t kp_len = sizeof(kp);
    int pidinfo[6] = { CTL_KERN, KERN_PROC2, KERN_PROC_PID, (int)pid,
                       sizeof(kp), 1 };
    int is = (sysctl(pidinfo, __arraycount(pidinfo), &kp, &kp_len,
                     NULL, 0) == 0);
    if (is) {
        is = (kp.p_stat == LSRUN && (kp.p_flag & P_TRACED) == 0);
    }
    return is;
#elif defined(_WIN32)
    HANDLE proc = OpenProcess(SYNCHRONIZE, FALSE, pid);
    int is = (proc != NULL);
    DWORD retval;
    if (is) {
        retval = WaitForSingleObject(proc, 0);
        is = (retval == WAIT_TIMEOUT);
        CloseHandle(proc);
    }
    return is;
#else
# Some code wanted.
#endif
}

static pid_t system_nowait(const char *command) {
#if defined(__unix__)
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execl(command, command, NULL);
        exit(1);
    }
    return pid;
#else
    pid_t pid = 0;
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi =  { 0 };
    if (CreateProcess(command, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) != FALSE) {
        pid = pi.dwProcessId;
    }
    return pid;
#endif
}

#elif defined(__OpenBSD__)

CUTE_TEST_CASE(aegis_has_debugger_tests)
    // WARN(Rafael): On OpenBSD all fork stuff done on Linux, FreeBSD and NetBSD looks really messy.
    //               In order to defeat this specific system complication let's combat it with simplicity.
    //               This is ugly but works. Better: without sucking anything that already works well
    //               on other systems. Until now let's stick with it for OpenBSD.
    //
    //               If you are not a brazilian programmer maybe you will never understand the
    //               reason of the pun with ('bacalhau' + 'puffy') = 'bacalhuffy'.
    const char *bacalhuffy_sh = "rm out.txt > /dev/null 2>&1\n"
                                "../../samples/wait4debug > out.txt &\n"
                                "sleep 5\n"
                                "pid=$(cat out.txt | tail -n 1 | cut -d '=' -f 2,7 | cut -d ')' -f 1)\n"
                                "echo \"attach ${pid}\" > .gdbinit\n"
                                "echo \"continue\" >> .gdbinit\n"
                                "echo \"quit\" >> .gdbinit\n"
                                "gdb > /dev/null 2>&1\n"
                                "rm .gdbinit out.txt\n"
                                "ps -p ${pid} > /dev/null 2>&1\n"
                                "if [ $? -ne 0 ] ; then\n"
                                "       echo \"OpenBSD Bacalhuffy info: program has exited.\"\n"
                                "       exit 0\n"
                                "else\n"
                                "       kill -9 ${pid}\n"
                                "       echo \"OpenBSD Bacalhuffy error: program is still running.\"\n"
                                "       exit 1\n"
                                "fi\n";
    FILE *fp;
    int ntry = 20;
    int exit_code;
    CUTE_ASSERT(has_gdb());
    fp = fopen(".bacalhuffy.sh", "w");
    CUTE_ASSERT(fp != NULL);
    fprintf(fp, "%s", bacalhuffy_sh);
    fclose(fp);
    CUTE_ASSERT(system("chmod +x .bacalhuffy.sh") == 0);
    do {
        exit_code = system("./.bacalhuffy.sh");
        if (exit_code != 0 && ntry > 1) {
            sleep(TEST_SLEEP_IN_SECS);
        }
    } while (ntry-- > 1 && exit_code != 0);
    remove(".bacalhuffy.sh");
    CUTE_ASSERT(exit_code == 0);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(aegis_set_gorgon_tests)
CUTE_TEST_CASE_END

#endif

static int has_gdb(void) {
#if defined(__unix__)
    return (system("gdb --version > /dev/null 2>&1") == 0);
#elif defined(_WIN32)
    return (system("gdb --version > nul 2>&1") == 0);
#else
# error Some code wanted.
#endif
}

static int has_lldb(void) {
#if defined(__unix__)
    return (system("lldb --version > /dev/null 2>&1") == 0);
#elif defined(_WIN32)
    return (system("lldb --version > nul 2>&1") == 0);
#else
# error Some code wanted.
#endif
}
