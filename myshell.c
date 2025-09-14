#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


void print_prompt() {
    printf("mishell:$ ");
    fflush(stdout);
}

int parse_line(char *line, char ***commands) {
    int size = 8, count = 0;
    *commands = malloc(size * sizeof(char*));
    char *cmd = strtok(line, "|");
    while (cmd != NULL) {
        if (count >= size) {
            size *= 2;
            *commands = realloc(*commands, size * sizeof(char*));
        }
        (*commands)[count++] = cmd;
        cmd = strtok(NULL, "|");
    }
    return count;
}

void parse_args(char *command, char ***args, int *argc) {
    int size = 8;
    *args = malloc(size * sizeof(char*));
    *argc = 0;
    char *arg = strtok(command, " \t\n");
    while (arg != NULL) {
        if (*argc >= size) {
            size *= 2;
            *args = realloc(*args, size * sizeof(char*));
        }
        (*args)[(*argc)++] = arg;
        arg = strtok(NULL, " \t\n");
    }
    (*args)[*argc] = NULL;
}

void execute_pipeline(char **commands, int num_cmds) {
    int i, fd[2];
    pid_t pid;
    int prev_fd = -1;

    for (i = 0; i < num_cmds; i++) {
        char **args;
        int argc;
        parse_args(commands[i], &args, &argc);

        if (i < num_cmds - 1) pipe(fd);

        pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (i < num_cmds - 1) {
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);
            }
            execvp(args[0], args);
            perror("Error al ejecutar comando");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Error en fork");
            return;
        }
        if (i > 0) close(prev_fd);
        if (i < num_cmds - 1) {
            close(fd[1]);
            prev_fd = fd[0];
        }
        free(args); 
    }
    for (i = 0; i < num_cmds; i++) wait(NULL);
}

int main() {
    char *line = NULL;
    size_t len = 0;
    char **commands;

    while (1) {
        print_prompt();
        ssize_t read = getline(&line, &len, stdin); 
        if (read == -1) break;
        if (line[0] == '\n') continue;
        line[strcspn(line, "\n")] = 0;
        int num_cmds = parse_line(line, &commands);
        if (num_cmds == 1 && strcmp(commands[0], "exit") == 0) {
            free(commands);
            free(line);
            exit(0);
        }
        execute_pipeline(commands, num_cmds);
        free(commands);
    }
    free(line);
    return 0;
}