// package aegis gathers all constants, types and functions related to libaegis cgo bind.
// --
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
#cgo LDFLAGS: -lfltlib
# include <native/windows/aegis_native.c>
#endif
*/
import "C"
import (
	"os"
	"time"
)

// AegisGorgonExitFunc defines the type of exit oracle function called by Aegis' anti-debugging gorgon.
type AegisGorgonExitFunc func(args interface{}) bool

// AegisGorgonOnDebuggerFunc defines the type of OnDebugger functions that will be triggered by Aegis' during a debugging
// attempting.
type AegisGorgonOnDebuggerFunc func(args interface{})

// HasDebugger is a Go wrapper for aegis_has_debugger() from libaegis. HasDebugger returns true is a debugger is
// detected otherwise (guess what?) false.
func HasDebugger() bool {
	return (C.aegis_has_debugger() == 1)
}

// SetGorgon is a Go native implementation of aegis_set_gorgon(). This function installs a goroutine responsible for watching
// out a debugging attempt. The argument exitFunc is a function that verifies if it is time to gracefully exiting. Its
// arguments is the 'generic' argument exitFuncArgs. The argument onDebuggerFunc is a function that takes some action when a
// debugger is detected. Its arguments is the 'generic' argument onDebuggerFuncArgs. When onDebuggerFunc is nil Aegis will
// use its internal default onDebuggerFunc (defaultOnDebugger).
func SetGorgon(exitFunc AegisGorgonExitFunc, exitFuncArgs interface{},
	onDebuggerFunc AegisGorgonOnDebuggerFunc, onDebuggerFuncArgs interface{}) {
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
			if C.aegis_has_debugger() == 1 {
				// INFO(Rafael): There is no way to know what user is intending on doing on OnDebugger.
				//               Anyway, on sane anti-debugging mitigations we need to exit process.
				//               Since we are probably exiting here (in a panic situation), there is no
				//               problem on leaking this go routine, but for conscience's sake let's try
				//               to exit more gracefully as possible.
				defer onDebugger(onDebuggerFuncArgs)
				stop = true
			}
			if !stop && exitFunc != nil {
				stop = exitFunc(exitFuncArgs)
			}
			time.Sleep(1 * time.Nanosecond)
		}
		done <- true
	}

	done := make(chan bool, 1)

	go gorgonRoutine(exitFunc, exitFuncArgs, onDebugger, onDebuggerFuncArgs, done)
	<-done
}

// defaultOnDebugger is the internal Aegis onDebuggerFunc. It is rather gross, being only about an os.Exit(1) and period.
func defaultOnDebugger(args interface{}) {
	os.Exit(1)
}
