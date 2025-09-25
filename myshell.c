#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#define MAX_ARGS 100
#define MAX_CMDS 20

void parse_command(char *line, char **args) {
    int pos = 0;
    char *token = strtok(line, " ");
    while (token != NULL && pos < MAX_ARGS - 1) {
        args[pos++] = token;
        token = strtok(NULL, " ");
    }
    args[pos] = NULL;
}

void execute_command(char **args) {
    pid_t pid = fork(); 
    if (pid == 0) {
        if (execvp(args[0], args) == -1) { 
            perror("Error ejecutando comando");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Error en fork");
    } else {
        int status;
        waitpid(pid, &status, 0); 
    }
}

void handle_pipe(char *line) {
    char *cmds[MAX_CMDS];
    char *args[MAX_CMDS][MAX_ARGS];
    int num_cmds = 0;
    char *token = strtok(line, "|");
    while (token != NULL && num_cmds < MAX_CMDS) {
        while (*token == ' ') token++; 
        cmds[num_cmds++] = token;
        token = strtok(NULL, "|");
    }
    for (int i = 0; i < num_cmds; i++) {
        parse_command(cmds[i], args[i]);
    }

    int pipefd[MAX_CMDS - 1][2];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd[i]) == -1) { 
            perror("Error creando pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork(); 
        if (pid == 0) {
            if (i > 0) {
                dup2(pipefd[i - 1][0], STDIN_FILENO); 
            }
            if (i < num_cmds - 1) {
                dup2(pipefd[i][1], STDOUT_FILENO); 
            }
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }
            if (execvp(args[i][0], args[i]) == -1) { 
                perror("Error ejecutando comando");
            }
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL); 
    }
}

void print_profile(struct timeval start, struct timeval end, struct rusage usage, char *cmd, FILE *out) {
    double real = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6;
    double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1e6;
    double sys = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1e6;
    long mem = usage.ru_maxrss;
    fprintf(out, "Comando: %s\n", cmd);
    fprintf(out, "Tiempo real: %.6f s\n", real);
    fprintf(out, "Tiempo usuario: %.6f s\n", user);
    fprintf(out, "Tiempo sistema: %.6f s\n", sys);
    fprintf(out, "Memoria máxima residente: %ld KB\n", mem);
    fprintf(out, "-----------------------------\n");
}

void run_miprof(char **args, int argc) {
    if (argc < 3) {
        printf("Uso: miprof [ejec|ejecsave archivo|maxtiempo segundos] comando args\n");
        return;
    }
    char mode[32];
    strncpy(mode, args[1], 31);
    mode[31] = 0;

    char cmdline[512] = "";
    int cmd_start = 2;
    FILE *out = stdout;
    int timeout = 0;

    if (strcmp(mode, "ejecsave") == 0) {
        if (argc < 4) {
            printf("Uso: miprof ejecsave archivo comando args\n");
            return;
        }
        out = fopen(args[2], "a");
        if (!out) {
            perror("No se pudo abrir archivo");
            return;
        }
        cmd_start = 3;
    } else if (strcmp(mode, "maxtiempo") == 0) {
        if (argc < 4) {
            printf("Uso: miprof maxtiempo segundos comando args\n");
            return;
        }
        timeout = atoi(args[2]);
        cmd_start = 3;
    }

    for (int i = cmd_start; i < argc; i++) {
        strcat(cmdline, args[i]);
        if (i < argc - 1) strcat(cmdline, " ");
    }

    struct timeval start, end;
    struct rusage usage;
    pid_t pid;
    gettimeofday(&start, NULL);

    pid = fork();
    if (pid == 0) {
        if (timeout > 0) {
            alarm(timeout);
        }
        execvp(args[cmd_start], &args[cmd_start]);
        perror("Error ejecutando comando");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        if (timeout > 0) {
            int finished = 0;
            for (int i = 0; i < timeout * 10; i++) {
                usleep(100000);
                pid_t res = waitpid(pid, &status, WNOHANG);
                if (res == pid) {
                    finished = 1;
                    break;
                }
            }
            if (!finished) {
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                fprintf(out, "Comando terminado por exceder tiempo máximo (%d s)\n", timeout);
            }
        } else {
            waitpid(pid, &status, 0);
        }
        gettimeofday(&end, NULL);
        getrusage(RUSAGE_CHILDREN, &usage);
        print_profile(start, end, usage, cmdline, out);
        if (out != stdout) fclose(out);
    } else {
        perror("Error en fork");
    }
}

int main(){
    printf("   _______________________________\n");
    printf("  /\\                              \\\n");
    printf(" /++\\    __________________________\\\n");
    printf(" \\+++\\   \\ ************************/\n");
    printf("  \\+++\\   \\___________________ ***/\n");
    printf("   \\+++\\   \\             /+++/***/\n");
    printf("    \\+++\\   \\           /+++/***/\n");
    printf("     \\+++\\   \\         /+++/***/\n");
    printf("      \\+++\\   \\       /+++/***/\n");
    printf("       \\+++\\   \\     /+++/***/\n");
    printf("        \\+++\\   \\   /+++/***/\n");
    printf("         \\+++\\   \\ /+++/***/\n");
    printf("          \\+++\\   /+++/***/\n");
    printf("           \\+++\\ /+++/***/\n");
    printf("            \\+++++++/***/\n");
    printf("             \\+++++/***/\n");
    printf("              \\+++/***/\n");
    printf("               \\+/___/\n\n");
    printf("      ¡Bienvenido a noha_shell!\n\n");

    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("noha_shell:$ ");
        if (getline(&line, &len, stdin) == -1) {
            perror("Error procesando el comando");
            free(line);
            exit(EXIT_FAILURE);
        }
        line[strcspn(line, "\n")] = 0;

        if (line[0] == '\0') {
            continue;
        }
        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (strchr(line, '|')) {
            handle_pipe(line);
        } else {
            char *args[MAX_ARGS];
            parse_command(line, args);
            if (args[0] == NULL) continue;
            if (strcmp(args[0], "miprof") == 0) {
                int argc = 0;
                while (args[argc]) argc++;
                run_miprof(args, argc);
            } else {
                execute_command(args);
            }
        }
    }
    free(line);
    return 0;
}
