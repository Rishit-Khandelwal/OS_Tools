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

pid_t shell_pgid;

int main() {
    char input[MAX_INPUT];

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

        // ✅ replaced read() with arrow-aware input
        if (read_input(input, MAX_INPUT) == NULL) {
            printf("Goodbye!\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        add_history(input);            // ✅ store every command

        char *args[MAX_ARGS];
        parser_input(input, args);
        parse_env_vars(args);

        char **pipe_cmds[MAX_ARGS];
        char **cmds[MAX_ARGS];
        Redirect r;

        if (parse_multicommands(args, cmds) > 1) {
            for (int i = 0; cmds[i] != NULL; i++) {
                parse_env_vars(cmds[i]);

                char **pipe_cmds[MAX_ARGS];
                int pipe_count = parse_all_pipes(cmds[i], pipe_cmds, MAX_ARGS);

                if (pipe_count > 1) {
                    execute_pipeline(pipe_cmds, pipe_count);
                } else if (!builtin_command(cmds[i])) {
                    parse_redirects(cmds[i], &r);
                    execute_redirect(cmds[i], &r);
                }
            }
        } else {
            int pipe_count = parse_all_pipes(args, pipe_cmds, MAX_ARGS);

            if (pipe_count > 1) {
                execute_pipeline(pipe_cmds, pipe_count);
            } else if (!builtin_command(args)) {
                parse_redirects(args, &r);
                execute_redirect(args, &r);
            }
        }
    }

    free_history();    // ✅ clean up on exit
    return 0;
}