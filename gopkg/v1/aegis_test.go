//
// Copyright (c) 2020, Rafael Santiago
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
package aegis

import (
    "testing"
    "os/exec"
    "io"
    "syscall"
    "runtime"
    "os"
    "fmt"
    "time"
)

const kTestSleepInSecs = 1

const kNextAttemptsNr = 65535

func hasGDB() bool {
    cmd := exec.Command("gdb", "--version")
    _, err := cmd.CombinedOutput()
    return (err == nil)
}

func hasLLDB() bool {
    cmd := exec.Command("lldb", "--version")
    _, err := cmd.CombinedOutput()
    return (err == nil)
}

func gdb() (io.WriteCloser, error) {
    cmd := exec.Command("gdb")
    go cmd.CombinedOutput()
    return cmd.StdinPipe()
}

func gdbAttach(gdb io.WriteCloser, pid int) error {
    _, err := io.WriteString(gdb, fmt.Sprintf("attach %d\n", pid))
    return err
}

func gdbContinue(gdb io.WriteCloser) error {
    _, err := io.WriteString(gdb, fmt.Sprintf("continue\n"))
    return err
}

func gdbNext(gdb io.WriteCloser) error {
    _, err := io.WriteString(gdb, fmt.Sprintf("next\n"))
    return err
}

func lldb() (io.WriteCloser, error) {
    cmd := exec.Command("lldb")
    go cmd.CombinedOutput()
    return cmd.StdinPipe()
}

func lldbAttach(lldb io.WriteCloser, pid int) error {
    _, err := io.WriteString(lldb, fmt.Sprintf("attach --pid %d\n", pid))
    return err
}

func lldbContinue(lldb io.WriteCloser) error {
    return gdbContinue(lldb)
}

func lldbNext(lldb io.WriteCloser) error {
    return gdbNext(lldb)
}

func runTestApp(appPath string) int {
    appProcAttr := syscall.ProcAttr {
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

func isProcessRunning(pid int) bool {
    var is bool
    if runtime.GOOS != "windows" {
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
        is = ((err != nil && len(data) > 0) || (len(data) > 0 && data[0] != 'Z' && data[0] != 'X'))
    } else {
        proc, err := os.FindProcess(pid)
        is = (err != nil || proc == nil)
    }
    return is
}

func TestHasDebugger(t *testing.T) {
    runGDBTests := hasGDB()
    if runtime.GOOS == "openbsd" {
        if runGDBTests == false {
            t.Error(`runGDBTests == false`)
        }
        if hasDebuggerOpenBSD() == false {
            t.Error(`hasDebuggerOpenBSD() == false`)
        }
        return
    }
    runLLDBTests := hasLLDB()
    if !runGDBTests && !runLLDBTests {
        t.Error(`!runGDBTests && !runLLDBTests: You need at least GDB or LLDB installed to execute tests.`)
    }

    if runGDBTests {
        wait4debugPath := "../../samples/golang-wait4debug"
        if runtime.GOOS == "windows" {
            wait4debugPath += ".exe"
        }

        // INFO(Rafael): Let's just attach and continue
        var pid int

        pid = runTestApp(wait4debugPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Second)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        var gdbProc io.WriteCloser
        var err error

        gdbProc, err = gdb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer gdbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = gdbAttach(gdbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        err = gdbContinue(gdbProc)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep(kTestSleepInSecs * time.Second)

        if isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(gdbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }

        // INFO(Rafael): Let's attach and try some next commands.

        pid = runTestApp(wait4debugPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        gdbProc, err = gdb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer gdbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = gdbAttach(gdbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        ntry := kNextAttemptsNr
        var isRunning bool = true
        for ntry > 0 && isRunning  {
            gdbNext(gdbProc)
            time.Sleep(1 * time.Nanosecond)
            ntry--
            isRunning = isProcessRunning(pid)
        }

        if isRunning {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(gdbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }
    }

    if runLLDBTests {
        wait4debugPath := "../../samples/golang-wait4debug"
        if runtime.GOOS == "windows" {
            wait4debugPath += ".exe"
        }

        // INFO(Rafael): Let's just attach and continue
        var pid int

        pid = runTestApp(wait4debugPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Second)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        var lldbProc io.WriteCloser
        var err error

        lldbProc, err = lldb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer lldbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = lldbAttach(lldbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        err = lldbContinue(lldbProc)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep(kTestSleepInSecs * time.Second)

        if isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(lldbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }

        // INFO(Rafael): Let's attach and try some next commands.

        pid = runTestApp(wait4debugPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        lldbProc, err = lldb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer lldbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = lldbAttach(lldbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        ntry := kNextAttemptsNr
        var isRunning bool = true
        for ntry > 0 && isRunning {
            lldbNext(lldbProc)
            time.Sleep(1 * time.Nanosecond)
            ntry--
            isRunning = isProcessRunning(pid)
        }

        if isRunning {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(lldbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }
    }
}

func TestSetGorgon(t *testing.T) {
    runGDBTests := hasGDB()
    if runtime.GOOS == "openbsd" {
        if runGDBTests == false {
            t.Error(`runGDBTests == false`)
        }
        setGorgonOpenBSD()
        return
    }
    runLLDBTests := hasLLDB()
    if !runGDBTests && !runLLDBTests {
        t.Error(`!runGDBTests && !runLLDBTests: You need at least GDB or LLDB installed to execute tests.`)
    }

    if runGDBTests {
        setgorgonPath := "../../samples/golang-setgorgon"
        if runtime.GOOS == "windows" {
            setgorgonPath += ".exe"
        }

        // INFO(Rafael): Let's just attach and continue
        var pid int

        pid = runTestApp(setgorgonPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Second)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        var gdbProc io.WriteCloser
        var err error

        gdbProc, err = gdb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer gdbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = gdbAttach(gdbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        err = gdbContinue(gdbProc)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep(kTestSleepInSecs * time.Second)

        if isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(gdbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }

        // INFO(Rafael): Let's attach and try some next commands.

        pid = runTestApp(setgorgonPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        gdbProc, err = gdb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer gdbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = gdbAttach(gdbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        ntry := kNextAttemptsNr
        var isRunning bool = true
        for ntry > 0 && isRunning {
            gdbNext(gdbProc)
            time.Sleep(1 * time.Nanosecond)
            ntry--
            isRunning = isProcessRunning(pid)
        }

        if isRunning {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(gdbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }
    }

    if runLLDBTests {
        setgorgonPath := "../../samples/golang-setgorgon"
        if runtime.GOOS == "windows" {
            setgorgonPath += ".exe"
        }

        // INFO(Rafael): Let's just attach and continue
        var pid int

        pid = runTestApp(setgorgonPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Second)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        var lldbProc io.WriteCloser
        var err error

        lldbProc, err = lldb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer lldbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = lldbAttach(lldbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        err = lldbContinue(lldbProc)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep(kTestSleepInSecs * time.Second)

        if isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(lldbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }

        // INFO(Rafael): Let's attach and try some next commands.

        pid = runTestApp(setgorgonPath)
        if pid == 0 {
            t.Error(`pid == 0`)
        }
        defer func(pid int) {
            proc, err := os.FindProcess(pid)
            if err == nil {
                proc.Kill()
            }
        }(pid)

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        if !isProcessRunning(pid) {
            t.Error(`isProcessRunning(pid) == false`)
        }

        lldbProc, err = lldb()
        if err != nil {
            t.Error(`err != nil`)
        }
        defer lldbProc.Close()

        time.Sleep(kTestSleepInSecs * time.Nanosecond)

        err = lldbAttach(lldbProc, pid)
        if err != nil {
            t.Error(`err != nil`)
        }

        time.Sleep((5 * kTestSleepInSecs) * time.Nanosecond)

        ntry := kNextAttemptsNr
        var isRunning bool = true
        for ntry > 0 && isRunning {
            lldbNext(lldbProc)
            time.Sleep(1 * time.Nanosecond)
            ntry--
            isRunning = isProcessRunning(pid)
        }

        if isRunning {
            t.Error(`isProcessRunning(pid) == true`)
        }

        _, err = io.WriteString(lldbProc, "quit")
        if err != nil {
            t.Error(`err != nil`)
        }
    }
}

func hasDebuggerOpenBSD() bool {
    const kBacalhuffy = `rm out.txt > /dev/null 2>&1
../../samples/golang-wait4debug > out.txt &
sleep 5
pid=$(cat out.txt | tail -n 1 | cut -d '=' -f 2,7 | cut -d ')' -f 1)
echo "attach ${pid}" > $HOME/.gdbinit
echo "continue" >> $HOME/.gdbinit
echo "quit" >> $HOME/.gdbinit
gdb > /dev/null 2>&1
rm ${HOME}/.gdbinit out.txt
ps -p ${pid} > /dev/null 2>&1
if [ $? -ne 0 ] ; then
    echo "OpenBSD Bacalhuffy info: program has exited."
    exit 0
else
    kill -9 ${pid}
    echo "OpenBSD Bacalhuffy error: program is still running."
    exit 1
fi
`
    fp, err := os.Create(".bacalhuffy.sh")
    if err != nil {
        return false
    }
    defer fp.Close()
    //defer os.Remove("bacalhuffy.sh")
    fp.WriteString(kBacalhuffy)

    cmd := exec.Command("chmod", "+x", ".bacalhuffy.sh")
    cmd.Start()

    cmd = exec.Command("./.bacalhuffy.sh")
    cmd.Start()

    if errCmd := cmd.Wait(); errCmd != nil {
        if _, ok := errCmd.(*exec.ExitError); ok {
            return false
        } else {
            return true
        }
    }

    return false
}

func setGorgonOpenBSD() bool {
    const kBacalhuffy = `rm out.txt > /dev/null 2>&1
../../samples/golang-setgorgon > out.txt &
sleep 5
pid=$(cat out.txt | tail -n 1 | cut -d '=' -f 2,7 | cut -d ')' -f 1)
echo "attach ${pid}" > $HOME/.gdbinit
echo "continue" >> $HOME/.gdbinit
echo "quit" >> $HOME/.gdbinit
gdb > /dev/null 2>&1
rm ${HOME}/.gdbinit out.txt
ps -p ${pid} > /dev/null 2>&1
if [ $? -ne 0 ] ; then
    echo "OpenBSD Bacalhuffy info: program has exited."
    exit 0
else
    kill -9 ${pid}
    echo "OpenBSD Bacalhuffy error: program is still running."
    exit 1
fi
`
    fp, err := os.Create(".bacalhuffy.sh")
    if err != nil {
        return false
    }
    defer fp.Close()
    //defer os.Remove("bacalhuffy.sh")
    fp.WriteString(kBacalhuffy)

    cmd := exec.Command("chmod", "+x", ".bacalhuffy.sh")
    cmd.Start()

    cmd = exec.Command("./.bacalhuffy.sh")
    cmd.Start()

    if errCmd := cmd.Wait(); errCmd != nil {
        if _, ok := errCmd.(*exec.ExitError); ok {
            return false
        } else {
            return true
        }
    }

    return false
}
