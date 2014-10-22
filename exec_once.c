#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec_once.h"
static int enable_debug = 0;
exec_once_tu_t * g_exec_once = (void*)0;
int g_exec_once_errors = 0;
static inline  void exec_once_run_tu(exec_once_tu_t* current);
static inline int exec_once_depend_on(exec_once_tu_t* current, exec_once_tu_t* p)
{
    int ret = 0;
    if(!p->done){
        for(int i = 0; current->depend[i] != 0; ++i){
            if(strcmp(current->depend[i],p->name)  == 0){
                ret = 1; break;
            }
        }
    }
    return ret;
}
static inline void exec_once_run_dependency(exec_once_tu_t* current)
{
    exec_once_tu_t* p = g_exec_once;
    while(p!=0){
        if(exec_once_depend_on(current,p)){
            fprintf(stderr,__FILE__ ":%d:[%s] %s depends on %s, running %s\n", __LINE__, __FUNCTION__,
                    current->name,p->name,p->name);

            exec_once_run_tu(p);
        }
        p = p->next;
    }
    return;
}
static inline void exec_once_run_tu(exec_once_tu_t* current)
{
    if(current->done) return;
    current->done = 1;
    exec_once_run_dependency(current);
    exec_once_run(*current->head);
    return;
}
void exec_once_init()
{
    exec_once_tu_t* p = g_exec_once;
    if(g_exec_once_errors > 0){
        fprintf(stderr,__FILE__ ":%d:[%s] %d error(s) for exec_once, you might forgot to define translation unit name.\n", __LINE__, __FUNCTION__
                ,g_exec_once_errors);
        exit(1976);
    }
    if(getenv("EXEC_ONCE_DEBUG") != NULL){
        enable_debug = 1;
    }
    p = g_exec_once;
    while(p!=0){
        if(enable_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] running %s\n", __LINE__, __FUNCTION__,p->name);
        }
        p = p->next;
    }
    p = g_exec_once;
    while(p!=0){
        exec_once_run_tu(p);
        p = p->next;
    }
}
