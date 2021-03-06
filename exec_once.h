#pragma once
/** @file
exec_once
=========

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// public symbols
void exec_once_init();
typedef struct exec_once_block_s {
    void (*f)();
    struct exec_once_block_s * next;
    const char * name;
    const char * file;
    int line;
    int index;
} exec_once_block_t;
typedef struct exec_once_tu_s exec_once_tu_t;
struct exec_once_tu_s {
    // it is used by depend. After dependencies are resolved, depend
    // is an array of pointer which point to exec_once_tu_t * object.
    // flag == "", so that *depend[0] == "", then dependency is
    // resolve.
    char flag[sizeof(char*)];
    const char * name;
    /** the pointer to ::exec_once_list
     */
    exec_once_block_t ** head;
    exec_once_tu_t * next;
    const char ** depend;
    int num_of_dep;
    int done;
    // for Tarjan algorithm
    int index;
    int lowlink;
    exec_once_tu_t * stack_next;
    int in_stack;
};
// if EXEC_ONCE_TU_NAME is not defined, the following functions and
// macros are not available.
#ifdef EXEC_ONCE_TU_NAME
// private functions
struct exec_once_block_s;
extern struct exec_once_tu_s* g_exec_once;
int g_exec_once_errorsr;
void exec_once_register(exec_once_block_t * x, exec_once_block_t** glist);
/** every tranform unit has a list of init code blocks, exec_once_list
 * is the head of the list */
static exec_once_block_t * exec_once_list = (void*)0;
#define EXEC_ONCE_MACRO_STRINGIFY(tag)  #tag
#define EXEC_ONCE_MACRO_CONCAT(tag,counter)  tag ## counter
#define EXEC_ONCE_PROGN_UNIQUE_ID(tag,counter)  EXEC_ONCE_MACRO_CONCAT(tag,counter)
#define EXEC_ONCE_PROGN_WITH_TAG(tag,counter)                           \
    static void EXEC_ONCE_MACRO_CONCAT(tag, _block)(void);              \
    __attribute__((constructor()))                                      \
    static void EXEC_ONCE_MACRO_CONCAT(tag,_register)()                 \
    {                                                                   \
        static exec_once_block_t x =                                    \
            {                                                           \
                EXEC_ONCE_MACRO_CONCAT(tag, _block)                     \
                ,NULL                                                   \
                , EXEC_ONCE_MACRO_STRINGIFY(                            \
                    tag)                                                \
                , __FILE__                                              \
                , __LINE__                                              \
                , counter                                               \
            };                                                          \
        exec_once_register(&x,&exec_once_list);                         \
    }                                                                   \
    static void EXEC_ONCE_MACRO_CONCAT(tag, _block)(void)               \

#define EXEC_ONCE_PROGN EXEC_ONCE_PROGN_WITH_TAG(EXEC_ONCE_PROGN_UNIQUE_ID(exec_once_,__COUNTER__),__COUNTER__)

#ifndef EXEC_ONCE_DEPENDS
#define EXEC_ONCE_DEPENDS {}
#endif
// no one should invoke this function
__attribute__((constructor))
static void dont_invoke_me_exec_once_register_tu()
{
    static const char * depend [] = EXEC_ONCE_DEPENDS;
    static exec_once_tu_t x = { {0x00,0x00,0x00,0x00},EXEC_ONCE_TU_NAME,
                                &exec_once_list,
                                0,
                                depend, sizeof(depend)/sizeof(const char*),
                                0 };
    if(x.name == 0) {
        fprintf(stderr, "EXEC_ONCE_TU_NAME is not defined\n");
        abort();
    }
    x.next  = g_exec_once;
    g_exec_once = &x;
}
#else  // EXEC_ONCE_TU_NAME is not defined
#define EXEC_ONCE_PROGN error you forgot to define EXEC_ONCE_TU_NAME
#endif // if EXEC_ONCE_TU_NAME defined

