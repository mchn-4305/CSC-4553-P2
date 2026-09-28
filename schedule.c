#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        //invalid input
        fprintf(stderr, "Usage: ./schedule <quantum-ms> <input-file>");
    }

    char *end;
    long quantum = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0') {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
    }
}