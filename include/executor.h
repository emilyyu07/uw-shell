#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"
void execute_command(Command *cmd);
int execute_pipeline(Command *head);

#endif
