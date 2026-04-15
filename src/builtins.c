/*
cd, exit, and other built-in commands that are handled directly by the shell 
(run in parent process) without forking a child process
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"

int handle_builtin(char **argv)
{
    // check if command is exit
    if (strcmp(argv[0], "exit") == 0)
    {
        exit(argv[1] ? atoi(argv[1]) : 0);
    }

    // check if command in cd
    if (strcmp(argv[0], "cd") == 0)
    {
        if (argv[1] == NULL)
        {
            chdir(getenv("HOME"));
        }
        else if (chdir(argv[1]) != 0)
        {
            perror("cd");
        }

        return 1;
    }

    return 0;
}
