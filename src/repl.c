// Loop and orchestration of the shell, including prompt display, input reading, and command execution

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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
        char **argv = parse_line(line);
        if (argv == NULL)
        {
            perror("parse_line");
            free(line);
            continue;
        }

        if (argv[0] == NULL)
        {
            // empty input, skip iteration
            free(argv);
            free(line);
            continue;
        }

        // built-in commands, executed by the shell itself (not child processes)
        if (handle_builtin(argv))
        {
            free(argv);
            free(line);
            continue;
        }

        // execute non-built-in command
        execute_command(argv);

        free(argv);
        // clean up memory allocated by getline for next iteration
        free(line);
    }
}
