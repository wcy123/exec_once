#include <stdio.h>
__attribute__((constructor))
static void foo() 
{
    printf("Hello World from a constructor\n");
}

int main(int argc, char *argv[])
{
    printf("Hello World");
    return 0;
}
