# ![Medusa by Caravaggio (1571-1610) / Public Domain](https://github.com/rafael-santiago/aegis/blob/master/etc/caravaggio_medusa.png "Medusa by Caravaggio (1571-1610) / Public Domain")
Aegis

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

## Using ``Aegis``

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
- To pass ``-lpthread`` flag if you are on ``Linux``.

All in one compilation line:

```
black-beard@QueensAnneRevenge:~/src/aegis/src# cd samples
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# gcc -I.. -L../../lib \
> wait4debug.c -owait4debug -laegis
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

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

### Debugging mitigation

Certain programs require some debugging avoidance. ``Aegis`` features a nice and straightforward way to implement this kind
of mitigation. For doing that you need:

- To implement a exit checking function with the prototype: ``int(void *)``.
- To call ``aegis_set_gorgon()`` passing your exit checking function and its argument pointer.

Take a look at the following code to get more details about:

```c
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

int main(int argc, char **argv) {
    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);
    if (aegis_set_gorgon(disable_gorgon, &bye) != 0) {
        fprintf(stderr, "error: unable to set gorgon.\n");
        exit(1);
    }

    fprintf(stdout, "info: the proces id is %d.\n", getpid());
    fprintf(stdout, "info: press ctrl + c to exit sample or attach a debugger.\n");
    while (!bye) {
        usleep(2);
    }

    fprintf(stdout, "\ninfo: gracefully exiting, no debugger was detected.\n");

    return 0;
}
```

You can find the presented code into ``src/samples/setgorgon.c``.

#### Testing ``setgorgon``

On a terminal run ``setgorgon``

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: the process id is 28582
info: press ctrl + c to exit sample or attach a debugger.
```

Now let's only press ``ctrl + c``:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: the process id is 28582.
info: press ctrl + c to exit sample or attach a debugger.
^C
info: gracefully exiting, no debugger was detected.
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

Nice but what about give debugging a try? Let's run it again:

```
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# ./setgorgon
info: the process id is 14753.
info: press ctrl + c to exit sample or attach a debugger.
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
info: the process id is 14753.
info: press ctrl + c to exit sample or attach a debugger.
black-beard@QueensAnneRevenge:~/src/aegis/src/samples# _
```

No gracefully exiting message, it was really aborted to avoid debugging. Our gorgon has done her job.

Maybe you are asking why call this feature of ``Gorgon``. Well, ``Perseus`` myth tells that ``Athena`` gave him a shield (aegis)
for help him to kill ``Medusa`` (also known as ``Gorgon``).``Perseus`` has killed her using ``aegis``. By watching for her
reflection in the shield he used the sword (also given by ``Athena``) to chop off ``Medusa``'s head. After that ``Athena`` got
``aegis`` back and in memory of ``Medusa``, ``Athena`` put ``Medusa``'s head in this shield.

Ancient greeks had used to sculpt or even drawn ``Gorgon`` heads at the front door of their houses in order to scare enemies,
bad people, bad things and stuff. Maybe it could be the origin of the ``Medusa``'s myth, who knows...

Well, that is it, here we are using gorgon to scare debuggers! That's all folks!

;)
