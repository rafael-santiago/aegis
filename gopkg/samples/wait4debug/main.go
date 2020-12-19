//
// Copyright (c) 2020, Rafael Santiago
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
package main

import (
    "os"
    "os/signal"
    "syscall"
    "time"
    "fmt"
    "github.com/rafael-santiago/aegis/gopkg"
)

func main() {
    go func() {
        sigintWatchdog := make(chan os.Signal, 1)
        signal.Notify(sigintWatchdog, os.Interrupt)
        signal.Notify(sigintWatchdog, syscall.SIGINT | syscall.SIGTERM)
        <-sigintWatchdog
        fmt.Fprintf(os.Stdout, "\rinfo: ctrl + C received. Aborted.\n")
        os.Exit(1)
    }()
    fmt.Fprintf(os.Stdout, "info: Waiting for debug attachment (pid=%d)...\n", os.Getpid())
    for !aegis.HasDebugger() {
        time.Sleep(1 * time.Nanosecond)
    }
    fmt.Fprintf(os.Stdout, "\rinfo: Debugged detected. Go home!\n")
}
