#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void run_shell();

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

        // tokenize input (insert here)

        if (argv[0] == NULL)
        {
            // empty input, skip iteration
            free(line);
            continue;
        }

        // built-in commands, executed by the shell itself (not child processes) ------------

        // check if command is exit
        if (strcmp(argv[0], "exit") == 0)
        {
            free(line);
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

            free(line);
            continue;
        }

        // ---------------------------------------

        // clean up memory allocated by getline for next iteration
        free(line);
    }
}