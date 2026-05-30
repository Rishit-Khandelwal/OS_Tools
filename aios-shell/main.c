#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "shell.h"

int main() {
    char input[MAX_INPUT];
    signal(SIGINT, SIG_IGN);

    while (1) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("AiSH:%s$ ", cwd);
        else
            printf("AiSH$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Goodbye!\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        char *args[MAX_ARGS];
        parser_input(input, args);

        // ── single execution path ──────────────────
        char *cmd1[MAX_ARGS];
        char *cmd2[MAX_ARGS];
        char **cmds[MAX_ARGS];
        Redirect r;

        if (parse_multicommands(args, cmds) > 1) {
            // "ls ; pwd ; whoami"
            for (int i = 0; cmds[i] != NULL; i++) {
                if (parse_pipe(cmds[i], cmd1, cmd2))
                    execute_pipe(cmd1, cmd2);
                else if (!builtin_command(cmds[i])) {
                    parse_redirects(cmds[i], &r);
                    execute_redirect(cmds[i], &r);
                }
            }
        } else if (parse_pipe(args, cmd1, cmd2)) {
            // "ls | grep main"
            execute_pipe(cmd1, cmd2);

        } else if (!builtin_command(args)) {
            // "ls > out.txt" or just "ls"
            parse_redirects(args, &r);
            execute_redirect(args, &r);
        }
    }
    return 0;
}