#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

typedef struct Process {
    int id;
    int priority;
    char *program;
    char **argv;
    int argc;
    pid_t pid;
    int finished;
    int stopped;
    struct Process *next;
} Process;

void free_process(Process *process);

#endif