// fork, exec, and wait for command execution

#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "command.h"
#include "executor.h"

static void apply_redirections(Command *cmd)
{
    for (int i = 0; i < cmd->redir_count; i++) {
        Redirection *r = &cmd->redirs[i];
        int fd = -1;

        switch (r->op) {
        case REDIR_IN:
            fd = open(r->target, O_RDONLY);
            if (fd < 0) {
                perror(r->target);
                _exit(1);
            }
            if (dup2(fd, r->fd) < 0) {
                perror("dup2");
                _exit(1);
            }
            close(fd);
            break;
        case REDIR_OUT:
            fd = open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(r->target);
                _exit(1);
            }
            if (dup2(fd, r->fd) < 0) {
                perror("dup2");
                _exit(1);
            }
            close(fd);
            break;
        case REDIR_APPEND:
            fd = open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror(r->target);
                _exit(1);
            }
            if (dup2(fd, r->fd) < 0) {
                perror("dup2");
                _exit(1);
            }
            close(fd);
            break;
        case REDIR_RDWR:
            fd = open(r->target, O_RDWR | O_CREAT, 0644);
            if (fd < 0) {
                perror(r->target);
                _exit(1);
            }
            if (dup2(fd, r->fd) < 0) {
                perror("dup2");
                _exit(1);
            }
            close(fd);
            break;
        case REDIR_DUP_OUT:
        case REDIR_DUP_IN:
            if (dup2(atoi(r->target), r->fd) < 0) {
                perror("dup2");
                _exit(1);
            }
            break;
        case REDIR_CLOSE_OUT:
        case REDIR_CLOSE_IN:
            if (close(r->fd) < 0) {
                perror("close");
                _exit(1);
            }
            break;
        }
    }
}

static void close_extraneous_fds(Command *cmd)
{
    long max_fd = sysconf(_SC_OPEN_MAX);
    if (max_fd < 0) {
        max_fd = 256;
    }

    char *keep = calloc((size_t)max_fd, 1);
    if (!keep) {
        perror("calloc");
        _exit(1);
    }

    if (max_fd > 0) keep[0] = 1;
    if (max_fd > 1) keep[1] = 1;
    if (max_fd > 2) keep[2] = 1;

    for (int i = 0; i < cmd->redir_count; i++) {
        Redirection *r = &cmd->redirs[i];
        if (r->fd < 0 || r->fd >= max_fd) {
            continue;
        }
        if (r->op == REDIR_CLOSE_IN || r->op == REDIR_CLOSE_OUT) {
            keep[r->fd] = 0;
        } else {
            keep[r->fd] = 1;
        }
    }

    for (int fd = 0; fd < max_fd; fd++) {
        if (!keep[fd]) {
            close(fd);
        }
    }

    free(keep);
}

void execute_command(Command *cmd)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        apply_redirections(cmd);
        close_extraneous_fds(cmd);
        if (handle_builtin(cmd)) {
            _exit(0);
        }
        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        _exit(1);
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
}

int execute_pipeline(Command *head)
{
    int count = 0;
    for (Command *c = head; c; c = c->next) {
        count++;
    }
    if (count == 0) {
        return 1;
    }

    pid_t *pids = malloc(sizeof(pid_t) * (size_t)count);
    if (!pids) {
        perror("malloc");
        return 1;
    }

    int prev_rd = -1;
    Command *cmd = head;

    for (int i = 0; i < count; i++, cmd = cmd->next) {
        int pfd[2];
        int has_next = (cmd->next != NULL);

        if (has_next && pipe(pfd) < 0) {
            perror("pipe");
            free(pids);
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            return 1;
        }

        if (pid == 0) {
            if (prev_rd != -1) {
                if (dup2(prev_rd, STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            if (has_next) {
                if (dup2(pfd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            if (prev_rd != -1) {
                close(prev_rd);
            }
            if (has_next) {
                close(pfd[0]);
                close(pfd[1]);
            }

            apply_redirections(cmd);
            close_extraneous_fds(cmd);

            if (handle_builtin(cmd)) {
                _exit(0);
            }

            execvp(cmd->argv[0], cmd->argv);
            perror(cmd->argv[0]);
            _exit(1);
        }

        pids[i] = pid;

        if (prev_rd != -1) {
            close(prev_rd);
        }
        if (has_next) {
            close(pfd[1]);
            prev_rd = pfd[0];
        } else {
            prev_rd = -1;
        }
    }

    int last_status = 0;
    pid_t last_pid = pids[count - 1];
    for (int i = 0; i < count; i++) {
        int status = 0;
        pid_t done;
        do {
            done = waitpid(pids[i], &status, 0);
        } while (done < 0 && errno == EINTR);
        if (done < 0) {
            perror("waitpid");
            continue;
        }
        if (done == last_pid) {
            last_status = status;
        }
    }
    free(pids);

    int exit_status = 1;
    if (WIFEXITED(last_status)) {
        exit_status = WEXITSTATUS(last_status);
    }

    return head->negate ? !exit_status : exit_status;
}
