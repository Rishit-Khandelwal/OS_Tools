#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    char input[100];
    while (1) {
        printf("AiSH$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Goodbye!\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;  // strip newline

        if (strlen(input) == 0) {
            printf("No command entered.\n");
            continue;
        }

        char *args[50];
        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL && i < 49) {  
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return 1;
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("Execution failed");
            exit(1);
        } else {
            wait(NULL);
        }
    }
}