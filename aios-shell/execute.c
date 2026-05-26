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

void execute_pipe(char **cmd1, char **cmd2){
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("Pipe failed");
        return;
    }
    pid_t pid1 = fork();
if (pid1 < 0) {
    perror("Fork 1 failed");
    return;
} else if (pid1 == 0) {
    // child 1 — ls
    dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(cmd1[0], cmd1);
    perror("Execution failed");
    exit(1);
}

pid_t pid2 = fork();
if (pid2 < 0) {
    perror("Fork 2 failed");
    wait(NULL);         // still wait for pid1 child
    return;
} else if (pid2 == 0) {
    // child 2 — grep
    dup2(pipefd[0], STDIN_FILENO);      // redirect stdin to pipe
    close(pipefd[1]);
    close(pipefd[0]);
    execvp(cmd2[0], cmd2);
    perror("Execution failed");
    exit(1);
}

// parent — only reaches here if both forks succeeded
close(pipefd[0]);
close(pipefd[1]);
wait(NULL);
wait(NULL);
}