
#ifndef _PARSER
#define _PARSER

typedef struct command Command;

struct command
{
    char *command_file;
    char **tokens;
    int in_stream;
    int out_stream;
    int err_stream;
    Command *next;
    char *__command_str;
};

#endif