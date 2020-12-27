//
// Copyright (c) 2020, Rafael Santiago
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
package aegis

import (
	"syscall"
)

func runTestApp(appPath string) int {
	var startInfo syscall.StartupInfo
	var procInfo syscall.ProcessInformation
	argv := syscall.StringToUTF16Ptr(appPath)
	err := syscall.CreateProcess(nil, argv, nil, nil, true, 0, nil, nil, &startInfo, &procInfo)
	if err != nil {
		return 0
	}
	return int(procInfo.ProcessId)
}

func isProcessRunningNative(pid int) bool {
	handle, err := syscall.OpenProcess(syscall.STANDARD_RIGHTS_REQUIRED|syscall.SYNCHRONIZE|0xfff, false, uint32(pid))
	is := (err == nil)
	if is {
		var exitCode uint32
		err = syscall.GetExitCodeProcess(handle, &exitCode)
		syscall.CloseHandle(handle)
		is = (err == nil && exitCode == 259)

	}
	return is
}
