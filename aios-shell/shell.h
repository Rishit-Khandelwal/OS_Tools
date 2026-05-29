#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 1024
#define MAX_ARGS  50

typedef struct {
    char *infile;
    char *outfile;
    int   append;
} Redirect;

// parser.c
void parser_input(char *input, char **args);
int  parse_pipe(char **args, char **cmd1, char **cmd2);
int parse_redirects(char **args, Redirect *r);

// builtins.c
int  builtin_command(char **args);

// execute.c
void execute_pipe(char **cmd1, char **cmd2);        
void execute_redirect(char **args, Redirect *r);

#endif
