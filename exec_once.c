#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EXEC_ONCE_TU_NAME "exec_once"
#include "exec_once.h"
static int exec_once_debug = 0;

exec_once_tu_t * g_exec_once = (void*)0;
int g_exec_once_errors = 0;
void exec_once_register(exec_once_t * x, exec_once_t** glist)
{
    if(EXEC_ONCE_TU_NAME == 0){
        fprintf(stderr,"%s:%d: EXEC_ONCE_TU_NAME is not defined, the translation unit name.\n",
                x->file,x->line);
        g_exec_once_errors ++;
    }
    if(exec_once_debug >=9){
        fprintf(stderr,"%s:%d: register exec once block.\n",
                x->file,x->line);
    }
    if(*glist == 0){
        *glist = x;
    }else{
        exec_once_t * p = *glist;
        while(p->next != 0) p = p->next;
        p->next = x;
    }
}
static inline void exec_once_run_tu(exec_once_tu_t* current);
static inline void exec_once_run(exec_once_t * p)
{
    while(p){
        if(exec_once_debug >8){
            fprintf(stderr, "%s:%d:[%s] start running:\n",
                    p->file,
                    p->line,
                    p->name);

        }
        p->f();
        if(exec_once_debug>8){
            fprintf(stderr, "%s:%d:[%s] end:\n",
                    p->file,
                    p->line,
                    p->name);

        }
        p = p->next;
    }
}
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
            if(exec_once_debug){
                fprintf(stderr,__FILE__ ":%d:[%s] %s depends on %s, running %s\n", __LINE__, __FUNCTION__,
                        current->name,p->name,p->name);
            }
            exec_once_run_tu(p);
        }
        p = p->next;
    }
    return;
}
static inline void exec_once_run_tu(exec_once_tu_t* current)
{
    if(current->done){
        if(0 && exec_once_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] `%s' was already done, cancelled\n", __LINE__, __FUNCTION__,
                    current->name);
        }
        return;
    }
    current->done = 1;
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] checking dependency for `%s'\n", __LINE__, __FUNCTION__,
                current->name);
    }
    exec_once_run_dependency(current);
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] `%s' start running\n", __LINE__, __FUNCTION__,
                current->name);
    }
    exec_once_run(*current->head);
    return;
}
static void check_consistent_of_dependency();
void exec_once_init()
{
    exec_once_tu_t* p = g_exec_once;
    check_consistent_of_dependency();
    if(g_exec_once_errors > 0){
        fprintf(stderr,__FILE__ ":%d:[%s] %d error(s) for exec_once, you might forgot to define translation unit name.\n", __LINE__, __FUNCTION__
                ,g_exec_once_errors);
        exit(1976);
    }
    p = g_exec_once;
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] the scheduled list for exec_once:\n", __LINE__, __FUNCTION__);
        while(p!=0){
            if(!p->done){
                fprintf(stderr,__FILE__ ":%d:[%s]      %s\n", __LINE__, __FUNCTION__,p->name);
            }
            p = p->next;
        }
    }
    p = g_exec_once;
    while(p!=0){
        exec_once_run_tu(p);
        p = p->next;
    }
}
static void check_one(const char * myname, const char * yourname);
static void check_consistent_of_dependency()
{
    exec_once_tu_t * p = g_exec_once;
    for(p = g_exec_once; p ; p = p->next){
        for(int i = 0; p->depend[i] != 0; ++i){
            check_one(p->name,p->depend[i]);
        }
    }
}
static void check_one(const char * myname, const char * yourname)
{
    exec_once_tu_t * p = g_exec_once;
    int ret = 0;
    for(p = g_exec_once; p ; p = p->next){
        if(strcmp(p->name, yourname) == 0){
            ret = 1;
            break;
        }
    }
    if(!ret){
        fprintf(stderr,__FILE__ ":%d:[%s] EXEC_ONCE error: `%s` requires `%s`, but there is no `%s`.\n"
                , __LINE__, __FUNCTION__
                , myname, yourname, yourname
            );
        abort();
    }
}
       
__attribute__((constructor))
static void __exec_once_init_self()
{
    if(getenv("EXEC_ONCE_DEBUG") != NULL){
        exec_once_debug = atoi(getenv("EXEC_ONCE_DEBUG"));
    }
}
