#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_command(char **argv);

void execute_command(char **argv)
{
    // fork process creation
    pid_t pid = fork();

    // if process id fails
    if (pid < 0)
    {
        perror("fork");
        continue;
    }

    // child process executes command
    if (pid == 0)
    {
        // replace child process w program, start execution
        execvp(argv[0], argv);

        // if execvp returns, there was an error
        perror(argv[0]);
        _exit(1);
    }
    else
    {
        // parent process waits for child to finish before continuing loop
        waitpid(pid, NULL, 0);
    }
}