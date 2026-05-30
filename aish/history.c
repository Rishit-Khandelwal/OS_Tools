#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "shell.h"

char *history[MAX_HISTORY];
int   history_count = 0;

void add_history(char *input) {
    if (strlen(input) == 0) return;
    
    // don't add duplicate of last command
    if (history_count > 0 && 
        strcmp(history[history_count-1], input) == 0) return;

    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(input);  // copy string
    } else {
        // buffer full — shift everything left, drop oldest
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY-1; i++)
            history[i] = history[i+1];
        history[MAX_HISTORY-1] = strdup(input);
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i+1, history[i]);
}

char *get_history(int idx) {
    if (idx < 0 || idx >= history_count) return NULL;
    return history[idx];
}

void free_history() {
    for (int i = 0; i < history_count; i++)
        free(history[i]);
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);   // disable line buffering + echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

// main.c — replace read() with arrow-aware input
char *read_input(char *buf, int max) {
    struct termios orig;
    tcgetattr(STDIN_FILENO, &orig);
    enable_raw_mode();

    int pos = 0;
    int hist_idx = history_count;   // start past end

    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == '\n' || c == '\r') {
            buf[pos] = '\0';
            write(STDOUT_FILENO, "\n", 1);
            break;
        }
        else if (c == 4) {          // Ctrl+D
            disable_raw_mode(&orig);
            return NULL;
        }
        else if (c == 127) {        // Backspace
            if (pos > 0) {
                pos--;
                write(STDOUT_FILENO, "\b \b", 3);
            }
        }
        else if (c == '\033') {     // escape sequence
            char seq[2];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);
            if (seq[0] == '[') {
                if (seq[1] == 'A') {        // UP arrow
                    if (hist_idx > 0) {
                        hist_idx--;
                        char *h = get_history(hist_idx);
                        // clear current line
                        while (pos--) write(STDOUT_FILENO, "\b \b", 3);
                        pos = 0;
                        // print history entry
                        write(STDOUT_FILENO, h, strlen(h));
                        strncpy(buf, h, max);
                        pos = strlen(h);
                    }
                }
                else if (seq[1] == 'B') {   // DOWN arrow
                    if (hist_idx < history_count - 1) {
                        hist_idx++;
                        char *h = get_history(hist_idx);
                        while (pos--) write(STDOUT_FILENO, "\b \b", 3);
                        pos = 0;
                        write(STDOUT_FILENO, h, strlen(h));
                        strncpy(buf, h, max);
                        pos = strlen(h);
                    } else {
                        // past end — clear line
                        hist_idx = history_count;
                        while (pos--) write(STDOUT_FILENO, "\b \b", 3);
                        pos = 0;
                        buf[0] = '\0';
                    }
                }
            }
        }
        else {
            // normal character
            buf[pos++] = c;
            write(STDOUT_FILENO, &c, 1);
        }
    }

    disable_raw_mode(&orig);
    return buf;
}