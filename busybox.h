#pragma once
struct cmd {
    const char * name;
    int (*cmd)(int argc, char *argv[]);
    struct cmd * next;
};
extern struct cmd * list;

