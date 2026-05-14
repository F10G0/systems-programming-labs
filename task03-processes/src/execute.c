#define _GNU_SOURCE

#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

// Pipe endpoint indices
#define PIPE_READ 0
#define PIPE_WRITE 1

// Execute one parsed pipeline
int run_pipeline(struct pipeline *p) {
    // Empty pipeline: nothing to execute
    if (!p || p->first_command.argc == 0)
        return 0;

    // Store child process IDs for later waiting
    pid_t processes[1024];

    // One pipe per command stage
    int pipes[1024][2];

    // Number of spawned child processes
    int pcnt = 0;

    // Iterate through all commands in the pipeline
    for (struct command *cmd = &p->first_command; cmd; cmd = cmd->next) {

        // Create a pipe for the current command
        // O_CLOEXEC ensures descriptors are closed after execvp()
        if (pipe2(pipes[pcnt], O_CLOEXEC) < 0)
            return 1;

        // Create child process
        pid_t pid = fork();

        if (pid == -1)
            return 1;

        // Child process
        if (pid == 0) {

            // Handle stdin redirection from file
            if (cmd->input_redir) {
                int fd = open(cmd->input_redir, O_RDONLY);

                if (fd < 0)
                    exit(1);

                dup2(fd, STDIN_FILENO);
                close(fd);

            // Otherwise connect stdin to previous pipe
            } else if (pcnt) {
                dup2(pipes[pcnt - 1][PIPE_READ], STDIN_FILENO);
            }

            // Handle stdout redirection to file
            if (cmd->output_redir) {
                int fd = open(
                    cmd->output_redir,
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644
                );

                if (fd < 0)
                    exit(1);

                dup2(fd, STDOUT_FILENO);
                close(fd);

            // Otherwise connect stdout to next pipe
            } else if (cmd->next) {
                dup2(pipes[pcnt][PIPE_WRITE], STDOUT_FILENO);
            }

            // Replace current process image with executable
            execvp(cmd->argv[0], cmd->argv);

            // execvp only returns on failure
            exit(1);
        }

        // Parent process stores child PID
        processes[pcnt++] = pid;
    }

    // Parent no longer needs pipe descriptors
    for (int i = 0; i < pcnt; i++) {
        close(pipes[i][PIPE_READ]);
        close(pipes[i][PIPE_WRITE]);
    }

    // Foreground execution:
    // wait until all child processes terminate
    if (!p->background) {
        for (int i = 0; i < pcnt; i++) {
            int status;

            if (waitpid(processes[i], &status, 0) < 0 || WEXITSTATUS(status)) {
                return 1;
            }
        }
    }

    return 0;
}

// Execute builtin shell commands
int run_builtin(enum builtin_type builtin, char *builtin_arg) {

    switch (builtin) {

        // exit [CODE]
        case BUILTIN_EXIT: {
            int code = 0;

            if (builtin_arg)
                code = atoi(builtin_arg);

            exit(code);
        }

        // wait [PID]
        case BUILTIN_WAIT: {

            // waitpid(-1, ...) waits for any child process
            pid_t pid = -1;

            if (builtin_arg)
                pid = (pid_t)atoi(builtin_arg);

            if (waitpid(pid, NULL, 0) < 0)
                return 1;

            return 0;
        }

        // kill PID
        case BUILTIN_KILL: {

            // Reject missing or obviously invalid PID strings
            if (!builtin_arg || builtin_arg[0] < '0' || builtin_arg[0] > '9') {
                fprintf(stderr,
                        "Invalid builtin_arg for BUILTIN_KILL\n");

                return 1;
            }

            pid_t pid = (pid_t)atoi(builtin_arg);

            // Send SIGTERM to target process
            if (kill(pid, SIGTERM) < 0)
                return 1;

            return 0;
        }

        default:
            return 1;
    }
}
