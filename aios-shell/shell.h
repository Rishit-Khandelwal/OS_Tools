#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 1024
#define MAX_ARGS  50    

#define MAX_JOBS 50
extern pid_t shell_pgid;


typedef enum { 
    RUNNING,   // = 0
    STOPPED,   // = 1  
    DONE       // = 2
} JobStatus;

typedef struct {
    pid_t     pid;
    JobStatus status;
    char      cmd[MAX_INPUT];
} Job;

typedef struct {
    char *infile;
    char *outfile;
    int   append;
} Redirect;

// parser.c
void parser_input(char *input, char **args);
int  parse_all_pipes(char **args, char **cmds[], int max);
int parse_redirects(char **args, Redirect *r);
int parse_multicommands(char **args , char ***cmds);
void parse_env_vars(char **args);

// builtins.c
int  builtin_command(char **args);

// execute.c
void execute_redirect(char **args, Redirect *r);
void execute_command(char **args);
void execute_pipeline(char **cmds[], int count);


// jobs.c
extern Job jobs[];
extern int job_count;
void cleanup_jobs();

// jobs.c
void add_job(pid_t pid, char *cmd);
void remove_job(pid_t pid);
void list_jobs();
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
#endif
