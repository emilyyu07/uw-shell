#ifndef PARSER_H
#define PARSER_H

#include "command.h"

Command *parse_line(char *line);
void free_command_list(Command *head);

#endif
