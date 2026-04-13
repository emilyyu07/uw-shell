#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define MAX_ARGS 64

char **parse_line(char *line)
{
    char **argv = malloc(sizeof(char *) * MAX_ARGS);
    if (argv == NULL)
    {
        return NULL;
    }

    int i = 0;

    char *token = strtok(line, " ");

    while (token != NULL && i < MAX_ARGS - 1)
    {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
    return argv;
}
