#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
// #include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        //invalid input
        fprintf(stderr, "Usage: ./schedule <quantum-ms> <input-file>");
        return EXIT_FAILURE;
    }

    char *endptr;
    long quantum = strtol(argv[1], &endptr, 10);
    if (end == argv[1] || *endptr != '\0' || quantum <= 0) {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *filename = argv[2];
    FILE *file = fopen(filename, r);

    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;
    while (getline(&line, &capacity, file) != -1) {
        printf("LINE: %s", line);
    }

    free(line);
    fclose(file);
    return EXIT_SUCCESS;
}