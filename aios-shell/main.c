#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include "shell.h"

pid_t shell_pgid;   // global — shared with builtins.c via shell.h

int main() {
    char input[MAX_INPUT];

    // ✅ assign to global directly — no local redeclaration
    shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    signal(SIGINT,  SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        cleanup_jobs();

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("AiSH:%s$ ", cwd);
        else
            printf("AiSH$ ");
        fflush(stdout);

        int n = read(STDIN_FILENO, input, MAX_INPUT - 1);
        if (n < 0) {
            if (errno == EINTR) continue;   // signal interrupted — retry
            // EIO or other error — don't loop, just retry once
            continue;
        }
        if (n == 0) {
            printf("Goodbye!\n");
            break;
        }

        input[n] = '\0';
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        char *args[MAX_ARGS];
        parser_input(input, args);

        char *cmd1[MAX_ARGS];
        char *cmd2[MAX_ARGS];
        char **cmds[MAX_ARGS];
        Redirect r;

        if (parse_multicommands(args, cmds) > 1) {
            for (int i = 0; cmds[i] != NULL; i++) {
                if (parse_pipe(cmds[i], cmd1, cmd2))
                    execute_pipe(cmd1, cmd2);
                else if (!builtin_command(cmds[i])) {
                    parse_redirects(cmds[i], &r);
                    execute_redirect(cmds[i], &r);
                }
            }
        } else if (parse_pipe(args, cmd1, cmd2)) {
            execute_pipe(cmd1, cmd2);
        } else if (!builtin_command(args)) {
            parse_redirects(args, &r);
            execute_redirect(args, &r);
        }
    }
    return 0;
}