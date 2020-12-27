//
// Copyright (c) 2020, Rafael Santiago
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
// +build !windows

package aegis

import (
	"os"
	"os/exec"
	"syscall"
	"time"
)

func runTestApp(appPath string) int {
	appProcAttr := syscall.ProcAttr{
		"",
		[]string{},
		[]uintptr{os.Stdin.Fd(), os.Stdout.Fd(), os.Stderr.Fd()},
		nil,
	}
	pid, err := syscall.ForkExec(appPath, []string{""}, &appProcAttr)
	if err != nil {
		return 0
	}
	return pid
}

func isProcessRunningNative(pid int) bool {
	cmd := exec.Command("ps", "-o", "stat=", "-p", fmt.Sprintf("%d", pid))
	data, err := cmd.CombinedOutput()
	if len(data) > 0 && data[0] == 't' {
		ntry := 20
		for ntry > 0 && len(data) > 0 && data[0] == 't' {
			time.Sleep(kTestSleepInSecs * time.Second)
			data, err = cmd.CombinedOutput()
			ntry--
		}
	}
	// INFO(Rafael): In doubt when not possible running ps let's assume the process as running.
	return ((err != nil && len(data) > 0) || (len(data) > 0 && data[0] != 'Z' && data[0] != 'X'))
}
