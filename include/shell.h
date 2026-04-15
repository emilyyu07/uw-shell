#ifndef SHELL_H
#define SHELL_H

typedef struct {
    char **argv;
} Command;

void run_shell(void);

#endif
