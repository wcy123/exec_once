#include <stdio.h>
#include <stdlib.h>
#include <exec_once.h>
int main(int argc, char *argv[])
{
    printf("start to init exec_once\n");
    exec_once_init();
    printf("after init exec_once\n");
    return 0;
}



