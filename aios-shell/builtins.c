#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "shell.h"

int builtin_command(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL)
            chdir(getenv("HOME"));
        else if (chdir(args[1]) != 0)
            perror("cd failed");
        return 1;
    }
//-----------------------------------------------------------------------------
    if (strcmp(args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
//-----------------------------------------------------------------------------
    if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return 1;
    }

//-----------------------------------------------------------------------------

    if(strcmp(args[0], "export")==0){
        if(args[1]==NULL){
            printf("export:no variable given\n");
            return 1;
        }
        char *eq= strchr(args[1], '=');
        if(eq==NULL){
            printf("export: invalid format use VAR=value\n");
            return 1;
        }
        *eq='\0';
        char *var=args[1];
        char *val=eq+1;
        setenv(var, val, 1);
        return 1;
    }
//-----------------------------------------------------------------------------
    if(strcmp(args[0], "unset")==0){
        if(args[1]==NULL){
            printf("unset: no variable given\n");
            return 1;
        }
        unsetenv(args[1]);
        return 1;
    }
//-----------------------------------------------------------------------------
    if (strcmp(args[0], "fg") == 0) {
        if (args[1] == NULL) {
             printf("fg: no job number given\n"); return 1; }
        int idx = atoi(args[1]) - 1;
        if (idx < 0 || idx >= job_count) { 
            printf("fg: no such job\n"); return 1; }
        if (jobs[idx].status == DONE) { 
            printf("fg: job already done\n"); return 1; }

        pid_t pid = jobs[idx].pid;
        jobs[idx].status = RUNNING;
        printf("fg: %s\n", jobs[idx].cmd);

        // give terminal to child
        tcsetpgrp(STDIN_FILENO, pid);
        kill(-pid, SIGCONT);

        // wait for exit OR Ctrl+Z stop
        int status;
        waitpid(pid, &status, WUNTRACED);

        // take terminal back to shell
        tcsetpgrp(STDIN_FILENO, shell_pgid);

        if (WIFSTOPPED(status)) {
            jobs[idx].status = STOPPED;
            printf("\n[%d] Stopped \t %s\n", idx+1, jobs[idx].cmd);
        } else {
            remove_job(pid);
        }
        return 1;
    }
//-------------------------------------------------------------------------------
    if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) { printf("bg: no job number given\n"); return 1; }
        int idx = atoi(args[1]) - 1;
        if (idx < 0 || idx >= job_count) { printf("bg: no such job\n"); return 1; }
        if (jobs[idx].status == DONE) { printf("bg: job already done\n"); return 1; }

        jobs[idx].status = RUNNING;
        kill(-jobs[idx].pid, SIGCONT);
        printf("[%d] %d running \t %s\n", idx+1, jobs[idx].pid, jobs[idx].cmd);
        return 1;
    }

//-------------------------------------------------------------------------------
    if (strcmp(args[0], "history") == 0) {
        print_history();
        return 1;
    }
//-------------------------------------------------------------------------------
    if (strcmp(args[0], "!!") == 0) {
        if (history_count == 0) {
            printf("No commands in history\n");
            return 1;
        }
        char *last = get_history(history_count - 1);
        printf("%s\n", last);          // print command being run
        
        // re-parse and execute last command
        char copy[MAX_INPUT];
        strncpy(copy, last, MAX_INPUT);
        char *args2[MAX_ARGS];
        parser_input(copy, args2);
        parse_env_vars(args2);
        if (!builtin_command(args2)) {
            Redirect r;
            parse_redirects(args2, &r);
            execute_redirect(args2, &r);
        }
        return 1;
    }
//-------------------------------------------------------------------------------
    if (strcmp(args[0], "help") == 0) {
        printf("\n");
        printf("  AiSH — Available Commands\n");
        printf("─────────────────────────────────────\n");
        printf("BUILTINS:\n");
        printf("  cd [dir]          change directory\n");
        printf("  exit              exit the shell\n");
        printf("  jobs              list background jobs\n");
        printf("  fg [n]            bring job n to foreground\n");
        printf("  bg [n]            resume job n in background\n");
        printf("  export VAR=val    set environment variable\n");
        printf("  unset VAR         unset environment variable\n");
        printf("  history           show command history\n");
        printf("  !!                run last command\n");
        printf("  help              show this help\n");
        printf("\n");
        printf("FEATURES:\n");
        printf("  cmd | cmd2        pipe output\n");
        printf("  cmd > file        redirect output\n");
        printf("  cmd >> file       append output\n");
        printf("  cmd < file        redirect input\n");
        printf("  cmd1 ; cmd2       run multiple commands\n");
        printf("  cmd &             run in background\n");
        printf("  $VAR              environment variable\n");
        printf("  UP/DOWN arrows    navigate history\n");
        printf("\n");
        return 1;
    }
//-------------------------------------------------------------------------------
    return 0;
}