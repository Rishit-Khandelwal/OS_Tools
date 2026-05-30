CC = gcc
CFLAGS = -Wall -g
SRCS = main.c parser.c builtins.c execute.c jobs.c history.c
TARGET = aish

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)