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
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int aegis_has_debugger(void) {
    pid_t pid = getpid(), cpid;
    struct kinfo_proc kp;
    size_t kp_len = sizeof(kp);
    int pidinfo_args[6] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid, kp_len, 1 };
    int is = 0;
    if ((cpid = fork()) == 0) {
        if (sysctl(pidinfo_args, nitems(pidinfo_args),
                   &kp, &kp_len, NULL, 0) == 0) {
            is = (kp.p_stat == SSTOP || (kp.p_psflags & PS_TRACED));
        }
        exit(is);
    } else {
        waitpid(cpid, &is, 0);
    }
    return (is != 0);
}
