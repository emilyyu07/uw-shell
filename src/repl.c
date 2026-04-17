// Loop and orchestration of the shell, including prompt display, input reading, and command execution

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "command.h"
#include "builtins.h"
#include "executor.h"
#include "parser.h"
#include "shell.h"


void run_shell()
{
    while (1)
    {
        // print initial prompt
        printf("uw-shell> ");

        // dynamic shell update
        // char cwd[1024];
        // getcwd(cwd, sizeof(cwd));
        // printf("%s> ", cwd);

        fflush(stdout); // ensure prompt is printed before waiting for input

        // buffer setup for getline
        char *line = NULL;
        size_t bufsize = 0;

        // read user input -> handles EOF
        if (getline(&line, &bufsize, stdin) == -1)
        {
            printf("\n");

            // manually free memory allocated by getline
            free(line);
            break;
        }

        // remove new line character from end of line
        line[strcspn(line, "\n")] = 0;

        // tokenize input
        Command *cmd = parse_line(line);
        if (cmd == NULL)
        {
            perror("parse_line");
            free(line);
            continue;
        }

        if (cmd->argv[0] == NULL)
        {
            // empty input, skip iteration
            free(cmd);
            free(line);
            continue;
        }

        // built-in commands, executed by the shell itself (not child processes)
        if (handle_builtin(cmd))
        {
            free(cmd);
            free(line);
            continue;
        }

        // execute non-built-in command
        execute_command(cmd);

        free(cmd);

        // clean up memory allocated by getline for next iteration
        free(line);
    }
}
