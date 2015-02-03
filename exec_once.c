#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec_once.h"
static int exec_once_debug = 0;
static void print_tu(FILE * fp, exec_once_tu_t * tu);
exec_once_tu_t * g_exec_once = (void*)0;
static void check_name_confict(exec_once_tu_t * head);
static void check_exec_block(exec_once_tu_t * head);
static void resolve_dependency(exec_once_tu_t * head);
static void check_dependency_exists(exec_once_tu_t * head);
static void check_cycly_dependency(exec_once_tu_t * head);
static void execute_tu(exec_once_tu_t* current);
static int is_over(exec_once_tu_t * head);
void exec_once_init()
{
    exec_once_tu_t * head = g_exec_once;
    exec_once_tu_t* p = head;
    check_name_confict(head);
    check_exec_block(head);
    resolve_dependency(head);
    check_dependency_exists(head);
    check_cycly_dependency(head);
    if(exec_once_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] the scheduled list for exec_once:\n", __LINE__, __FUNCTION__);
        for(p=head;p;p=p->next){
            if(!p->done){
                print_tu(stderr,p);
            }
        }
    }
    while(!is_over(head)){
        for(p=head;p;p=p->next){
            execute_tu(p);
        }
    }
}
static void resolve_dependency(exec_once_tu_t * head)
{
    exec_once_tu_t * p = NULL;
    exec_once_tu_t * q = NULL;
    for(p = head; p ; p = p->next){
        for(int i = 0; i < p->num_of_dep ; ++i){
            if(p->depend[i][0] == '\0'){
                // dependency is already resolved.
            }else{
                for(q = head; q ; q = q->next){
                    if(strcmp(p->depend[i], q->name) == 0){
                        p->depend[i] = (const char*) q;
                    }
                }
            }
        }
    }
    return;
}
static void check_dependency_exists(exec_once_tu_t * head)
{
    int ok = 1;
    exec_once_tu_t * p = head;
    for(p = head; p ; p = p->next){
        for(int i = 0; i < p->num_of_dep; ++i){
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
            "TU `%s` depends on:" ,p->name);
    for(int i = 0; i < p->num_of_dep; ++i){
        const char * name = p->depend[i][0] == '\0'? ((exec_once_tu_t*)p->depend[i])->name : p->depend[i];
        fprintf(fp," `%s`", name);
    }
    fprintf(fp,"\n");
    fprintf(fp,"   %d blocks registered\n",c);
    for(blk = *p->head;
        blk; blk=blk->next){
        fprintf(fp,
                "    %s:%d: %s\n"
                ,blk->file,blk->line,blk->name);
    }
    return;
}

static void strongconnect(int * index, exec_once_tu_t **S, exec_once_tu_t * v, int * ok);
static void check_cycly_dependency(exec_once_tu_t * head)
{
    //  Tarjan's strongly connected components algorithm
    //  http://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
    exec_once_tu_t * p = NULL;
    // initialization
    for(p = head; p ; p = p->next){
        p->index = -1;
        p->lowlink = -1;
        p->stack_next = NULL;
        p->in_stack = 0;
    }
    int ok = 1;
    // index := 0
    int index = 0;
    //  S := empty
    exec_once_tu_t * S = NULL;
    /* 
       for each v in V do
         if (v.index is undefined) then
            strongconnect(v)
         end if
      end for
    */
    for(p = head; p ; p = p->next){
        if(p->index == -1){
            strongconnect(&index,&S,p,&ok);
        }
    }
    if(!ok) abort();
}
static void strongconnect(int * index, exec_once_tu_t **S, exec_once_tu_t * v, int * ok)
{
    // Set the depth index for v to the smallest unused index
    v->index = *index;
    v->lowlink = *index;
    *index = *index + 1;
    // S.push(v)
    v->stack_next = *S;
    v->in_stack = 1;
    *S = v;
/*  // Consider successors of v
    for each (v, w) in E do
      if (w.index is undefined) then
        // Successor w has not yet been visited; recurse on it
        strongconnect(w)
        v.lowlink  := min(v.lowlink, w.lowlink)
      else if (w is in S) then
        // Successor w is in stack S and hence in the current SCC
        v.lowlink  := min(v.lowlink, w.index)
      end if
    end for
*/
    for(int i = 0; v->depend[i] != 0; ++i){
        exec_once_tu_t* w = (exec_once_tu_t*)v->depend[i];
        if(w->index == -1){
            strongconnect(index, S, w, ok);
            v->lowlink = v->lowlink < w->lowlink? v->lowlink: w->lowlink;
        }else if( w->in_stack ){
            v->lowlink = v->lowlink < w->index? v->lowlink: w->index;
        }
    }
/*
    // If v is a root node, pop the stack and generate an SCC
    if (v.lowlink = v.index) then
      start a new strongly connected component
      repeat
        w := S.pop()
        add w to current strongly connected component
      until (w = v)
      output the current strongly connected component
    end if
*/
    if(v->lowlink == v->index){
        if(*S == v){
            // OK, single element CSS
        }else{
            *ok = 0;
            fprintf(stderr,"EXEC_ONCE: cycle dependency detected for the following TU.\n");
            exec_once_tu_t * q = *S;
            do{
                print_tu(stderr, q);
                q = q->stack_next;
            }while(q != v);
             print_tu(stderr, q);
        }
        exec_once_tu_t* w = NULL;
        do{
            // w:= S.pop()
            w = *S;
            w->in_stack = 0;
            *S = w->stack_next;
        }while(w!=v);
    }
    return;
}
static void execute_blocks(exec_once_block_t * p);
static int execute_dependency(exec_once_tu_t* current);
static void execute_tu(exec_once_tu_t* current)
{
    if(current->done){
        if(0 && exec_once_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] `%s' was already done, cancelled\n", __LINE__, __FUNCTION__,
                    current->name);
        }
        return;
    }
    current->done = 1;
    int ok = execute_dependency(current);
    if(!ok){
        // depenency is running, cancelled.
        current->done = 0;
    }else{
        if(exec_once_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] `%s' start running\n", __LINE__, __FUNCTION__,
                    current->name);
        }
    }
    execute_blocks(*current->head);
    current->done = 2;
    return;
}
static int execute_dependency(exec_once_tu_t* p)
{
    int ret = 1;
    for(int i = 0; i < p->num_of_dep; ++i){
        if(p->depend[i][0] != '\0'){
            fprintf(stderr, "logcial error.\n"); abort();
        }
        exec_once_tu_t * q = (exec_once_tu_t*)p->depend[i];
        switch(q->done){
        case 0:
            execute_tu(q); break;
        case 1: // if dependency is running; cancelled
            ret = 0; break;
        case 2: // if dependency is already done. do nothing
            break;
        default: abort();
        }
    }
    return ret;
}
static void execute_blocks(exec_once_block_t * p)
{
    while(p){
        if(exec_once_debug >8){
            fprintf(stderr, "%s:%d:[%s] start running:\n", p->file, p->line, p->name);}
        p->f();
        if(exec_once_debug>8){
            fprintf(stderr, "%s:%d:[%s] end:\n", p->file, p->line, p->name);
        }
        p = p->next;
    }
}
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
        if(p->index >  x->index){
            x->next = p;
            *glist = x;
        }else{
            for(p = *glist; p->next ; p = p->next){
                if(p->next->index > x->index){
                    break;
                }
            }
            x->next = p->next;
            p->next = x;
        }
    }
}
static int is_over(exec_once_tu_t * head)
{
    int ret = 1;
    for(exec_once_tu_t * p  = head; p ; p = p->next){
        switch(p->done){
        case 0:
            ret = 0; break; // not over
        case 1:
            // if it is running; reentrance, over.
            // let previous exec_once_init to
            break;
        case 2:
            // it is done. OK. continue to check others
            break;
        default: abort();
        }
        if(!ret) break;
    }
    return ret;
}
__attribute__((constructor))
static void __exec_once_init_self()
{
    if(getenv("EXEC_ONCE_DEBUG") != NULL){
        exec_once_debug = atoi(getenv("EXEC_ONCE_DEBUG"));
    }
}
