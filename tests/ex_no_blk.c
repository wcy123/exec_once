#include <stdio.h>
#define EXEC_ONCE_TU_NAME "a"
#include "exec_once.h"
int main(int argc, char *argv[])
{
    printf("start initalization.\n");
    exec_once_init();
    printf("initalization is done.\n");
    return 0;
}
