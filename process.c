#include <sys/types.h>

typedef struct Process {
    int id;
    int priority;

    char *program;
    char **args;
    int argc;

    pid_t pid;

    int finished;
    int stopped;

    struct Process *next;
} Process;