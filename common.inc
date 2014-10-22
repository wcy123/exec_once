#define EXEC_ONCE_TU_NAME FILE_X
#include "exec_once.h"
static void foo()
{
    fprintf(stderr,FILE_X ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
static void bar()
{
    fprintf(stderr,FILE_X ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
EXEC_ONCE(foo)
EXEC_ONCE(bar)
