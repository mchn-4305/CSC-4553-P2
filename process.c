#include <stdlib.h>
#include "process.h"

void free_process(Process *process) {
    if (process == NULL) {
        return;
    }

    if (process->argv != NULL) {
        for (int i = 0; i < process->argc; i++) {
            free(process->argv[i]);
        }

        free(process->argv);
    }

    free(process);
}