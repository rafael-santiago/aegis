![Medusa by Caravaggio (1571-1610) / Public Domain](https://github.com/rafael-santiago/aegis/blob/master/etc/aegis_logo.png "Medusa by Caravaggio (1571-1610) / Public Domain")
#

``Aegis`` is a library that allows you detect if your software is being debugged or not on ``Linux``, ``FreeBSD``, ``NetBSD``, ``OpenBSD`` and
``Windows``. You can use it natively from ``C`` or use the ``Go`` bind.

The name is about a lousy acronym: **A**n **E**LF's -**g** **i**nspection **s**ignalling. If you are hooked on
Greek mithology you should know that ``Aegis`` is the name of the shield gave by ``Athena`` to ``Perseus`` to help him
kill ``Medusa``. If you are using it from ``Windows`` understand as **A**n **E**xecutable's -**g** **i**nspection
**s**ignalling ;)

On ``Windows`` we have plenty of ways to easily do this kind of detection. Opposingly, on ``Unix`` world we do not have any
standard way. ``Aegis`` is an attempt of filling up this gap.

You can use ``Aegis`` as an ``anti-debugging mitigation`` or as a ``debugging facility``. It just depends on you and your
current requirements.

Originally, I wrote this library to use on another tool of mine called [``blackcat``](https://github.com/rafael-santiago/blackcat)
as an ``anti-debugging`` stuff.

# Contents

- [How can I build it?](#how-can-i-build-it)
    - [How should I easily clone ``Aegis``?](#how-should-i-easily-clone-aegis)
    - [Build it by using ``Hefesto``](#building-it-by-using-hefesto)
    - [Poor man's build by using make](#poor-mans-build-by-using-make)
    - [Making a distribution package](#making-a-distribution-package)
    - [How should I build ``Aegis`` ``Go`` stuff?](#how-should-i-build-aegis-go-stuff)
- [Using Aegis](#using-aegis)
    - [Debugging detection](#debugging-detection)
        - [Testing ``wait4debug``](#testing-wait4debug)
    - [Debugging mitigation](#debugging-mitigation)
        - [Testing ``setgorgon``](#testing-setgorgon)
    - [``Aegis`` from ``Go``](#aegis-from-go)
        - [``wait4debug`` on ``Go``](#wait4debug-on-go)
        - [What about a ``Gopher Gorgon``?](#what-about-a-gopher-gorgon)
- [Contributors](#contributors)

## How can I build it?

I am using a build tool of mine called [``Hefesto``](https://github.com/rafael-santiago/hefesto) (Yes, mithology, I love it).

If you are looking for running the build in all its capabilities you need ``Hefesto`` otherwise I also supply a well-simple
Makefile.

[``Back``](#contents)

### How should I easily clone ``Aegis``?

The easiest way is:

```
black-beard@QueensAnneRevenge:~/src# git clone https://github.com/rafael-santiago/aegis --recursive
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

[``Back``](#contents)

### Building it by using ``Hefesto``

After following all steps to put ``Hefesto`` to work on your system, just change to ``src`` sub-directory:

```
black-beard@QueensAnneRevenge:~/src/aegis# cd src
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

The hardest part: invoke ``Hefesto``. Look:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# hefesto
(...)
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

If all has occurred fine during your build, ``aegis`` library was built at ``../lib`` sub-directory. Additionaly,
test has ran and all samples was built at ``../samples`` sub-directory.

[``Back``](#contents)

### Poor man's build by using ``make``

Well this will just build the library at ``../lib``. The clumsy idea here is: If all has compiled so all is working...

Change to ``src`` sub-directory:

```
black-beard@QueensAnneRevenge:~/src/aegis# cd src
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

Now it is just about calling ``make``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# make
(...)
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

If you are on some ``BSD-like``, besides ``make`` you also need ``gmake`` to run this limited alternative build.

On ``*BSD`` you can also invoke the ``poor man's build`` by running ``gmake``.

[``Back``](#contents)

### Making a distribution package

You can easily do it by invoking ``Hefesto`` passing the build option ``--mkdist``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# hefesto --mkdist
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

Once done the distribution package will be at ``../lib`` sub-directory. The package's name depends on your system.
It will follow this nomenclature scheme: ``libaegis-<os-name>.zip``.

[``Back``](#contents)

### How should I build ``Aegis`` ``Go`` stuff?

``Go`` is a language with automagically build capabilities. Once inside package sub-directory (``gopkg/vN``) call
``go build -a`` or ``go test`` will do the job. For samples, once inside a sample directory run ``go build -a -o sample-name``.
It would be the poor man's build for ``Go``.

If you are in a rush and looking for a more automated way of doing it. Inside ``src`` top-level sub-directory invoke ``Hefesto``
passing ``--gopkg`` build option:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# hefesto --gopkg
(...)
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

After a successful build you will got the ``Go`` samples inside ``../samples`` with their names prepended with ``golang-``.

[``Back``](#contents)

## Using ``Aegis``

``Aegis`` is a well-simple tiny library:

- It only has one header called ``aegis.h``.

- It only has one library archive called ``libaegis.a``.

``Aegis`` brings you two features:

- Detect debugging.

- Protect against debugging by using our simpathetic ``Gorgon`` (yes, I know, Greek mitholoy again).

**Remark**: In order to make this anti-debug resilient against library hooking you always should link your software ``static``.
Moreover, if you want to keep eavesdroppers out as much as possible: link your software static. Otherwise there is no necessity
of worrying about none of it. Because your front door is wide open or you do not have even a door! Haha!

[``Back``](#contents)

### Debugging detection

In some bug hunting cases is useful to wait for debugger before continuing the program execution. Specially concurrent stuff or
even event oriented processing. In this case you can use ``aegis_has_debugger()`` function.

This function returns 1 when a debugger has being attached otherwise 0.

The following program will wait for debugger before exiting:

```c
#include <aegis.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sigint_watchdog(int signo) {
    printf("\nCanceled.\n");
    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);
    printf("*** Waiting for debug attachment (pid=%d)...\n", getpid());
    while (!aegis_has_debugger()) {
        usleep(1);
    }
    printf("*** Debugger is attached.\n");
    return 0;
}
```

The program above can be found at ``src/samples`` sub-directory under the name ``wait4debug.c``.
The manual compilation of this code is fairly simple and involves:

- To indicate where ``aegis.h`` is found.
- To indicate where ``libaegis.a`` is found.
- To pass ``-laegis`` flag to linker.
- To pass ``-lpthread`` flag if you are on ``Linux``, ``FreeBSD``, ``NetBSD`` or ``OpenBSD``.

All in one compilation line:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# cd samples
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# gcc -I.. -L../../lib \
> wait4debug.c -owait4debug -laegis
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

[``Back``](#contents)

#### Testing ``wait4debug``

On a terminal run ``wait4debug``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./wait4debug
*** Wait for debug attachment (pid=29670)
```

``wait4debug`` has facilitate the things for you by giving its ``pid``. Now on another terminal run ``GDB`` as follows:

```
black-beard@QueensAnneRevenge:~# gdb attach 29670
(...)
(gdb)
```

You will attach to the ``wait4debug`` process, now let's continue on ``GDB``:

```
black-beard@QueensAnneRevenge:~# gdb attach 29670
(...)
(gdb) continue
Continuing.
[Inferior 1 (process 29670) exited normally]
(gdb)
```

Nice, the program has exited. If you back to your ``wait4debug`` terminal you will see something like:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./wait4debug
*** Wait for debug attachment (pid=29670)
*** Debugger is attached.
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

[``Back``](#contents)

### Debugging mitigation

Certain programs require some debugging avoidance. ``Aegis`` features a nice and straightforward way to implement this kind
of mitigation. For doing that you need:

- To implement a exit checking function with the prototype: ``int(void *)``. A return different from zero means that gorgon should exit.
- If necessary to implement a on debugger function with the prototype: ``void(void *)``. This function will be called when a debugger is detected.
- To call ``aegis_set_gorgon()`` passing your exit checking function and its argument pointer, besides on debugger function and its argument pointer.

Take a look at the following code to get more details about:

```c
/*
 * Copyright (c) 2020, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <aegis.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int bye = 0;

void sigint_watchdog(int signo) {
    bye = 1;
}

int disable_gorgon(void *args) {
    return (*(int *)args);
}

void on_debugger(void *args) {
    fprintf(stdout, "\ninfo: debugger detected.\n");
    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);
    if (aegis_set_gorgon(disable_gorgon, &bye, on_debugger, NULL) != 0) {
        fprintf(stderr, "error: unable to set gorgon.\n");
        exit(1);
    }

    fprintf(stdout, "info: process started (pid=%d)...\n", getpid());
    while (!bye) {
        usleep(2);
    }

    fprintf(stdout, "\ninfo: gracefully exiting, no debugger was detected.\n");

    return 0;
}
```

You can find the presented code into ``src/samples/setgorgon.c``.

When on debugger function was passed as ``NULL``. It asks ``aegis`` to use its default on debugger function. This
default callback is only about calling ``exit(1)``. Thus, after a debugger detection, the process will immediately exit.
There are some cases that you need to do something before exiting, for those cases on debugger function would be handy.
Anyway, once a debugger attached, the best action is terminate the process as soon as possible or try to kill the debugger.

[``Back``](#contents)

#### Testing ``setgorgon``

On a terminal run ``setgorgon``

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: process started (pid=28582)...
```

Now let's only press ``ctrl + c``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: process started (pid=28582)...
^C
info: gracefully exiting, no debugger was detected.
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

Nice but what about give debugging a try? Let's run it again:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: process started (pid=14753)...
```

On another terminal let's attach ``GDB``:

```
black-beard@QueensAnneRevenge:~# gdb attach 14753
(...)
(gdb) _
```

Now, still on ``GDB`` continue the ``setgorgon`` process:

```
black-beard@QueensAnneRevenge:~# gdb attach 14753
(...)
(gdb) continue
Continuing.
[Thread 0x7f47880ec700 (LWP 14754) exited]
[Inferior 1 (process 14753) exited with code 01]
(gdb) _
```

No chance for our debugging attempt, let's go back to our setgorgon's terminal:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: process started (pid=14753)...

info: debugger detected.
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

No gracefully exiting message, it was really aborted to avoid debugging. Our gorgon has done her job.

Maybe you are asking why call this feature of ``Gorgon``. Well, ``Perseus`` myth tells that ``Athena`` gave him a shield (aegis)
for help him to kill ``Medusa`` (also known as ``Gorgon``).``Perseus`` has killed her using ``aegis``. By watching for her
reflection in the shield he used the sword (also given by ``Athena``) to chop off ``Medusa``'s head. After that ``Athena`` has
picked ``aegis`` back and in memory of ``Medusa``, ``Athena`` put ``Medusa``'s head in this shield.

Ancient greeks had used to sculpt or even drawn ``Gorgon`` heads at the front door of their houses in order to scare enemies,
bad people, bad things and stuff. Maybe it could be the origin of the ``Medusa``'s myth, who knows...

Well, that is it, here we are using gorgon to scare debuggers! That's all folks!

;)

[``Back``](#contents)

### ``Aegis`` from ``Go``

I have decided to make an ``Aegis``' ``Go`` bind because I am seeing much applications related to information security
being writen mainly on ``Go``, showing up during these years (2020). Who knows this bind can be useful for somebody
somewhere over a concurrent multiplatform goroutine rainbow... ``Go`` is also my second best programming language so
I have done it for fun, too. It was a good excuse for using ``Cgo``.

Basically you need to import aegis pacakge from this repo:

```go
import (
    "github.com/rafael-santiago/aegis/gopkg"
)
```

After you will define in your ``go.mod`` the following:

```
(...)
replace github.com/rafael-santiago/aegis/gopkg => github.com/rafael-santiago/aegis/gopkg/v1
(...)
```

You can also host it as a local package (no problem). Take a look how it can be done by taking a look at ``go.mod`` from
``Go`` samples (``gopkg/samples``).

This replace trick you allow you use ``Aegis``' future releases without needing be noisy into your own related code.
Renaming packages and all those ``MacGyver-like`` incantations.

The usage of ``Aegis`` on ``Go`` is almost the same of its usage in ``C``. Follow on reading if you are interested on it.

[``Back``](#contents)

#### ``wait4debug`` on ``Go``

I am taking into consideration that you have already follow my notes about ``wait4debug`` C sample. Doing it
on ``Go`` is quite straightforward, to. It is only about testing the attachment state by calling ``aegis.HasDebugger()``
oracle function, look:

```go
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
    fmt.Fprintf(os.Stdout, "\rinfo: Debug detected. Go home!\n")
}
```

The program will wait for a user's ``Ctrl + c`` interruption or for a debugger attachment. It is always important to
sleep for some time interval, otherwise you will busy the main thread and cause starvation on other threads.

[``Back``](#contents)

#### What about a ``Gopher Gorgon``?

Well, if you have some artistic inclination and want to make a ``Medusa-like gopher`` to put here I would be thankful haha!

Anyway, use ``Aegis Gorgon`` in ``Go`` is pretty straightforward, too. You just call ``aegis.SetGorgon()``. Take a look:

```go
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
```

The program will run until detecting a debugger attachment or being asked for gracefully exiting through a ``ctrl + C``.

[``Back``](#contents)

## Contributors

The following table lists all contributors for this project until now.

|            **GitHub profile**  |       **Who**          |        **Contact**          |                            **Done stuff**                               |
|:------------------------------:|:----------------------:|:---------------------------:|:-----------------------------------------------------------------------:|
|[<img src="https://github.com/rafael-santiago.png" width=60 height=60></a>](https://github.com/rafael-santiago){:target="_blank"}|``Rafael Santiago``|``/dev/null``|Initial idea, ``C library``, initial ``cgo-bind``, current maintainer.|

[``Back``](#contents)
