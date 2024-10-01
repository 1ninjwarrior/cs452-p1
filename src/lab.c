/** Shell Starter Code **/
#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>

#define SHELL_VERSION_MAJOR 1
#define SHELL_VERSION_MINOR 0
#define MAX_BG_JOBS 100

volatile pid_t foreground_pid = 0;

struct bg_job {
    int job_id;
    pid_t pid;
    char *command;
};

struct bg_job bg_jobs[MAX_BG_JOBS];
int next_job_id = 1;

/** Executes a command and handles foreground/background jobs **/
int execute_command(char **args) {
    pid_t pid;
    int bg = 0;

    if (args[0] == NULL || (args[0][0] == '&' && args[0][1] == '\0' && args[1] == NULL)) {
        fprintf(stderr, "Error: Empty command\n");
        return 1;
    }

    for (int i = 0; args[i] != NULL; i++) {
        if (args[i + 1] == NULL && strcmp(args[i], "&") == 0) {
            bg = 1;
            args[i] = NULL;
            break;
        }
    }
    
    pid = fork();
    if (pid == 0) {
        // Child process
        pid_t child = getpid();
        setpgid(child, child);
        
        // Reset signal handlers to default
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            perror("shell"); 
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        if (bg) {
            add_bg_job(pid, args);
            printf("[%d] %d %s\n", next_job_id, pid, args[0]);
        } else {
            int status;
            waitpid(pid, &status, WUNTRACED);
        }
    }
    setup_shell_signal_handlers(); // Ensure signal handlers are reset after command execution
    return 1;
}

/** Setup signal handling for the shell **/
void setup_shell_signal_handlers(void) {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}

/** Add a job to the background job list **/
void add_bg_job(pid_t pid, char **args) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].pid == 0) {
            bg_jobs[i].job_id = i+1;
            next_job_id = i+1;
            bg_jobs[i].pid = pid;

            char full_command[1024] = "";
            for (int j = 0; args[j] != NULL; j++) {
                strcat(full_command, args[j]);
                strcat(full_command, " ");
            }
            strcat(full_command, "&");
            bg_jobs[i].command = strdup(full_command);
            return;
        }
    }
    fprintf(stderr, "Maximum number of background jobs reached\n");
}

/** Check status of background jobs and remove completed jobs **/
void check_bg_jobs(void) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].pid != 0) {
            int status;
            pid_t result = waitpid(bg_jobs[i].pid, &status, WNOHANG);
            if (result == bg_jobs[i].pid) {
                printf("[%d] Done %s\n", bg_jobs[i].job_id, bg_jobs[i].command);
                free(bg_jobs[i].command);

                bg_jobs[i].pid = 0;
            }
        }
    }
}

/** Prints shell version **/
void print_version(void) {
    printf("Shell version %d.%d\n", SHELL_VERSION_MAJOR, SHELL_VERSION_MINOR);
}

/** Change directory command **/
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

/** Frees dynamically allocated command strings **/
void cmd_free(char **cmd) {
    if (cmd == NULL) return;
    for (int i = 0; cmd[i] != NULL; i++) {
        free(cmd[i]);
    }
    free(cmd);
}

/** Parses input command into individual arguments **/
char **cmd_parse(const char *input) {
    char **result = NULL;
    char *token;
    int count = 0;
    char *input_copy = strdup(input);
    long arg_max = sysconf(_SC_ARG_MAX);

    token = strtok(input_copy, " ");
    while (token != NULL && count < arg_max) {
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

/** Retrieves the shell prompt, defaults to "shell>" **/
char *get_prompt(const char *env_var) {
    char *prompt = getenv(env_var);
    if (prompt == NULL) {
        return strdup("shell>");
    }
    return strdup(prompt);
}

/** Trims white spaces from the input string **/
char *trim_white(char *str) {
    char *end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return str;

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

/** Prints the shell's history **/
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

/** Checks if the command is a shell built-in command **/
bool do_builtin(struct shell *sh, char **argv) {
    UNUSED(sh);

    if (strcmp(argv[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    } else if (strcmp(argv[0], "history") == 0) {
        print_history();
        return true;
    } else if (strcmp(argv[0], "jobs") == 0) {
        print_jobs();
        return true;
    }

    return false;
}

/** Prints all current background jobs **/
void print_jobs(void) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].pid != 0) {
            int status;
            pid_t result = waitpid(bg_jobs[i].pid, &status, WNOHANG);
            
            if (result == 0) {
                printf("[%d] %d Running %s\n", bg_jobs[i].job_id, bg_jobs[i].pid, bg_jobs[i].command);
            } else if (result == bg_jobs[i].pid) {
                printf("[%d] Done    %s\n", bg_jobs[i].job_id, bg_jobs[i].command);
                bg_jobs[i].pid = 0;
                free(bg_jobs[i].command);
            } else if (result == -1) {
                perror("waitpid");
            }
        }
    }
}