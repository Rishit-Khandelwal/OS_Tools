#include <string.h>
#include <fcntl.h>
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

int parse_pipe(char **args, char **cmd1, char **cmd2) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;

            // copy left side into cmd1
            int j = 0;
            while (args[j] != NULL) {
                cmd1[j] = args[j];   // ✅ separated
                j++;
            }
            cmd1[j] = NULL;

            // copy right side into cmd2
            int k = 0;
            i++;
            while (args[i] != NULL) {
                cmd2[k] = args[i];   // ✅ separated
                k++;
                i++;
            }
            cmd2[k] = NULL;
            return 1;
        }
        i++;
    }
    return 0;
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