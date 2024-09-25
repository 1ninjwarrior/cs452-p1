#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/lab.h"

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    printf("Shell is running...\n");
    return 0;
}