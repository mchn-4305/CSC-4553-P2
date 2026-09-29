#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char *text = argv[1];
    int n = atoi(argv[2]);

    for (int i = 0; i < n; i++) {
        printf("%s\n", text);
        fflush(stdout);
        sleep(1);
    }

    return 0;
}