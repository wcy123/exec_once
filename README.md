=======
exec_once
=========

execute initialization block for C transform unit a very simple
wrapper for `__attribute__((constructor))`

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


```c
__attribute__((constructor))
static void foo() 
{
    printf("Hello World from a constructor\n");
}

int main(int argc, char *argv[])
{
    printf("Hello World");
    return 0;
}
```

the output of the above program is

```shell-session
gcc a.c && ./a.out
Hello World from a constructor
Hello World
```

so we can use this feature to execute to register a small function to 


{% highlight c %}
#include <stdio.h>

static void foo() 
{
    printf("Hello World from a constructor\n");
}


typedef struct exec_once_s {
    void (*f)();
    struct exec_once_s * next;
} exec_once_t;
static exec_once_t * exec_once_list = NULL;

__attribute__((constructor))
static void register_foo()
{
    static exec_once_t x = {foo, NULL};
    x.next = exec_once_list;
    exec_once_list = &x;
}

void exec_once_init()
{
    while(exec_once_list){
        exec_once_list->f();
        exec_once_list = exec_once_list->next;
    }
}

int main(int argc, char *argv[])
{
    printf("start initalization.\n");
    exec_once_init();
    printf("initalization is done.\n");
    return 0;
}
{% endhighlight %}

The output of the program:


{% highlight shell-session %}
gcc a.c && ./a.out
start initalization.
Hello World from a constructor
initalization is done.
{% endhighlight %}


In order to have a better API interface, one macro `EXEC_ONCE_PROGN`
makes it easier as below.

{% highlight c %}
#include <stdio.h>
#define EXEC_ONCE_TU_NAME "a"
#include "exec_once.h"
EXEC_ONCE_PROGN {
    printf("Hello World from a constructor 1\n");
}
EXEC_ONCE_PROGN {
    printf("Hello World from a constructor 2\n");
}
int main(int argc, char *argv[])
{
    printf("start initalization.\n");
    exec_once_init();
    printf("initalization is done.\n");
    return 0;
}
{% endhighlight %}

the output of the program is as below

{% highlight shell-session %}
gcc a.c -lexec_once -L.  && LD_LIBRARY_PATH=. ./a.out
start initalization.
Hello World from a constructor 1
Hello World from a constructor 2
initalization is done.
{% endhighlight %}

# how to use it

- define a transform unit name at the begining of a source file,
  i.e. before including `exec_once.h`, e.g.

{% highlight c %}
// in foo.c
#define EXEC_ONCE_TU_NAME "foo"
{% endhighlight %}

- include `exec_once.h`

- start to use macro `EXEC_ONCE_PROGN`, as below
{% highlight c %}
// in foo.c
#define EXEC_ONCE_TU_NAME "foo"
#include <exec_once.h>
EXEC_ONCE_PROGN {
    printf("Hello World From Foo\n");
}
{% endhighlight %}

- `EXEC_ONCE_PROGN` blocks are not executed until `exec_once_init` is
  invoke, so in the `main` function, we invoke it.

{% highlight c %}
// in main.c
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
{% endhighlight %}

{% highlight shell-session %}
bash$ git clone /home/git.repository/exec_once.git/
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
{% endhighlight %}

# dependency

why we have to define `EXEC_ONCE_TU_NAME`? It is because of dependency.
{% highlight shell-session %}
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
{% endhighlight %}

We can see the execution order depends on the linker. If we put
`bar.c` in front of `foo.c`, `exec_once` blocks in `bar.c` executes
before blocks in `foo.c`.

If every transform unit has a name, we can define the dependency
relationship, by using macro `EXEC_ONCE_DEPENDS`, as below





[constructor]: https://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Function-Attributes.html#Function-Attributes "GNU GCC Manual"

 
