// tokenize user input into an array of strings (argv) for execvp
/*
Important personal notes:
- commands are structured objects with properties like name, args, etc.

Syntax note:
cmd->argv mens (*cmd).argv, which is the argv field of the Command struct pointed to by cmd
*/

#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "command.h"

#define MAX_ARGS 64

Command *parse_line(char *line)
{

    Command *cmd = malloc(sizeof(Command));
    if (cmd == NULL)
    {
        return NULL;
    }

    char **argv = malloc(sizeof(char *) * MAX_ARGS);
    if (argv == NULL)
    {
        free(cmd);
        return NULL;
    }

    cmd->argv = argv;
    

    int i = 0;
    char *token = strtok(line, " ");

    while (token != NULL && i < MAX_ARGS - 1)
    {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    
    argv[i] = NULL;
    return cmd;
}
