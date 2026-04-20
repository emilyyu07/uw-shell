// Loop and orchestration of the shell, including prompt display, input reading, and command execution.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "builtins.h"
#include "executor.h"
#include "parser.h"
#include "shell.h"

typedef enum {
    OP_NONE,
    OP_AND,
    OP_OR
} ListOp;

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
    return s;
}

static int append_chunk(char **out, size_t *len, size_t *cap, const char *chunk, size_t n)
{
    if (*len + n + 1 > *cap) {
        size_t new_cap = (*cap == 0) ? 64 : *cap;
        while (*len + n + 1 > new_cap) {
            new_cap *= 2;
        }
        char *new_out = realloc(*out, new_cap);
        if (!new_out) {
            return 0;
        }
        *out = new_out;
        *cap = new_cap;
    }
    memcpy(*out + *len, chunk, n);
    *len += n;
    (*out)[*len] = '\0';
    return 1;
}

// Minimal command substitution for $(...) used in stage 2.
static char *expand_command_substitutions(const char *input)
{
    char *out = NULL;
    size_t out_len = 0;
    size_t out_cap = 0;

    for (size_t i = 0; input[i] != '\0'; ) {
        if (input[i] == '$' && input[i + 1] == '(') {
            size_t j = i + 2;
            int depth = 1;
            while (input[j] != '\0' && depth > 0) {
                if (input[j] == '(') {
                    depth++;
                } else if (input[j] == ')') {
                    depth--;
                }
                j++;
            }

            if (depth != 0) {
                free(out);
                return NULL;
            }

            size_t inner_len = (j - 1) - (i + 2);
            char *inner = malloc(inner_len + 1);
            if (!inner) {
                free(out);
                return NULL;
            }
            memcpy(inner, input + i + 2, inner_len);
            inner[inner_len] = '\0';

            FILE *fp = popen(inner, "r");
            free(inner);
            if (!fp) {
                free(out);
                return NULL;
            }

            char buf[256];
            char *captured = NULL;
            size_t cap_len = 0;
            size_t cap_cap = 0;

            while (fgets(buf, sizeof(buf), fp)) {
                size_t n = strlen(buf);
                if (!append_chunk(&captured, &cap_len, &cap_cap, buf, n)) {
                    pclose(fp);
                    free(out);
                    free(captured);
                    return NULL;
                }
            }
            pclose(fp);

            while (cap_len > 0 && (captured[cap_len - 1] == '\n' || captured[cap_len - 1] == '\r')) {
                captured[--cap_len] = '\0';
            }

            if (!append_chunk(&out, &out_len, &out_cap, captured ? captured : "", cap_len)) {
                free(out);
                free(captured);
                return NULL;
            }
            free(captured);
            i = j;
            continue;
        }

        if (!append_chunk(&out, &out_len, &out_cap, input + i, 1)) {
            free(out);
            return NULL;
        }
        i++;
    }

    if (!out) {
        out = malloc(1);
        if (!out) {
            return NULL;
        }
        out[0] = '\0';
    }
    return out;
}

static int run_pipeline_string(char *segment)
{
    char *expanded = expand_command_substitutions(segment);
    if (!expanded) {
        return -1;
    }

    Command *cmd = parse_line(expanded);
    if (!cmd) {
        free(expanded);
        return -1;
    }

    // Single builtins must run in parent to affect shell state.
    if (cmd->next == NULL && handle_builtin(cmd)) {
        free_command_list(cmd);
        free(expanded);
        return 0;
    }

    int status = execute_pipeline(cmd);
    free_command_list(cmd);
    free(expanded);
    return status;
}

static int should_run(ListOp prev_op, int last_status)
{
    if (prev_op == OP_AND && last_status != 0) {
        return 0;
    }
    if (prev_op == OP_OR && last_status == 0) {
        return 0;
    }
    return 1;
}

static void run_command_list(char *line)
{
    char *p = line;
    int last_status = 0;
    ListOp prev_op = OP_NONE;

    while (1) {
        char *segment_start = p;
        char *op_ptr = NULL;
        ListOp next_op = OP_NONE;
        int op_len = 0;

        for (; *p; p++) {
            if (*p == ';') {
                op_ptr = p;
                next_op = OP_NONE;
                op_len = 1;
                break;
            }
            if (p[0] == '&' && p[1] == '&') {
                op_ptr = p;
                next_op = OP_AND;
                op_len = 2;
                break;
            }
            if (p[0] == '|' && p[1] == '|') {
                op_ptr = p;
                next_op = OP_OR;
                op_len = 2;
                break;
            }
        }

        if (op_ptr) {
            *op_ptr = '\0';
        }

        char *segment = trim(segment_start);
        if (*segment != '\0' && should_run(prev_op, last_status)) {
            last_status = run_pipeline_string(segment);
            if (last_status < 0) {
                break;
            }
        }

        if (!op_ptr) {
            break;
        }

        p = op_ptr + op_len;
        prev_op = next_op;
    }
}

void run_shell(void)
{
    while (1) {
        fprintf(stderr, "uw-shell> ");
        fflush(stderr);
        tcdrain(STDERR_FILENO);

        char *line = NULL;
        size_t bufsize = 0;
        if (getline(&line, &bufsize, stdin) == -1) {
            printf("\n");
            free(line);
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        run_command_list(line);
        free(line);
    }
}
