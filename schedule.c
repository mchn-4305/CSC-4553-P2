#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        //invalid input
        fprintf(stderr, "Usage: ./schedule <quantum-ms> <input-file>");
        return EXIT_FAILURE;
    }

    char *end;
    long quantum = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0') {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *filename = argv[2];
    FILE *file = fopen(filename, r);

    if (file == NULL) {
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