#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "shell.h"

Job jobs[MAX_JOBS];    // global job table
int job_count = 0;

void add_job(pid_t pid, char *cmd) {
    jobs[job_count].pid    = pid;
    jobs[job_count].status = RUNNING;
    strncpy(jobs[job_count].cmd, cmd, MAX_INPUT);
    job_count++;
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            // shift everything left
            for (int j = i; j < job_count - 1; j++)
                jobs[j] = jobs[j+1];
            job_count--;
            return;
        }
    }
}

void list_jobs() {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %s \t %s\n",
            i+1,
            jobs[i].status == RUNNING ? "Running" :
            jobs[i].status == STOPPED ? "Stopped" : "Done",
            jobs[i].cmd);
    }
}

void cleanup_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].status == DONE)
            printf("[%d] Done \t %s\n", i+1, jobs[i].cmd);
    }
    // then remove done jobs
    int j = 0;
    for (int i = 0; i < job_count; i++)
        if (jobs[i].status != DONE)
            jobs[j++] = jobs[i];
    job_count = j;
}

// called when Ctrl+Z pressed — stops foreground child
void sigtstp_handler(int sig) {
    // shell itself ignores SIGTSTP
    // child handles it via SIG_DFL
}

//  just mark DONE silently
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid)
                jobs[i].status = DONE;  // silent — no printf here
        }
    }
}