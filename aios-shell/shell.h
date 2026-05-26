#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 1024
#define MAX_ARGS  50

void parser_input(char *input, char **args);
int  builtin_command(char **args);
void execute_command(char **args);

#endif
