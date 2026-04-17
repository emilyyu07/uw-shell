#ifndef COMMAND_H
#define COMMAND_H

typedef struct Command {
    char **argv;
    struct Command *next; // for the next command in pipeline
} Command;

#endif
