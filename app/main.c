#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include "../src/lab.h"

#define DEFAULT_PROMPT "$ "

int main(int argc, char *argv[]) {
    int opt;
    char *line;
    const char *prompt;

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

    prompt = getenv("MY_PROMPT");
    if (prompt == NULL) {
        prompt = DEFAULT_PROMPT;
        printf("Using default prompt: \"%s\"\n", prompt);
    } else {
        printf("Using custom prompt from MY_PROMPT: \"%s\"\n", prompt);
    }

    using_history();
    while ((line = readline(prompt)) != NULL) {
        if (strcmp(line, "exit") == 0) {
            break;
        }
        if (strncmp(line, "cd", 2) == 0) {
            char **args = cmd_parse(line);
            change_dir(args);
            cmd_free(args);
        } else if (strcmp(line, "history") == 0) {
            print_history();
        } else if (line[0] != '\0') {
            add_history(line);
            printf("You entered: %s\n", line);
        }
        free(line);
    }

    printf("\nExiting shell...\n");
    return 0;
}