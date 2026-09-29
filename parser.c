#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "parser.h"

#define INITIAL_ARG_CAPACITY 4

static void strip_quotes(char *text) {
    size_t length = strlen(text);

    if (length >= 2 && text[0] == '"' && text[length-1] == '"') {
        memmove(text, text + 1, length - 2);
        text[length - 2] = '\0';
    }
}

static void strip_newline(char *text) {
    size_t length = strlen(text);

    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        length--;
    }
}

Process *parse_process_line(char *line) {
    Process *process = calloc(1, sizeof(Process));

    if (process == NULL) {
        perror("calloc");
        return NULL;
    }

    char *saveptr = NULL;

    // ID
    char *token = strtok_r(line, "\t", &saveptr);

    if (token == NULL) {
        fprintf(stderr, "Missing process ID\n");
        free(process);
        return NULL
    }

    char *endptr;

    long id = strtol(token, &endptr, 10);

    if (*endptr != '\0' || id <= 0) {
        fprintf(stderr, "Invalid process ID: %s\n," token);
        free(process);
        return NULL;
    }

    process->id = (int) id;

    // Priority
    token = strtok_r(NULL, "\t", &saveptr);
    
    if (token == NULL) {
        fprintf(stderr, "Missing priority\n");
        free(process);
        return NULL;
    }

    long priority = strtol(token, &endptr, 10);

    if (*endptr != '\0' || priority < 0 || priority > 127) {
        fprintf(stderr, "Invalid priority: %s\n", token);
        free(process);
        return NULL;
    }

    process->priority = (int) priority;

    // Executable
    token = strtok_r(NULL, "\t", &saveptr);

    if (token == NULL) {
        fprintf(stderr, "Missing executable\n");
        free(process);
        return NULL;
    }

    strip_newline(token);
    strip_quotes(token);

    int capacity = INITIAL_ARG_CAPACITY;

    process->argv = malloc(sizeof(char *) * capacity);

    if (process->argv == NULL) {
        perror("malloc");
        free(process);
        return NULL;
    }

    process->argv[0] = strdup(token);

    if (process->argv[0] == NULL) {
        perror("strdup");
        free(process->argv);
        free(process);
        return NULL;
    }

    process->argc = 1;

    // Arguments
    while ((token = strtok_r(NULL, "\t", &saveptr)) != NULL) {

        strip_newline(token);
        strip_quotes(token);

        if (process->argc + 1 >= capacity) {

            capacity *= 2;

            char **new_argv =
                realloc(process->argv,
                        sizeof(char *) * capacity);

            if (new_argv == NULL) {
                perror("realloc");
                free_process(process);
                return NULL;
            }

            process->argv = new_argv;
        }

        process->argv[process->argc] = strdup(token);

        if (process->argv[process->argc] == NULL) {
            perror("strdup");
            free_process(process);
            return NULL;
        }

        process->argc++;
    }

    process->argv[process->argc] = NULL;

    process->program = process->argv[0];

    process->pid = -1;
    process->finished = 0;
    process->stopped = 0;
    process->next = NULL;

    return process;
}