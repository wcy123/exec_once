#define EXEC_ONCE_TU_NAME FILE_X
#include "exec_once.h"
EXEC_ONCE_PROGN
{
    fprintf(stderr,FILE_X ":%d:[%s] \n", __LINE__, __FUNCTION__);
}
EXEC_ONCE_PROGN
{
    fprintf(stderr,FILE_X ":%d:[%s] \n", __LINE__, __FUNCTION__);
}

