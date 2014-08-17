#pragma once

/*

A transform unit contains an independt execute list, which is belong
to the transform unit. A transform unit can have exact one execute
list.


INTERFACE MACROS:
   
  1. EXEC_ONCE(f):
  
  `f` is a `void (void)` function. `f` is pushed into the execute
  list.

  2. EXEC_DO( any c statements):

  create an anonymouse function contains the statements and push it
  into the execute list.

  3. EXEC_ONCE_START

  Create an constructor function which executes the execute list when
  loading the transform unit, before invoking the main
  entry,e.g. `main`
  
  If there is no such constructor function, you should invoke
  `exec_once_fun()` manually.

 */

typedef struct exec_once_s {
    void (*f)();
    struct exec_once_s * next;
} exec_once_t;

static exec_once_t * exec_once_list = NULL;
static inline
void exec_once_register(exec_once_t * x)
{
    if(exec_once_list == NULL){
        exec_once_list = x;
    }else{
        exec_once_t * p = exec_once_list;
        while(p->next != NULL) p = p->next;
        p->next = x;
    }
}
static inline
void exec_once_run()
{
    exec_once_t * p = exec_once_list;
    while(p){
        p->f();
        p = p->next;
    }
    exec_once_list = NULL;
}
#define EXEC_ONCE_HELPER(x,y) x ## y
#define EXEC_ONCE(f)                            \
__attribute__((constructor))                    \
static void EXEC_ONCE_HELPER(f,_exec_once)()    \
{                                               \
    static exec_once_t x = {f,NULL};            \
    exec_once_register(&x);                     \
}

#define EXEC_ONCE_DO_ID_HELPER(v1) do__begin__ ##v1## __do_end
#define EXEC_ONCE_DO_ID(line) EXEC_ONCE_DO_ID_HELPER(line)
#define EXEC_ONCE_DO(expr)                      \
    static void EXEC_ONCE_DO_ID(__LINE__)(void) \
    {                                           \
        expr                                    \
    }                                           \
    EXEC_ONCE(EXEC_ONCE_DO_ID(__LINE__))



#define EXEC_ONCE_START                         \
__attribute__((constructor))                    \
static void exec_once_main()                    \
{                                               \
    exec_once_run();                            \
}
