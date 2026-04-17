/*
cd, exit, and other built-in commands that are handled directly by the shell 
(run in parent process) without forking a child process
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "command.h"

int handle_builtin(Command *cmd)
{
    // check if command is exit
    if (strcmp(cmd->argv[0], "exit") == 0)
    {
        exit(cmd->argv[1] ? atoi(cmd->argv[1]) : 0);
    }

    // check if command is cd
    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        if (cmd->argv[1] == NULL)
        {
            chdir(getenv("HOME"));
        }
        else if (chdir(cmd->argv[1]) != 0)
        {
            perror("cd");
        }

        return 1;
    }

    return 0;
}
