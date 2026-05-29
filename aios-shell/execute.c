#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <string.h>
#include <stdlib.h>
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

void execute_pipe(char **cmd1, char **cmd2) {
    Redirect r1, r2;
    parse_redirects(cmd1, &r1);    // parse left  side redirects
    parse_redirects(cmd2, &r2);    // parse right side redirects

    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 < 0) { perror("Fork 1 failed"); return; }
    else if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);   // left cmd writes to pipe
        close(fd[0]);
        close(fd[1]);
        if (r1.infile != NULL) {      // handle < on left side
            int in = open(r1.infile, O_RDONLY);
            if (in < 0) { perror("open infile"); exit(1); }
            dup2(in, STDIN_FILENO);
            close(in);
        }
        execvp(cmd1[0], cmd1);
        perror("Execution failed");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) { perror("Fork 2 failed"); wait(NULL); return; }
    else if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);    // right cmd reads from pipe
        close(fd[1]);
        close(fd[0]);
        if (r2.outfile != NULL) {     // handle > or >> on right side
            int flags = O_WRONLY | O_CREAT | (r2.append ? O_APPEND : O_TRUNC);
            int out = open(r2.outfile, flags, 0644);
            if (out < 0) { perror("open outfile"); exit(1); }
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        execvp(cmd2[0], cmd2);
        perror("Execution failed");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
} 

void execute_redirect(char **args, Redirect *r) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Failed");
        return;
    }
    else if (pid == 0) {
        // input redirection  < infile
        if (r->infile != NULL) {
            int fd = open(r->infile, O_RDONLY);
            if (fd < 0) { perror("open infile"); exit(1); }
            dup2(fd, STDIN_FILENO);         // stdin now reads from file
            close(fd);                      // fd no longer needed
        }

        // output redirection  > or >> outfile
        if (r->outfile != NULL) {
            int flags = O_WRONLY | O_CREAT | (r->append ? O_APPEND : O_TRUNC);
            int fd = open(r->outfile, flags, 0644);
            if (fd < 0) { perror("open outfile"); exit(1); }
            dup2(fd, STDOUT_FILENO);        // stdout now writes to file
            close(fd);                      // fd no longer needed
        }

        execvp(args[0], args);
        perror("Execution Failed");
        exit(1);
    }
    else {
        wait(NULL);
    }
}