/**Update this file with the starter code**/
#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <readline/history.h>

#define SHELL_VERSION_MAJOR 1
#define SHELL_VERSION_MINOR 0

void print_version(void) {
    printf("Shell version %d.%d\n", SHELL_VERSION_MAJOR, SHELL_VERSION_MINOR);
}

int change_dir(char **dir) {
    const char *new_dir;
    
    if (dir[1] == NULL) {
        new_dir = getenv("HOME");
        if (new_dir == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                perror("Unable to determine home directory");
                return -1;
            }
            new_dir = pw->pw_dir;
        }
    } else {
        new_dir = dir[1];
    }
    
    if (chdir(new_dir) != 0) {
        perror("cd failed");
        return -1;
    }
    
    printf("Changed directory to: %s\n", new_dir);
    return 0;
}

void cmd_free(char **cmd) {
    if (cmd == NULL) return;
    for (int i = 0; cmd[i] != NULL; i++) {
        free(cmd[i]);
    }
    free(cmd);
}

char **cmd_parse(const char *input) {
    char **result = NULL;
    char *token;
    int count = 0;
    char *input_copy = strdup(input);

    token = strtok(input_copy, " ");
    while (token != NULL) {
        result = realloc(result, sizeof(char*) * (count + 1));
        result[count] = strdup(token);
        count++;
        token = strtok(NULL, " ");
    }

    result = realloc(result, sizeof(char*) * (count + 1));
    result[count] = NULL;

    free(input_copy);
    return result;
}

char *get_prompt(const char *env_var) {
    char *prompt = getenv(env_var);
    if (prompt == NULL) {
        return strdup("shell>");
    }
    return strdup(prompt);
}

char *trim_white(char *str) {
    char *end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return str;

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

void print_history(void) {
    HIST_ENTRY *entry;
    int i;

    using_history();
    for (i = 0; ; i++) {
        entry = history_get(i + history_base);
        if (entry == NULL) {
            break;
        }
        printf("%d: %s\n", i + history_base, entry->line);
    }
}

bool do_builtin(struct shell *sh, char **argv) {
    UNUSED(sh);

    if (strcmp(argv[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    } else if (strcmp(argv[0], "history") == 0) {
        print_history();
        return true;
    }

    return false;
}