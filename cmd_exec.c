#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "parser.h"
#include "cmd_exec.h"

void exec_child(Command *command, int input_stream, int output_stream, int error_stream) {
    int exec_ret_val;
    char* const* args = command->tokens;

    // Setting appropriate input, output and error streams
    if (input_stream != -1)
        dup2(input_stream, STDIN_FILENO);
    if (output_stream  -1)
        dup2(output_stream, STDOUT_FILENO);
    if (error_stream != -1)
        dup2(error_stream, STDOUT_FILENO);
    
    exec_ret_val = execvp(command->command_file, args);
    if (exec_ret_val == -1) {
        perror("[ERROR : exec] While running child process");
        exit(EXIT_FAILURE);
    }
}

int wait_for_child() {
    return -1;
}

int create_process() {
    int pid = fork();
    if (pid == -1) {
        perror("[ERROR : fork] While creating child process");
        exit(EXIT_FAILURE);
    }
    return pid;
}

void create_pipes(int num_commands) {
    
}

int run(Command *command, int num_commands) {
    if (num_commands > 1) {

    }
    while (command != NULL) {
        printf("%s\n\n", command->command_file);
        command = command->next;
    }
    // if (create_process() == 0) { // is child
    //     if (num_commands == 0) exec_child(command, -1, -1, -1);
    // }
    return 1;
}