// tokenize user input into argv plus redirection metadata

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "parser.h"

static char *trim_left(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

// Parse one token that may include an inline redirection target:
//   >foo, 2>foo, <in, 3</dev/null, >>log, <>file, 1>&2, 0<&-, etc.
// Returns 1 if token is a redirection token. If target is not embedded,
// caller must consume the next token as target.
static int parse_redir_token(const char *tok, Redirection *r)
{
    const char *p = tok;
    int fd = -1;

    if (isdigit((unsigned char)p[0])) {
        fd = p[0] - '0';
        p++;
    }

    r->target = NULL;

    if (p[0] == '<' && p[1] == '&') {
        r->fd = (fd < 0) ? 0 : fd;
        if (p[2] == '-') {
            r->op = REDIR_CLOSE_IN;
            r->target = (char *)(p + 2);
            return 1;
        }
        if (p[2] != '\0') {
            r->op = REDIR_DUP_IN;
            r->target = (char *)(p + 2);
            return 1;
        }
        r->op = REDIR_DUP_IN;
        return 1;
    }

    if (p[0] == '>' && p[1] == '&') {
        r->fd = (fd < 0) ? 1 : fd;
        if (p[2] == '-') {
            r->op = REDIR_CLOSE_OUT;
            r->target = (char *)(p + 2);
            return 1;
        }
        if (p[2] != '\0') {
            r->op = REDIR_DUP_OUT;
            r->target = (char *)(p + 2);
            return 1;
        }
        r->op = REDIR_DUP_OUT;
        return 1;
    }

    if (p[0] == '<' && p[1] == '>' ) {
        r->fd = (fd < 0) ? 0 : fd;
        r->op = REDIR_RDWR;
        if (p[2] != '\0') {
            r->target = (char *)(p + 2);
        }
        return 1;
    }

    if (p[0] == '>' && p[1] == '>') {
        r->fd = (fd < 0) ? 1 : fd;
        r->op = REDIR_APPEND;
        if (p[2] != '\0') {
            r->target = (char *)(p + 2);
        }
        return 1;
    }

    if (p[0] == '<') {
        r->fd = (fd < 0) ? 0 : fd;
        r->op = REDIR_IN;
        if (p[1] != '\0') {
            r->target = (char *)(p + 1);
        }
        return 1;
    }

    if (p[0] == '>') {
        r->fd = (fd < 0) ? 1 : fd;
        r->op = REDIR_OUT;
        if (p[1] != '\0') {
            r->target = (char *)(p + 1);
        }
        return 1;
    }

    return 0;
}

void free_command_list(Command *head)
{
    while (head) {
        Command *next = head->next;
        free(head->argv);
        free(head);
        head = next;
    }
}

static Command *parse_single_command(char *segment)
{
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) {
        return NULL;
    }

    cmd->next = NULL;
    cmd->negate = 0;
    cmd->redir_count = 0;

    cmd->argv = malloc(sizeof(char *) * MAX_ARGS);
    if (!cmd->argv) {
        free(cmd);
        return NULL;
    }

    char *tokens[MAX_ARGS];
    int ntok = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(segment, " \t", &saveptr);
    while (tok && ntok < MAX_ARGS - 1) {
        tokens[ntok++] = tok;
        tok = strtok_r(NULL, " \t", &saveptr);
    }

    int argc = 0;
    for (int i = 0; i < ntok; i++) {
        Redirection r;
        if (parse_redir_token(tokens[i], &r)) {
            if (r.target == NULL) {
                if (i + 1 >= ntok) {
                    free(cmd->argv);
                    free(cmd);
                    return NULL;
                }
                r.target = tokens[++i];
            }
            if (cmd->redir_count >= MAX_REDIRS) {
                free(cmd->argv);
                free(cmd);
                return NULL;
            }
            cmd->redirs[cmd->redir_count++] = r;
            continue;
        }

        if (strcmp(tokens[i], "!") == 0) {
            free(cmd->argv);
            free(cmd);
            return NULL;
        }

        cmd->argv[argc++] = tokens[i];
    }

    if (argc == 0) {
        free(cmd->argv);
        free(cmd);
        return NULL;
    }

    cmd->argv[argc] = NULL;
    return cmd;
}

Command *parse_line(char *line)
{
    Command *head = NULL;
    Command *prev = NULL;

    char *cursor = trim_left(line);
    int negate = 0;

    // '!' is only valid at start of a pipeline.
    if (*cursor == '!') {
        char next = cursor[1];
        if (next == '\0' || next == ' ' || next == '\t') {
            negate = 1;
            cursor = trim_left(cursor + 1);
        }
    }

    char *saveptr = NULL;
    char *segment = strtok_r(cursor, "|", &saveptr);
    while (segment) {
        Command *cmd = parse_single_command(trim_left(segment));
        if (!cmd) {
            free_command_list(head);
            return NULL;
        }

        if (!head) {
            head = cmd;
        } else {
            prev->next = cmd;
        }
        prev = cmd;
        segment = strtok_r(NULL, "|", &saveptr);
    }

    if (!head) {
        return NULL;
    }

    head->negate = negate;
    return head;
}
