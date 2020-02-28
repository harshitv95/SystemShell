/**
 * Command Shell
 * 
 * @author Harshit Vadodaria (harshitv95@gmail.com)
 * @version 1.0
 * */


#ifndef _PARSER
#define _PARSER

extern int mysh_debug;

typedef struct command Command;

struct command
{
    char *command_file; // Absolute path of file
    char **tokens;      // All space-separated tokens (including the command itself)
    int in_stream;      // Input Stream / Input File Descriptor (file or pipe)
    int out_stream;     // Output Stream / Output File Descriptor (file or pipe)
    int err_stream;     // Error Stream / Error File Descriptor (file or pipe)
    Command *next;      // Pointer to next command; tip: Last command in LL will have a NULL pointer for next
    char *__command_str; // Entire command as typed into the console (just 1 single command, not all pipe-separated commands)
};

#endif