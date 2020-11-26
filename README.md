# ![Medusa by Caravaggio (1571-1610) / Public Domain](https://github.com/rafael-santiago/aegis/blob/master/etc/caravaggio-medusa.jpg "Medusa by Caravaggio (1571-1610) / Public Domain") Aegis

``Aegis`` is a library that allows you detect if your software is being debugged or not on ``Linux``.

The name is about a lousy acronym: **A**n **E**LF's -**g** **i**nspection **s**ignalling. If you are hooked on
Greek mithology you should know that ``Aegis`` is the name of the shield gave by ``Athena`` to ``Perseus`` to help him
kill ``Medusa``.

On ``Windows`` we have plenty of ways to easily do this kind of detection. Opposingly, on ``Unix`` world we do not have any
standard way. ``Aegis`` is an attempt of filling up this gap.

## How can I build it?

I am using a build tool of mine called [``Hefesto``](https://github.com/rafael-santiago/hefesto) (Yes, mithology, I love it).

If you are looking for running the build in all its capabilities you need ``Hefesto`` otherwise I also supply a well-simple
Makefile.

## How should I easily clone ``Aegis``?

The easiest way is:

```
black-beard@QueenAnneRevenge:~/src# git clone https://github.com/rafael-santiago/aegis --recursive
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

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

### Poor's man build by using ``make``

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

## Using Aegis

``Aegis`` is a well-simple tiny library:

- It only has one header called ``aegis.h``.

- It only has one library archive called ``aegis.a``.

``Aegis`` brings you two features:

- Detect debugging.

- Protect against debugging by using our simpathetic ``Gorgon`` (yes, I know, Greek mitholoy again).


### Detect debugging

In some bug hunting cases is useful to wait for debug before continuing the program execution. Specially concurrent stuff or
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
- To indicate where ``aegis.a`` is found.
- To pass ``-laegis`` flag to linker.

All in one compilation line:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# cd samples
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# gcc -I.. -L../../lib \
> wait4debug.c -owait4debug -laegis
black-beard@QueensAnneRevenge:~/src/aegis/src# _
```

#### Testing ``wait4debug``

On a terminal run ``wait4debug``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/sample# ./wait4debug
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
black-beard@QueensAnneRevenge:~/src/aegis/src/sample# ./wait4debug
*** Wait for debug attachment (pid=29670)
*** Debugger is attached.
black-beard@QueensAnneRevenge:~/src/aegis/src/sample# _
```
