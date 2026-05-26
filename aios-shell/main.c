#include <stdio.h>
#include <string.h>
#include "shell.h"

int main() {
    char input[MAX_INPUT];
    while (1) {
        printf("AiSH$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {       // handle EOF (Ctrl+D)
            printf("Goodbye!\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;    // strip newline

        if (strlen(input) == 0) {           // handle empty input
            printf("No command entered.\n");
            continue;
        }
        char *args[MAX_ARGS];
        parser_input(input, args);          // parse input into arguments
        if (!builtin_command(args))         // execute built-in command if applicable
            execute_command(args);          // execute external command
    }
}
        