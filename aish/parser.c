#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "shell.h"

void parser_input(char *input, char **args) {
    static char processed[MAX_INPUT * 2];
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '|' || input[i] == ';' ||
            input[i] == '<' || input[i] == '&') {
            processed[j++] = ' ';
            processed[j++] = input[i];
            processed[j++] = ' ';
        } else if (input[i] == '>') {
            processed[j++] = ' ';
            processed[j++] = '>';
            if (input[i+1] == '>') {
                processed[j++] = '>';
                i++;
            }
            processed[j++] = ' ';
        } else {
            processed[j++] = input[i];
        }
    }
    processed[j] = '\0';

    char *token = strtok(processed, " ");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int parse_all_pipes(char **args, char **cmds[], int max) {
    int count = 0;
    int i = 0;

    cmds[count++] = &args[0];      // first command starts at args[0]

    while (args[i] != NULL && count < max) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;             // split here — terminates current cmd
            cmds[count++] = &args[i+1]; // next cmd starts after |
        }
        i++;
    }
    cmds[count] = NULL;             // null terminate cmds array
    return count;                   // number of commands found
}

int parse_redirects(char **args, Redirect *r) {
    r->infile  = NULL;
    r->outfile = NULL;
    r->append  = 0;

    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            r->infile  = args[i+1];
            args[i]    = NULL;      // remove 
            args[i+1]  = NULL;      // ✅ remove filename too
        } else if (strcmp(args[i], ">") == 0) {
            r->outfile = args[i+1];
            args[i]    = NULL;      // remove >
            args[i+1]  = NULL;      // ✅ remove filename too
        } else if (strcmp(args[i], ">>") == 0) {
            r->outfile = args[i+1];
            r->append  = 1;
            args[i]    = NULL;      // remove >>
            args[i+1]  = NULL;      // ✅ remove filename too
        }
        i++;
    }
    return 0;
}

int parse_multicommands(char **args, char ***cmds) {
    int count = 0;
    int i = 0;
    cmds[count++] = &args[0];
    while (args[i] != NULL) {
        if (strcmp(args[i], ";") == 0) {
            args[i]    = NULL;
            cmds[count++] = &args[i+1];
        }
        i++;
    }
    cmds[count] = NULL;
    return count;
}

void parse_env_vars(char **args){
    for(int i=0; args[i]!=NULL; i++){
        if(args[i][0]=='$'){
            char *var= args[i]+1;
            char *val= getenv(var);
            if(val!=NULL)
                args[i]=val;
             else 
                args[i]="no such variable"; 
        }
    }
}