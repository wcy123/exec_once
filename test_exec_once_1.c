#include <stdio.h>
#include <stdlib.h>


#include "exec_once.h"

static void foo()
{
    fprintf(stderr,__FILE__ ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
static void bar()
{
    fprintf(stderr,__FILE__ ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
EXEC_ONCE(foo)
EXEC_ONCE(bar)
EXEC_ONCE_START

int main(int argc, char *argv[])
{
    fprintf(stderr,__FILE__ ":%d:[%s] start exec once\n", __LINE__, __FUNCTION__);
    exec_once_run();
    fprintf(stderr,__FILE__ ":%d:[%s] end exec once\n", __LINE__, __FUNCTION__);    
    return 0;
}
