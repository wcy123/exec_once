#include <stdio.h>
#include <stdlib.h>
#include "exec_once.h"
int main(int argc, char *argv[])
{
    fprintf(stderr,__FILE__ ":%d:[%s] start exec once\n", __LINE__, __FUNCTION__);
    exec_once_init();
    fprintf(stderr,__FILE__ ":%d:[%s] end exec once\n", __LINE__, __FUNCTION__);    
    return 0;
}
