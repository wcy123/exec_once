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

  this macro must be at the end of a transform unit.

*/
#ifndef EXEC_ONCE_TU_NAME
#define EXEC_ONCE_TU_NAME 0
#endif


typedef struct exec_once_s {
    void (*f)();
    struct exec_once_s * next;
} exec_once_t;

typedef struct exec_once_tu_s exec_once_tu_t;
struct exec_once_tu_s {
    const char * name;
    exec_once_t ** head;
    exec_once_tu_t * next;
    const char ** depend;
    int done;
};
extern exec_once_tu_t* g_exec_once;
void exec_once_init();
int g_exec_once_errors;
static exec_once_t * exec_once_list = (void*)0;
static inline
void exec_once_register(exec_once_t * x, const char * file, int line)
{
    if(EXEC_ONCE_TU_NAME == 0){
        fprintf(stderr,"%s:%d: EXEC_ONCE_TU_NAME is not defined, the translation unit name.\n", file,line);
        g_exec_once_errors ++;
    }
    if(exec_once_list == 0){
        exec_once_list = x;
    }else{
        exec_once_t * p = exec_once_list;
        while(p->next != 0) p = p->next;
        p->next = x;
    }
}
static inline
void exec_once_run(exec_once_t * p)
{
    while(p){
        p->f();
        p = p->next;
    }
}
#define EXEC_ONCE_HELPER(x,y) x ## y
#define EXEC_ONCE(f)                                    \
    __attribute__((constructor))                        \
    static void EXEC_ONCE_HELPER(f,_exec_once)()        \
    {                                                   \
        static exec_once_t x = {f,NULL};                \
        exec_once_register(&x,__FILE__,__LINE__);       \
    }

#define EXEC_WITH_COUNTER(m) m(EXEC_WITH_COUNTER_EVAL(__COUNTER__))
#define EXEC_WITH_COUNTER_EVAL(counter) counter

#define EXEC_ONCE_PROGN_ID1(tag) do__begin__ ##tag## __do_end
#define EXEC_ONCE_PROGN_ID(tag) EXEC_ONCE_PROGN_ID1(tag)
#define EXEC_ONCE_PROGN_UNIQUE_ID1(tag,counter)  tag ## counter
#define EXEC_ONCE_PROGN_UNIQUE_ID(tag,counter)  EXEC_ONCE_PROGN_UNIQUE_ID1(tag,counter)
#define EXEC_ONCE_PROGN_WITH_TAG(counter)                               \
    static void EXEC_ONCE_PROGN_ID(tag)(void);                          \
                                       EXEC_ONCE(EXEC_ONCE_PROGN_ID(tag)) \
                                       static void EXEC_ONCE_PROGN_ID(tag)(void) 
#define EXEC_ONCE_PROGN EXEC_ONCE_PROGN_WITH_TAG(EXEC_ONCE_PROGN_UNIQUE_ID(s,__COUNTER__))


#define EXEC_ONCE_START
    

#ifndef EXEC_ONCE_DEPENDS
#define EXEC_ONCE_DEPENDS {0,}
#endif
// no one should invoke this function
__attribute__((constructor))
static void exec_once_register_tu()
{
    static const char * depend [] = EXEC_ONCE_DEPENDS;
    static exec_once_tu_t x = { EXEC_ONCE_TU_NAME,
                                &exec_once_list,
                                0, depend, 0 };
    if(x.name == 0) return;
    if(g_exec_once == 0){
        g_exec_once = &x;
    }else{
        exec_once_tu_t * p = g_exec_once;
        while(p->next != 0) p = p->next;
        p->next = &x;
    }
}
