#include <string.h>
#include "shell.h"

void parser_input(char *input, char **args) {
    char *token = strtok(input, " ");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");      // Get next token
    }
    args[i] = NULL;                     // Null-terminate the args array
}