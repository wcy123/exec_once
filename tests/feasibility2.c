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
