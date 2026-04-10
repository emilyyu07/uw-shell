/*
Personal Notes:
- shell is a loop, creates processes to run commands

Shell execution flow:
Shell -> tokenize -> fork -> Parent: wait for child to finish
Child: execvp to run command -> becomes ls program -> ls program runs/prints output, then exits -> back to shell loop

Flow:
main() -> run_shell() -> while(1) -> user interaction loop -> exit loop -> return to main -> program exits

run_shell() is a REPL loop
- print prompt
- read user input
- tokenize input
- execute via process
- repeat until user exits

High-level flow:
ls -la
1. read line -> "ls -la\n"
2. strip newline → "ls -la"
3. tokenize -> ["ls", "-la", NULL]
4. fork()
    ├── parent -> wait
    └── child -> execvp("ls")
5. child runs ls
6. child exits
7. parent resumes

fork()
- creates a new process by duplicating the calling process
- returns a value in the parent process and a different value in the child process
- parent process receives the PID of the child process
- child process receives 0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void run_shell();

int main()
{
    run_shell();
    return 0;
}

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
        char *argv[64];
        int i = 0;
        char *token = strtok(line, " ");

        // token loop to fill argv array with command and arguments
        while (token != NULL)
        {
            argv[i++] = token;
            token = strtok(NULL, " ");
        }
        argv[i] = NULL;

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

        // fork process creation
        pid_t pid = fork();

        // if process id fails
        if (pid < 0)
        {
            perror("fork failed");
            free(line);
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

        // clean up memory allocated by getline for next iteration
        free(line);
    }
}