#include <stdio.h>
#define EXEC_ONCE_TU_NAME "a"
#define EXEC_ONCE_DEPENDS { "no-exists", NULL}
#include "exec_once.h"
EXEC_ONCE_PROGN {
    printf("Hello World from a constructor 1\n");
}
EXEC_ONCE_PROGN {
    printf("Hello World from a constructor 2\n");
}
