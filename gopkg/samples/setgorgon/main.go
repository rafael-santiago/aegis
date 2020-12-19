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

// INFO(Rafael): This function flags for aegis. SetGorgon if it is time to stop watching and exit.
func shouldExit(exit interface{}) bool {
    exitChan := exit.(chan bool)
    var should bool = false
    timeout := time.Tick(1 * time.Nanosecond)
    select {
        case should = <-(exitChan):
        case <-timeout:
    }
    return should
}

func main() {
    exit := make(chan bool, 1)
    go func(exit chan<- bool) {
        sigintWatchdog := make(chan os.Signal, 1)
        signal.Notify(sigintWatchdog, os.Interrupt)
        signal.Notify(sigintWatchdog, syscall.SIGINT | syscall.SIGTERM)
        <-sigintWatchdog
        exit <- true
        fmt.Fprintf(os.Stdout, "\rinfo: ctrl + c received from the user. Exiting...\n")
    }(exit)
    fmt.Fprintf(os.Stdout, "info: process started (pid=%d)...\n", os.Getpid())
    // INFO(Rafael): Let's be less ambitious here. We will just pass our gracefully exit check function and
    //               its argument. OnDebugger and OnDebuggerArgs will be null, with this we will use the
    //               default Aegis' on debugger function that is only about a gross os.Exit(1) >:P
    go aegis.SetGorgon(shouldExit, exit, nil, nil)
    // INFO(Rafael): All your sensitive instructions not suitable for eavesdroppers would go here.
    //               On some requirements you will need to flush some buffers, files and stuff before exiting.
    //               For those cases you would pass your custom OnDebugger and OnDebuggerArgs to Aegis' Gorgon.
    <-exit
}
