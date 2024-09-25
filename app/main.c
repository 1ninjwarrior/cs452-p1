#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../src/lab.h"

int main(int argc, char *argv[]) {
    int opt;
    char *line;

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

    using_history();
    while ((line = readline("$ ")) != NULL) {
        if (line[0] != '\0') {
            add_history(line);
            printf("You entered: %s\n", line);
        }
        free(line);
    }

    printf("\nExiting shell...\n");
    return 0;
}