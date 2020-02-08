#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "parser.h"
#include "cmd_exec.h"

//limits
#define MAX_TOKENS 10
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;

// builtin commands
#define EXIT_STR "exit"
// #define EXIT_CMD 0
// #define VALID_CMD 1
#define CMD_SYNTAX_ERR 98
#define UNKNOWN_CMD 99

// Configuration constants
#define CMD_DIR "/bin/"

FILE *fp; // file struct for stdin
char **tokens;
char *line;

Command *command;
int num_commands;
int do_exit = 0;

void initialize()
{

	// allocate space for the whole line
	assert((line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// open stdin as a file pointer
	assert((fp = fdopen(STDIN_FILENO, "r")) != NULL);
}

// Returns absolute location of command
char *get_command_file(char *cmd)
{
	int len_cmd = (strlen(cmd)) + (strlen(CMD_DIR));
	char *cmd_file = (char *)malloc(len_cmd); // To accomodate command + 5 characters of parent directory
	if (access(strcat(strcat(cmd_file, CMD_DIR), cmd), F_OK) != -1)
	{
		// File exists
		return cmd_file;
	}

	return NULL;
}

static int print_error(int err_code, Command *command, char* error_msg) {
	switch (err_code)
	{
	case CMD_SYNTAX_ERR:
		printf("Syntax error in command [%s], %s", command->__command_str, error_msg);
		break;
	
	default:
		printf("Error in command [%s], %s", command->__command_str, error_msg);
		break;
	}
	return 0;
}

int redirect_io(Command *command)
{
	int is_filename = 0;
	char **sep, *filename;

	// Pointer to start of input file
	filename = strstr(command->__command_str, "<");

	if (filename != NULL)
	{ // '<' character found, need to read input from file
		if (strlen(filename) <= 1)
		{
			return print_error(CMD_SYNTAX_ERR, command, "Input filename expected after'<'");
		} else if (count_occurences(filename, '<') > 1) {
			return print_error(CMD_SYNTAX_ERR, command, "Expected 1 filename after '<', found another '<'");
		} else {
			filename++;
		}
	}
	else
	{
		// No input file provided in command, let command take input from STDIN
		command->in_stream = STDIN_FILENO;
	}

	while ((sep = strsep(command->__command_str, "<")) != NULL)
	{
		is_filename++;
		*filename = *sep;
	}
	if (is_filename)
	{
	}
}

Command *tokenize(char *string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_command;
	char *this_token;

	// allocate space for individual tokens
	assert((tokens = malloc(sizeof(char *) * MAX_TOKENS)) != NULL);

	// Intialize structure for Command
	Command *command = NULL;

	// Split the command string using the Filter / Pipe "|" operator
	// This will give us different commands
	while ((this_command = strsep(&string, "|")) != NULL &&
		   !(do_exit = (strcmp(trim_whitespace(this_command), EXIT_STR) == 0)))
	{

		if (*(trim_whitespace(this_command)) == '\0')
			continue;

		printf("Command [%d]\n", ++num_commands);
		command = (Command *)malloc(sizeof(Command));
		command->__command_str = this_command;
		token_count = 0;

		// Split the command using whitespaces, to obtain tokens
		while (((this_token = strsep(&this_command, " \t\v\f\n\r")) != NULL))
		{

			if (*this_token == '\0')
				continue;

			if (token_count == 0)
			{
				// Get absolute path of command's binary file
				if (!(command->command_file = get_command_file(this_token)))
				{
					printf("Command [%s] not found\n", this_token);
					return NULL;
				}
			}

			if (*this_token == '|')
			{
				tokens[token_count] = NULL;
				command->tokens = tokens;
				command->next = tokenize(string);
				return command;
			}

			tokens[token_count] = this_token;

			printf("Token %d: %s\n", token_count, tokens[token_count]);

			token_count++;

			// if there are more tokens than space ,reallocate more space
			if (token_count >= size)
			{
				size *= 2;

				assert((tokens = realloc(tokens, sizeof(char *) * size)) != NULL);
			}
		}
		command->next = tokenize(string);
	}

	if (command != NULL)
	{
		tokens[token_count] = NULL;
		command->tokens = tokens;
		command->next = NULL;
	}
	return command;
}

void read_command()
{

	// getline will reallocate if input exceeds max length
	assert(getline(&line, &MAX_LINE_LEN, fp) > -1);

	printf("Shell read this line: %s", line);

	command = tokenize(line);
}

int run_command()
{

	// if (strcmp( command->tokens[0], EXIT_STR ) == 0)
	// 	return EXIT_CMD;
	// int len_cmd = (strlen(command->tokens[0])) + (strlen(CMD_DIR));
	// command->command_file = (char*) malloc(len_cmd); // To accomodate command + 5 characters of parent directory
	// if( access( strcat( strcat(command->command_file, CMD_DIR), command->tokens[0] ), F_OK ) != -1 ) {
	//     // File exists

	return run(command, num_commands);

	// 	return VALID_CMD;
	// } else {

	// }

	// return UNKNOWN_CMD;
}

// static int command_validator(int cmd) {
//     if (cmd == UNKNOWN_CMD)
//         printf("Command [%s] not found\n", tokens[0]);

//     return cmd;
// }

int main()
{
	initialize();

	do
	{
		num_commands = 0;
		printf("mysh> ");
		read_command();
		run_command();

	} while (!do_exit);

	return 0;
}
