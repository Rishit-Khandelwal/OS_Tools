#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

int main() {
    char input[MAX_INPUT];
    while (1) {
        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("AiSH%s$", cwd);  
        }
        else
        printf("AiSH$");  
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
   
        char *cmd1[MAX_ARGS]; 
        char *cmd2[MAX_ARGS];
        parser_input(input, args);
        if(has_pipe(args, cmd1 , cmd2)){
            execute_pipe(cmd1 , cmd2);
        }
        else if (!builtin_command(args))
            execute_command(args);
        
    }
}
        