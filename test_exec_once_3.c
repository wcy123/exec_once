#include <stdio.h>
#include <stdlib.h>


#include "exec_once.h"

EXEC_ONCE_PROGN {
    fprintf(stderr,__FILE__ ":%d:[%s] \n", __LINE__, __FUNCTION__);
}


int main(int argc, char *argv[])
{
    fprintf(stderr,__FILE__ ":%d:[%s] \n", __LINE__, __FUNCTION__);
    return 0;
}


EXEC_ONCE_START
