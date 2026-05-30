#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include "shell.h"

void execute_pipe(char **cmd1, char **cmd2) {
    Redirect r1, r2;
    parse_redirects(cmd1, &r1);
    parse_redirects(cmd2, &r2);

    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 < 0) { perror("Fork 1 failed"); return; }
    else if (pid1 == 0) {
        setpgid(0, 0);                      // ✅ own process group
        signal(SIGINT,  SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        dup2(fd[1], STDOUT_FILENO);         // write to pipe
        close(fd[0]);
        close(fd[1]);
        if (r1.infile != NULL) {
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
        setpgid(0, pid1);                   // ✅ same group as pid1
        signal(SIGINT,  SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        dup2(fd[0], STDIN_FILENO);          // read from pipe
        close(fd[1]);
        close(fd[0]);
        if (r2.outfile != NULL) {
            int flags = O_WRONLY|O_CREAT|(r2.append ? O_APPEND : O_TRUNC);
            int out = open(r2.outfile, flags, 0644);
            if (out < 0) { perror("open outfile"); exit(1); }
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        execvp(cmd2[0], cmd2);
        perror("Execution failed");
        exit(1);
    }

    // parent
    setpgid(pid1, pid1);                    // ✅ race condition fix
    setpgid(pid2, pid1);
    close(fd[0]);
    close(fd[1]);
    tcsetpgrp(STDIN_FILENO, pid1);          // ✅ give terminal to pipe group
    wait(NULL);
    wait(NULL);
    tcsetpgrp(STDIN_FILENO, shell_pgid);    // ✅ take terminal back
}

void execute_redirect(char **args, Redirect *r) {
    int background = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL;
            break;
        }
    }

    pid_t pid = fork();
    if (pid < 0) { perror("Fork failed"); return; }
    else if (pid == 0) {
        setpgid(0, 0);
        signal(SIGINT,  SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        if (r->infile != NULL) {
            int fd = open(r->infile, O_RDONLY);
            if (fd < 0) { perror("open infile"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (r->outfile != NULL) {
            int flags = O_WRONLY|O_CREAT|(r->append ? O_APPEND : O_TRUNC);
            int fd = open(r->outfile, flags, 0644);
            if (fd < 0) { perror("open outfile"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(args[0], args);
        perror("Execution Failed");
        exit(1);
    } else {
        setpgid(pid, pid);
        if (background) {
            add_job(pid, args[0]);
            printf("[%d] %d\n", job_count, pid);
        } else {
            tcsetpgrp(STDIN_FILENO, pid);

            int status;
            waitpid(pid, &status, WUNTRACED);   // ✅ detects Ctrl+Z

            tcsetpgrp(STDIN_FILENO, shell_pgid);

            if (WIFSTOPPED(status)) {
                // Ctrl+Z — add to job table as stopped
                add_job(pid, args[0]);
                jobs[job_count-1].status = STOPPED;
                printf("\n[%d] Stopped \t %s\n", job_count, args[0]);
            }
        }
    }
}