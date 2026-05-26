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

int has_pipe(char **args, char **cmd1, char **cmd2){
    int i=0;
    while(args[i]!=NULL){
        if(strcmp(args[i],"|")==0){
            args[i]=NULL;
            
            int j=0;
            while(args[j]!=NULL){
                args[j]=args[j];
                j++;
            }
            cmd1[j]= NULL;

            int k=0;
            while(args[i]!=NULL){
                cmd2[k++]=args[i++];
            }
            cmd2[k]=NULL;
            return 1; // Pipe found
        } 
        i++;
    }
    return 0; // No pipe found
}