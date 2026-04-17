// tokenize user input into an array of strings (argv) for execvp
/*
Important personal notes:
- commands are structured objects with properties like name, args, etc.

Syntax note:
cmd->argv mens (*cmd).argv, which is the argv field of the Command struct pointed to by cmd

Current implementation assumes well-formed input.
*/

#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "parser.h"


#define MAX_ARGS 64

void free_command_list(Command *head)
{
    Command *current = head;
    while (current != NULL){
        Command *next = current->next;
        free(current->argv);
        free(current);
        current = next;
    }

}

Command *parse_single_command(char *line){

    Command *cmd = malloc(sizeof(Command));

    if (cmd == NULL)
    {
        return NULL;
    }
    
    cmd->next = NULL; // initialize next pointer to NULL for single command
    cmd->argv = NULL; // initialize argv to NULL before allocation

    char **argv = malloc(sizeof(char *) * MAX_ARGS);

    if (argv == NULL)
    {
        free(cmd);
        return NULL;
    }

    cmd->argv = argv;
    
    int i = 0;
    char *saveptr; 
    char *token = strtok_r(line, " \t", &saveptr);

    while (token != NULL && i < MAX_ARGS - 1)
    {
        argv[i++] = token;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    // empty command segment, free
    if (i==0){
        free(cmd->argv);
        free(cmd);
        return NULL;
    }
    
    argv[i] = NULL;
    return cmd;

}

Command *parse_line(char *line)
{
    Command *head = NULL;
    Command *prev = NULL;

    char *saveptr;
    
    char *segment = strtok_r(line,"|", &saveptr);

    while (segment!=NULL){
        Command *cmd = parse_single_command(segment);

        // if parsing fails, free any previously allocated commands 
        if (cmd==NULL){
            free_command_list(head);
            return NULL;
        }

        if (head == NULL){
            head = cmd;
        }
        else{
            prev->next = cmd;
        }

        prev = cmd;

        segment = strtok_r(NULL,"|", &saveptr);
    }

    return head;

}
