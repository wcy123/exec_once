// in bar.c
#define EXEC_ONCE_TU_NAME "bar"
#define EXEC_ONCE_DEPENDS {"foo", NULL}
#include <exec_once.h>
EXEC_ONCE_PROGN {
    printf("Hello World From Bar\n");
}
