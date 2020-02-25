#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "parser.h"
#include "cmd_exec.h"

void exec_child(Command *command, int input_stream, int output_stream, int error_stream)
{
    int exec_ret_val;
    char *const *args = command->tokens;

    // Setting appropriate input, output and error streams
    if (input_stream != -1)
    {
        dup2(input_stream, STDIN_FILENO);
        close(input_stream);
    }
    if (output_stream - 1)
    {
        dup2(output_stream, STDOUT_FILENO);
        close(output_stream);
    }
    if (error_stream != -1)
    {
        dup2(error_stream, STDOUT_FILENO);
        close(error_stream);
    }

    exec_ret_val = execv(command->command_file, args);
    if (exec_ret_val == -1)
    {
        perror("[ERROR : exec] While running child process");
        exit(EXIT_FAILURE);
    }
}

int wait_for_child()
{
    return -1;
}

int create_process()
{
    int pid = fork();
    if (pid < 0)
    {
        perror("[ERROR : fork] While creating child process");
        exit(EXIT_FAILURE);
    }
    return pid;
}

// Creates pipes for IPC between n processes
// Each IPC needs between a pair of 2 process needs a single pair of pipes,
// n processes need a total of n-1 pairs of pipes, or a total of (2*n)-1 pipes in general
void create_pipes(int *pipes)
{
    if (pipe(pipes) == -1)
    {
        perror("[ERROR : pipe] While creating creating pipe");
        exit(EXIT_FAILURE);
    }
}

int run(Command *command, int num_commands)
{
    if (num_commands == 0)
        return 0;
    char *cmd_names[num_commands];
    if (num_commands > 1)
    {
        int pipes[2 * (num_commands - 1)];
        int i;
        for (i = 0; i < num_commands; i++)
        {
            if (i < num_commands - 1)
                create_pipes(&pipes[i * 2]);

            if (create_process() == 0)
            {
                // Child process
                int in_stream, out_stream;

                // [(2*i):read_end, (2*i)+1:write_end] <- pipe
                // Except first process, every process reads from pipe's read end
                // First process might read either from STDIN or File
                in_stream = (i != 0) ? pipes[(i - 1) * 2] : command->in_stream;
                // Except last process, every process writes to pipe's write end
                // Last process might write either to STDOUT or File
                out_stream = (i != num_commands - 1) ? pipes[((i * 2) + 1)] : command->out_stream;

                if (i < num_commands - 1)
                    close(pipes[i * 2]);

                exec_child(command, in_stream, out_stream, -1);
            }
            else
            {
                // Parent process

                // Parent does not need input or output end of pipe created by process i,
                // and neither does the next child i + 1

                if (i > 1)
                    // Closing the read end of the pipe created by process i-2,
                    // since process i only reads inputs from pipe created by process i-1, and thus pipe of i-2 is not needed
                    close(pipes[(i - 2) * 2]);
                if (i < num_commands - 1)
                    // Closing the write end of the pipe created by the current process i, in the parent,
                    // since this pipe is passed into the next child process i+1, which, just like the parent,
                    // does not need the write end of process i's pipe
                    close(pipes[(i * 2) + 1]);

                cmd_names[i] = command->tokens[0];
                // Select next from Linked List of commands
                command = command->next;
            }
        }
    }
    else
    {
        cmd_names[0] = command->tokens[0];
        if (create_process() == 0)
            exec_child(command, command->in_stream, command->out_stream, -1);
        else
        {
        }
    }

    int status, pid;
    int pids[num_commands], statuses[num_commands];
    for (int i = 0; i < num_commands; i++)
    {
        pid = wait(&status);
        pids[i] = pid;
        statuses[i] = status;
    }
    // For the sake of simplicity of output and preventing the output from getting messy,
    // printing the exit statuses and PIDs of all child processes after all the processes have finished
    if (mysh_debug) {
            printf("\n");
        for (int i = 0; i < num_commands; i++)
        {
            printf("Process [%s] with PID [%d] exited with status [%d]\n", cmd_names[i], pids[i], statuses[i]);
        }
    }
    return num_commands;
}