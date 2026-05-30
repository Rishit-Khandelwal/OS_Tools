#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include "shell.h"

void execute_pipeline(char **cmds[], int count) {
    // create count-1 pipes
    int fd[count-1][2];
    for (int i = 0; i < count-1; i++) {
        if (pipe(fd[i]) < 0) {
            perror("pipe failed");
            return;
        }
    }

    pid_t pgid = -1;    // track process group

    for (int i = 0; i < count; i++) {
        Redirect r;
        parse_redirects(cmds[i], &r);

        pid_t pid = fork();
        if (pid < 0) { perror("Fork failed"); return; }
        else if (pid == 0) {
            // ── process group ──────────────────────────
            if (pgid == -1) pgid = getpid();
            setpgid(0, pgid);               // all children same group

            // ── signals ────────────────────────────────
            signal(SIGINT,  SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);

            // ── connect pipes ──────────────────────────
            if (i > 0)                      // not first — read from prev pipe
                dup2(fd[i-1][0], STDIN_FILENO);
            if (i < count-1)                // not last — write to next pipe
                dup2(fd[i][1], STDOUT_FILENO);

            // close ALL pipe fds — child doesn't need them
            for (int j = 0; j < count-1; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            // ── redirections ───────────────────────────
            if (r.infile != NULL) {
                int in = open(r.infile, O_RDONLY);
                if (in < 0) { perror("open infile"); exit(1); }
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if (r.outfile != NULL) {
                int flags = O_WRONLY|O_CREAT|(r.append ? O_APPEND : O_TRUNC);
                int out = open(r.outfile, flags, 0644);
                if (out < 0) { perror("open outfile"); exit(1); }
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            execvp(cmds[i][0], cmds[i]);
            perror("Execution failed");
            exit(1);
        } else {
            // ── parent sets pgid too — race condition fix ──
            if (pgid == -1) pgid = pid;
            setpgid(pid, pgid);
        }
    }

    // parent closes all pipe fds
    for (int i = 0; i < count-1; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }

    // give terminal to pipeline group
    tcsetpgrp(STDIN_FILENO, pgid);

    // wait for all children
    for (int i = 0; i < count; i++)
        wait(NULL);

    // take terminal back
    tcsetpgrp(STDIN_FILENO, shell_pgid);
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