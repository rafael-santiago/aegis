//
// Copyright (c) 2020, Rafael Santiago
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
package aegis

/*
#cgo CFLAGS:  -I../../src -DCGO=1
#include <aegis.h>
#include <aegis.c>
#if defined(__linux__)
# include <native/linux/aegis_native.c>
#elif defined(__FreeBSD__)
# include <native/freebsd/aegis_native.c>
#elif defined(__NetBSD__)
# include <native/netbsd/aegis_native.c>
#elif defined(__OpenBSD__)
# include <native/openbsd/aegis_native.c>
#elif defined(_WIN32)
# include <native/windows/aegis_native.c>
#endif
*/
import "C"
import (
    "os"
    "time"
//    "fmt"
)

type AegisGorgonExitFunc func(args interface{})bool

type AegisGorgonOnDebuggerFunc func(args interface{})

func HasDebugger() bool {
    return (C.aegis_has_debugger() == 1)
}

func SetGorgon(exitFunc AegisGorgonExitFunc, exitFuncArgs interface{},
               onDebuggerFunc AegisGorgonOnDebuggerFunc, OnDebuggerFuncArgs interface{}) {
    var onDebugger AegisGorgonOnDebuggerFunc

    if onDebuggerFunc != nil {
        onDebugger = onDebuggerFunc
    } else {
        onDebugger = defaultOnDebugger
    }

    gorgonRoutine := func(exitFunc AegisGorgonExitFunc,
                          exitFuncArgs interface{},
                          onDebuggerFunc AegisGorgonOnDebuggerFunc,
                          onDebuggerFuncArgs interface{}, done chan bool) {
        var stop bool = false
        for !stop {
            //fmt.Println("gorgonRoutine...")
            if (C.aegis_has_debugger() == 1) {
                go func(done chan bool) {
                        // INFO(Rafael): There is no way to know what user is intending on doing on OnDebugger.
                        //               Anyway, on sane anti-debugging mitigations we need to exit process.
                        //               This function will schedule a gracefully SetGorgon exit before
                        //               actually calling user defined OnDebugger statements. Since we are
                        //               probably exiting here, there is no problem on leaking this go routine,
                        //               but for conscience's sake let's try to exit more gracefully as possible.
                        time.Sleep(10 * time.Nanosecond)
                        done <- true
                    }(done)
                onDebugger(onDebuggerFuncArgs)
            }
            //fmt.Println(stop)
            if exitFunc != nil {
                stop = exitFunc(exitFuncArgs)
            }
            //fmt.Println(stop)
            if !stop {
                timeout := time.Tick(1 * time.Nanosecond)
                select {
                    case stop = <-done:
                    case <-timeout:
                }
            }
            //fmt.Println(stop)
            time.Sleep(1 * time.Nanosecond)
            //fmt.Println("--")
        }
        done <- true
    }

    done :=  make(chan bool, 1)

    go gorgonRoutine(exitFunc, exitFuncArgs, onDebugger, OnDebuggerFuncArgs, done)
    <- done
}

func defaultOnDebugger(args interface{}) {
    os.Exit(1)
}
