#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ARGS  64
#define MAX_REDIRS 16

// Redirectors
typedef enum {
    REDIR_IN,        // <
    REDIR_OUT,       // >
    REDIR_APPEND,    // >>
    REDIR_RDWR,      // <>
    REDIR_DUP_IN,    // n<&m
    REDIR_DUP_OUT,   // n>&m
    REDIR_CLOSE_IN,  // n<&-
    REDIR_CLOSE_OUT  // n>&-
} RedirectType;

typedef struct {
    int fd; // file descriptor to redirect (e.g., 0 for stdin, 1 for stdout)
    RedirectType op; // type of redirection
    char *target; // file name or file descriptor
} Redirection;

// Command structure - represent single command and its arguments
typedef struct Command {
    char **argv;
    struct Command *next; // for the next command in pipeline
    int negate; // for handling negation with '!'
    int redir_count;
    Redirection redirs[MAX_REDIRS]; // array of redirections

} Command;

#endif
