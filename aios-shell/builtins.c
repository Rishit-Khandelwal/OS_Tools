#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

int builtin_command(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL)
            chdir(getenv("HOME"));
        else if (chdir(args[1]) != 0)
            perror("cd failed");
        return 1;
    }
    if (strcmp(args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    return 0;
}