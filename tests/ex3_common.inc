#include "exec_once.h"
EXEC_ONCE_PROGN
{
    fprintf(stderr,EXEC_ONCE_TU_NAME ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
EXEC_ONCE_PROGN
{
    fprintf(stderr,EXEC_ONCE_TU_NAME ":%d:[%s] \n", __LINE__, __FUNCTION__);
}

