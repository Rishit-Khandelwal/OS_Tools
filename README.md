# OS_Tools 🛠️

A collection of low-level system programming tools built in C, exploring core OS concepts hands-on.

> ⚠️ **Note:** This repository is a personal learning project and is not intended for production use. More tools will be added over time.

---

## Projects

### AiSH — A Unix Shell
> A fully functional Unix shell built from scratch in C

AiSH (AI Shell) is a custom Unix shell that implements the core features you'd expect from any real shell — pipes, redirection, job control, signal handling, and more. Built as a deep dive into how shells actually work under the hood.

#### Features
```
Command Execution       run any system command
Pipes                   ls | grep a | sort | wc -l
Redirection             cmd > file, cmd >> file, cmd < file
Multiple Commands       ls ; pwd ; whoami
Background Jobs         sleep 99 &
Job Control             jobs, fg, bg
Signal Handling         Ctrl+C kills child, not shell
Ctrl+Z                  stop foreground job, resume with fg/bg
Environment Variables   echo $HOME, export VAR=val, unset VAR
Command History         history, !!, UP/DOWN arrow keys
Built-in Commands       cd, exit, help, jobs, fg, bg, export, unset
Process Groups          terminal ownership handled correctly
```

#### File Structure
```
aish/
├── main.c        — shell loop, signal setup, terminal ownership
├── parser.c      — tokenizer, pipe/redirect/multicommand detection
├── execute.c     — fork/exec, redirection, background jobs
├── builtins.c    — cd, exit, jobs, fg, bg, export, unset, help
├── jobs.c        — job table, sigchld handler, cleanup
├── history.c     — command history, arrow key input, raw mode
├── shell.h       — shared types and declarations
└── Makefile      — build system
```

#### Build & Run
```bash
git clone https://github.com/Rishit-Khandelwal/OS_Tools.git
cd OS_Tools/aish
make
./aish
```

#### Usage
```bash
# basic commands
AiSH$ ls -la
AiSH$ pwd

# pipes — single and chained
AiSH$ ls | grep main
AiSH$ cat main.c | grep int | sort | uniq

# redirection
AiSH$ ls > out.txt
AiSH$ cat < out.txt
AiSH$ echo hello >> out.txt

# multiple commands
AiSH$ ls ; pwd ; whoami

# background jobs
AiSH$ sleep 10 &
AiSH$ jobs
AiSH$ fg 1
AiSH$ bg 1

# environment variables
AiSH$ export NAME=rishit
AiSH$ echo $NAME
AiSH$ unset NAME

# history
AiSH$ history
AiSH$ !!
# UP/DOWN arrows to navigate history

# help
AiSH$ help
```

#### OS Concepts Covered
```
Process Creation        fork(), exec(), wait()
File Descriptors        dup2(), pipe(), open()
Signal Handling         SIGINT, SIGTSTP, SIGCHLD, SIGTTOU
Process Groups          setpgid(), tcsetpgrp()
Terminal Control        raw mode, escape sequences
Job Control             foreground/background process management
Environment             getenv(), setenv(), unsetenv()
```

---

## Roadmap
- [x] AiSH — Unix Shell
- [ ] More tools coming soon...

---

## License
MIT
