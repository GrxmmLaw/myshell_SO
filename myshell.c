#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMDS 10
#define MAX_ARGS 100

void parse_line(char *line, char *cmds[MAX_CMDS]) {
    int i = 0;
    char *token = strtok(line, "|");
    while (token && i < MAX_CMDS) {
        while (*token == ' ') token++;
        cmds[i++] = token;
        token = strtok(NULL, "|");
    }
    cmds[i] = NULL;
}

void parse_cmd(char *cmd, char *args[MAX_ARGS]) {
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int main() {
    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("mishell> ");
        fflush(stdout);

        ssize_t read = getline(&line, &len, stdin);
        if (read == -1) break;

        if (read == 1 || (line[0] == '\n')) continue;

        if (line[read - 1] == '\n') line[read - 1] = '\0';

        if (strcmp(line, "exit") == 0) break;

        char *cmds[MAX_CMDS];
        parse_line(line, cmds);

        int num_cmds = 0;
        while (cmds[num_cmds]) num_cmds++;

        int pipefd[2 * (num_cmds - 1)];
        for (int i = 0; i < num_cmds - 1; i++) {
            if (pipe(pipefd + i*2) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        int status;
        pid_t pid;
        for (int i = 0; i < num_cmds; i++) {
            pid = fork();
            if (pid == 0) {
                if (i > 0) {
                    dup2(pipefd[(i-1)*2], 0); 
                }
                if (i < num_cmds - 1) {
                    dup2(pipefd[i*2+1], 1); 
                }
                for (int j = 0; j < 2*(num_cmds-1); j++) close(pipefd[j]);

                char *args[MAX_ARGS];
                parse_cmd(cmds[i], args);

   
                execvp(args[0], args);

                perror("comando no encontrado");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
        
     for (int j = 0; j < 2*(num_cmds-1); j++) close(pipefd[j]);
        for (int i = 0; i < num_cmds; i++) waitpid(-1, &status, 0); 
    }

    free(line);
    return 0;
}