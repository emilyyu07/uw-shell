#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char **parse_line(char *line);

char **parse_line(char *line)
{
    // tokenize input
    char **argvc = malloc(sizeof(char *) * 64);
    int i = 0;

    char *token = strtok(line, " ");

    // token loop to fill argv array with command and arguments
    while (token != NULL)
    {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
    return argv;
}