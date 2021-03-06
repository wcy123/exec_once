=======
exec_once
=========
[![Build Status](https://travis-ci.org/wcy123/exec_once.svg)](https://travis-ci.org/wcy123/exec_once)

<http://wcy123.github.io/exec_once/> doxygen document

Execute initialization block for C transform unit.

Sometimes, we need to execute C codes per transform unit to initialize
static variables. Compilation time initialization is so limited that
only some constant values are valid, you cannot execute arbitrary C
codes. C++ constructor function for a static variable could solve the
problem, but it has new problems as below

 - it need c++ compiler and c++ source code.
 - there is no dependency between transform units.
 - the codes are executed before `main` function.

In a C transform unit, into to initalized a file local variable, you
have to export a function which to be invoked by C++ part.

We cannot guarantee that one transform unit executes before another
transform unit.

We want to the initalization code is executed in a controlled way,
e.g. after `main` function processed command line options and
initialized some other libraries.

## feasibility

[__attribute__((contructor))][constructor] is a well-known gcc
extension, which causes the function to be called automatically before
execution enters `main`.


<code>
@include tests/feasibility1.c
</code>

the output of the above program is

<code>
@include tests/feasibility1.out
</code>

so we can use this feature to register a small function to be invoked
later on.
<code>
@include tests/feasibility2.c
</code>

The output of the program:

<code>
@include tests/feasibility2.out
</code>


In order to have a better API interface, one macro `EXEC_ONCE_PROGN`
makes it easier as below.

<code>
@include tests/ex_basic.c
</code>

the output of the program is as below

<code>
@include tests/ex_basic.out
</code>

## how to use it

- define a transform unit name at the begining of a source file,
  i.e. before including `exec_once.h`, e.g.

~~~{.c}
// in foo.c
#define EXEC_ONCE_TU_NAME "foo"
~~~

- include `exec_once.h`

~~~{.c}
// in foo.c
#include <exec_once.h>
~~~

- start to use macro `EXEC_ONCE_PROGN`, as below
~~~{.c}
// in foo.c
EXEC_ONCE_PROGN {
    printf("Hello World From Foo\n");
}
~~~

- `EXEC_ONCE_PROGN` blocks are not executed until `exec_once_init` is
  invoke, so in the `main` function, we invoke it.

~~~{.c}
// in main.c
int main(int argc, char *argv[])
{
    printf("start to init exec_once\n");
    exec_once_init();
    printf("after init exec_once\n");
    return 0;
}
~~~

so put it together

~~~shell-session
bash$ git clone https://github.com/wcy123/exec_once.git
Cloning into 'exec_once'...
done.
bash$ cd exec_once/
bash$ make
bash$ cat foo.c
// in foo.c
#define EXEC_ONCE_TU_NAME "foo"
#include <exec_once.h>
EXEC_ONCE_PROGN {
    printf("Hello World From Foo\n");
}

bash$ cat main.c
#include <stdio.h>
#include <stdlib.h>
#include <exec_once.h>
int main(int argc, char *argv[])
{
    printf("start to init exec_once\n");
    exec_once_init();
    printf("after init exec_once\n");
    return 0;
}

bash$ gcc -I . foo.c main.c -L. -lexec_once && LD_LIBRARY_PATH=. ./a.out
start to init exec_once
Hello World From Foo
after init exec_once
~~~

## dependency

why we have to define `EXEC_ONCE_TU_NAME`? It is because of dependency.
~~~shell-session
bash$ cat bar.c
// in bar.c
#define EXEC_ONCE_TU_NAME "bar"
#include <exec_once.h>
EXEC_ONCE_PROGN {
    printf("Hello World From Bar\n");
}
bash$ gcc -I . foo.c bar.c main.c -L. -lexec_once && LD_LIBRARY_PATH=. ./a.out
start to init exec_once
Hello World From Foo
Hello World From Bar
after init exec_once
bash$ gcc -I . bar.c foo.c main.c -L. -lexec_once && LD_LIBRARY_PATH=. ./a.out
start to init exec_once
Hello World From Bar
Hello World From Foo
after init exec_once
~~~

We can see the execution order depends on the linker. If we put
`bar.c` in front of `foo.c`, `exec_once` blocks in `bar.c` executes
before blocks in `foo.c`.

If every transform unit has a name, we can define the dependency
relationship, by using macro `EXEC_ONCE_DEPENDS`, as below
~~~{.c}
// ...
#define EXEC_ONCE_TU_NAME "bar"
#define EXEC_ONCE_DEPENDS {"foo", NULL}
#include <exec_once.h>
// ...
~~~

then we run these commands again,

~~~shell-session
 % cat bar.c
// in bar.c
#define EXEC_ONCE_TU_NAME "bar"
#define EXEC_ONCE_DEPENDS {"foo", NULL}
#include <exec_once.h>
EXEC_ONCE_PROGN {
    printf("Hello World From Bar\n");
}
 % gcc -I . bar.c foo.c main.c -L. -lexec_once && LD_LIBRARY_PATH=. ./a.out
start to init exec_once
Hello World From Foo
Hello World From Bar
after init exec_once
chunywan@debian ~/workspace/exec_once
 % gcc -I . foo.c bar.c main.c -L. -lexec_once && LD_LIBRARY_PATH=. ./a.out
start to init exec_once
Hello World From Foo
Hello World From Bar
after init exec_once
~~~
we can see that `foo` always execute before `bar`.

### important note

- `EXEC_ONCE_TU_NAME` must be unique, otherwise it is a fatal error.
- `EXEC_ONCE_DEPENDS` must refer to an existing transform unit,
  otherwise, it is a fatal error.
- If `EXEC_ONCE_TU_NAME` is defined, the transform unit must have at
  least one `EXEC_ONCE_PROGN` block
- If cycle dependency is detected, it is a fatal error.

## A use case

For example, we write a program similiar to `busybox`, which execute
command according to `args[0]`.

~~~{.c}
// in busybox.h
#pragma once
struct cmd {
    const char * name;
    int (*cmd)(int argc, char *argv[]);
    struct cmd * next;
};
extern struct cmd * list;
~~~

We define a list of struct `cmd`, we can search the corresponding
command by `name`.

~~~{.c}
// in busybox.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busybox.h"
#include "exec_once.h"
struct cmd * list = NULL;
int main(int argc, char *argv[])
{
    exec_once_init();
    struct cmd * p = list;
    while(p){
        if(strcmp(p->name, argv[0]) ==0){
            return p->cmd(argc,argv);
        }
    }
    return 0;
}
~~~

That's all for the host application, we need to develop our first
plugin, `ls`.

~~~{.c}
// in ls.c
#define EXEC_ONCE_TU_NAME "ls"
#include "exec_once.h"
#include "busybox.h"

static int ls(int argc, char *argv[])
{
    fprintf(stderr,__FILE__ ":%d:[%s] this is ls, haha\n", __LINE__, __FUNCTION__);
    return 0;
}
EXEC_ONCE_PROGN {
    static struct cmd x = { "ls", ls, NULL };
    x.next = list;
    list = &x;
}
~~~

Test it!

~~~shell-session
gcc -ggdb -O0 -std=c99 -I. ls.c busybox.c exec_once.c -o busybox
ln -s ./busybox ls
PATH=.:$PATH
ls
ls.c:7:[ls] this is ls, haha
~~~

The main adventage of the structure is the dependency management, the
host application `busybox.c` doesn't depend on any plugin,
e.g. `ls.c`.


[constructor]: https://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Function-Attributes.html#Function-Attributes "GNU GCC Manual"

 
