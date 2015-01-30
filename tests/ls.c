#define EXEC_ONCE_TU_NAME "ls"
#include "exec_once.h"
#include "busybox.h"

static int ls(int argc, char *argv[])
{
    fprintf(stderr,__FILE__ ":%d:[%s] this is ls, haha\n", __LINE__, __FUNCTION__);
    return 0;
}
EXEC_ONCE_PROGN {
    static struct cmd x = { "ls", ls, NULL };
    x.next = list;
    list = &x;
}
