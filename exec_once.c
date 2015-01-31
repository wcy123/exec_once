#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec_once.h"
static int exec_once_debug = 0;
static void print_tu(FILE * fp, exec_once_tu_t * tu);
exec_once_tu_t * g_exec_once = (void*)0;
void exec_once_register(exec_once_block_t * x, exec_once_block_t** glist)
{
    if(exec_once_debug >=9){
        fprintf(stderr,"%s:%d: register exec once block.\n",
                x->file,x->line);
    }
    if(*glist == 0){
        *glist = x;
    }else{
        exec_once_block_t * p = *glist;
        while(p->next != 0) p = p->next;
        p->next = x;
    }
}
static inline void exec_once_run_tu(exec_once_tu_t* current,exec_once_tu_t * head);
static inline void exec_once_run(exec_once_block_t * p)
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
static inline void exec_once_run_dependency(exec_once_tu_t* current, exec_once_tu_t * head)
{
    exec_once_tu_t* p = head;
    while(p!=0){
        if(exec_once_depend_on(current,p)){
            if(exec_once_debug){
                fprintf(stderr,__FILE__ ":%d:[%s] %s depends on %s, running %s\n", __LINE__, __FUNCTION__,
                        current->name,p->name,p->name);
            }
            exec_once_run_tu(p,head);
        }
        p = p->next;
    }
    return;
}
static inline void exec_once_run_tu(exec_once_tu_t* current,exec_once_tu_t * head)
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
    exec_once_run_dependency(current,head);
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] `%s' start running\n", __LINE__, __FUNCTION__,
                current->name);
    }
    exec_once_run(*current->head);
    return;
}
static void check_name_confict(exec_once_tu_t * head);
static void check_exec_block(exec_once_tu_t * head);
static void resolve_dependency(exec_once_tu_t * head);
static void check_dependency_exists(exec_once_tu_t * head);
void exec_once_init()
{
    exec_once_tu_t * head = g_exec_once;
    g_exec_once = NULL;
    exec_once_tu_t* p = head;
    check_name_confict(head);
    check_exec_block(head);
    resolve_dependency(head);
    check_dependency_exists(head);
    p = head;
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] the scheduled list for exec_once:\n", __LINE__, __FUNCTION__);
        while(p!=0){
            if(!p->done){
                fprintf(stderr,__FILE__ ":%d:[%s]      %s\n", __LINE__, __FUNCTION__,p->name);
            }
            p = p->next;
        }
    }
    p = head;
    while(p!=0){
        exec_once_run_tu(p,head);
        p = p->next;
    }
}
static void resolve_dependency(exec_once_tu_t * head)
{
    exec_once_tu_t * p = NULL;
    exec_once_tu_t * q = NULL;
    int ok = 1;
    for(p = head; p ; p = p->next){
        for(int i = 0; p->depend[i] != 0; ++i){
            if(p->depend[i][0] == '\0'){
                fprintf(stderr, "EXEC_ONCE: depends on a invalid TU name. TU as below\n");
                print_tu(stderr,p);
                ok = 0;
                continue;
            }
            for(q = head; q ; q = q->next){
                if(strcmp(p->depend[i], q->name) == 0){
                    p->depend[i] = (const char*) q;
                }
            }
        }
    }
    if(!ok) abort();
    return;
}
static void check_dependency_exists(exec_once_tu_t * head)
{
    int ok = 1;
    exec_once_tu_t * p = head;
    for(p = head; p ; p = p->next){
        for(int i = 0; p->depend[i] != 0; ++i){
            if(p->depend[i][0] != '\0'){
                ok = 0;
                fprintf(stderr,"EXEC_ONCE: cannot resolve dependency for the below TU.\n");
                print_tu(stderr,p);
            }
        }
    }
    if(!ok) abort();
    return;
}
static void check_name_confict(exec_once_tu_t * head)
{
    exec_once_tu_t * p = NULL;
    exec_once_tu_t * q = NULL;
    int ok = 1;
    for(p = head; p ; p = p->next){
        for(q = p->next; q ; q = q->next){
            if(strcmp(p->name, q->name) == 0){
                fprintf(stderr,"EXEC_ONCE: name conflict detected.\n");
                print_tu(stderr,p);
                print_tu(stderr,q);
                ok = 0;
            }
        }
    }
    if(!ok) abort();
    return;
}
static void check_exec_block(exec_once_tu_t * head)
{
    exec_once_tu_t * p = NULL;
    int ok = 1;
    for(p = head; p ; p = p->next){
        if(!*p->head){
            fprintf(stderr,"EXEC_ONCE: no EXEC_ONCE_PROGN defined in the transform unit.\n");
            print_tu(stderr,p);
            ok = 0;
        }
    }
    if(!ok) abort();
    return;
}
static void print_tu(FILE * fp, exec_once_tu_t * p)
{
    int c = 0;
    exec_once_block_t * blk = *p->head;
    while(blk) {blk=blk->next; c++;};
    fprintf(fp,
            "TU `%s`: %d blocks registered\n"
            ,p->name,c);
    fprintf(fp,
            "  depend on:");
    for(int i = 0; p->depend[i] != 0; ++i){
        const char * name = p->depend[i][0] == '\0'? ((exec_once_tu_t*)p->depend[i])->name : p->depend[i];
        fprintf(fp," `%s`", name);
    }
    fprintf(fp,"\n");
    for(blk = *p->head;
        blk; blk=blk->next){
        fprintf(fp,
                "    %s:%d: %s\n"
                ,blk->file,blk->line,blk->name);
    }
    return;
}       
__attribute__((constructor))
static void __exec_once_init_self()
{
    if(getenv("EXEC_ONCE_DEBUG") != NULL){
        exec_once_debug = atoi(getenv("EXEC_ONCE_DEBUG"));
    }
}
