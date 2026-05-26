#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

void execute_command(char **args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("Execution failed");
        exit(1);
    } else {
        wait(NULL);
    }
}