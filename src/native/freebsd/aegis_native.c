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

int aegis_has_debugger(void) {
    pid_t pid = getpid(), cpid;
    int pidinfo_args[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
    struct kinfo_proc kp;
    size_t kp_len = sizeof(kp);
    int is = 0;

    fflush(stdout);
    fflush(stdin);
    fflush(stderr);

    if ((cpid = fork()) == 0) {
        if (sysctl(pidinfo_args, nitems(pidinfo_args),
                   &kp, &kp_len, NULL, 0) == 0) {
            is = (kp.ki_stat == SSTOP || (kp.ki_flag & P_TRACED));
        }
        exit(is);
    } else {
        waitpid(cpid, &is, 0);
    }
    return (is != 0);
}
