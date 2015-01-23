#define EXEC_ONCE_TU_NAME "test_4"
#define EXEC_ONCE_DEPENDS {"test3.c",NULL}
#include "exec_once.h"

EXEC_ONCE_PROGN {
    printf("hello from test 4\n");
}



