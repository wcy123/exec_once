#include <stdio.h>
__attribute__((constructor))
static void foo1() 
{
    printf("Hello World from a constructor foo1\n");
}
__attribute__((constructor))
static void foo2() 
{
    printf("Hello World from a constructor foo2\n");
}

int main(int argc, char *argv[])
{
    printf("Hello World");
    return 0;
}
