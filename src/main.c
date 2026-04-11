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
