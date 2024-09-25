#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "../src/lab.h"

#define DEFAULT_PROMPT "$ "
#define MAX_ARGS 64

int main(int argc, char *argv[]) {
    int opt;
    char *line;
    const char *prompt;
    char *args[MAX_ARGS];
    int arg_count;

    // Set up signal handlers
    setup_shell_signal_handlers();

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
        if (line[0] != '\0') {
            add_history(line);
            
            // Parse the command into arguments
            arg_count = 0;
            args[arg_count] = strtok(line, " \t\n");
            while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
                arg_count++;
                args[arg_count] = strtok(NULL, " \t\n");
            }
            args[arg_count] = NULL;

            if (arg_count > 0) {
                if (strcmp(args[0], "exit") == 0) {
                    break;
                } else if (strcmp(args[0], "history") == 0) {
                    print_history();
                } else if (strcmp(args[0], "cd") == 0) {
                    if (arg_count > 1) {
                        if (chdir(args[1]) != 0) {
                            perror("cd");
                        }
                    } else {
                        // Change to home directory if no argument is provided
                        const char *home = getenv("HOME");
                        if (home && chdir(home) != 0) {
                            perror("cd");
                        }
                    }
                } else {
                    // Execute external command
                    execute_command(args);
                }
            }
        }
        free(line);
    }

    printf("\nExiting shell...\n");
    return 0;
}