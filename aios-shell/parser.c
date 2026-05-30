#include <string.h>
#include <fcntl.h>
#include "shell.h"

void parser_input(char *input, char **args) {
    static char processed[MAX_INPUT * 2];
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i]=='|' || input[i]==';' || 
            input[i]=='>' || input[i]=='<') {
            processed[j++] = ' ';
            processed[j++] = input[i];
            // handle >> as one token
            if (input[i]=='>' && input[i+1]=='>') {
                processed[j++] = input[++i];
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
        token = strtok(NULL, " ");      // Get next token
    }
    args[i] = NULL;                     // Null-terminate the args array
}

int parse_pipe(char **args, char **cmd1, char **cmd2){
    int i=0;
    while(args[i]!=NULL){
        if(strcmp(args[i],"|")==0){
            args[i]=NULL;
            
            int j=0;
            while(args[j]!=NULL){
                cmd1[j]=args[j];         // Copy args to cmd1 until pipe symbol
                j++;
            }
            cmd1[j]= NULL;

            int k=0;
            while(args[i]!=NULL){
                cmd2[k++]=args[i++];        // Copy remaining args to cmd2
            }
            cmd2[k]=NULL;
            return 1;           // Pipe found 
        } 
        i++;
    }
    return 0;                   // No pipe found 
}

int parse_redirects (char **args , Redirect *r){
    r->infile=NULL;
    r->outfile=NULL;
    r->append=0;

    int i=0;
    while(args[i]!=NULL){
        if(strcmp(args[i],"<")==0){
            r->infile=args[i+1];
            args[i]=NULL;
        }
        else if(strcmp(args[i],">")==0){
            r->outfile=args[i+1];
            args[i]=NULL;
        }
        else if (strcmp(args[i], ">>") == 0) {
            r->outfile = args[i+1];
            r->append  = 1;
            args[i] = NULL;
        }
        i++;
    }
    return 0;
}

int parse_multicommands(char **args , char ***cmds){
    int count=0;
    int i=0;
    cmds[count++]=&args[0];     // First command starts at args[0]
    while(args[i]!=NULL){
        if(strcmp(args[i],";")==0){
            args[i]=NULL;          // Null-terminate current command
            cmds[count++]=&args[i+1];  // Next command starts after ;
        }
        i++;
    }
    cmds[count]=NULL;           // Null-terminate the cmds array
    return count;               // Return number of commands parsed
}