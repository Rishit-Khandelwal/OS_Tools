#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

int main() {
    char input[MAX_INPUT];

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

        char *cmd1[MAX_ARGS];
        char *cmd2[MAX_ARGS];
        Redirect r;

        if (parse_pipe(args, cmd1, cmd2)) {
            execute_pipe(cmd1, cmd2);           // ← redirects handled inside

        } else if (!builtin_command(args)) {
            parse_redirects(args, &r);          // ← redirects handled here
            execute_redirect(args, &r);
        }
    }
    return 0;
}