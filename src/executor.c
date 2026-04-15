// fork, exec, and wait for command execution
// interprets command structures and manages process creation and execution flow

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "executor.h"
#include "command.h"

void execute_command(Command *cmd)
{
    // fork process creation
    pid_t pid = fork();

    // if process id fails
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    // child process executes command
    if (pid == 0)
    {
        // replace child process w program, start execution
        execvp(cmd->argv[0], cmd->argv);

        // if execvp returns, there was an error
        perror(cmd->argv[0]);
        _exit(1);
    }
    else
    {
        // parent process waits for child to finish before continuing loop
        waitpid(pid, NULL, 0);
    }
}
