#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busybox.h"
#include "exec_once.h"
struct cmd * list = NULL;
int main(int argc, char *argv[])
{
    exec_once_init();
    struct cmd * p = list;
    while(p){
        if(strcmp(p->name, argv[0]) ==0){
            return p->cmd(argc,argv);
        }
    }
    return 0;
}
