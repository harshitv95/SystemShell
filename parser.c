#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

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
// char **tokens;
char *line;

Command *command;
int num_commands;
int do_exit = 0;
int cmd_valid = 1;
int redir_output = 0;

int mysh_debug = 0;

void initialize(int argc, char **argv)
{

	// allocate space for the whole line
	assert((line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// open stdin as a file pointer
	assert((fp = fdopen(STDIN_FILENO, "r")) != NULL);

	while (argc-- > 0)
	{
		if (strcmp(argv[0], "debug") == 0)
			mysh_debug = 1;
		argv++;
	}
}

// Returns absolute location of command
char *get_command_file(char *cmd)
{
	int len_cmd = (strlen(cmd)) + (strlen(CMD_DIR));
	char *cmd_file = (char *)malloc(len_cmd); // To accomodate command + 5 characters of parent directory
	if (access(strcat(strcpy(cmd_file, CMD_DIR), cmd), F_OK) != -1)
	{
		// File exists
		return cmd_file;
	}

	return NULL;
}

static int print_error(int err_code, Command *command, char *error_msg)
{
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

int redirect_io(Command *command, char* cmd_str)
{
	// Default initialize
	command->in_stream = -1;
	command->out_stream = -1;
	int retval = 0;
	int bracket_idx = -1;

	for (int i = 0; cmd_str[i] != '\0'; i++)
	{
		char c = cmd_str[i];
		if (c == '<' || c == '>')
		{
			if (bracket_idx == -1) 
				bracket_idx = i;
			// open input file descriptor and copy to command
			do
				i++;
			while (cmd_str[i] != '\0' && cmd_str[i] == ' ');
			if (cmd_str[i] == '\0')
				return 0;
			int start_idx = i, len = 0;
			do
			{
				i++;
				len++;
			} while (cmd_str[i] != '\0' && cmd_str[i] != ' ' && cmd_str[i] != '<' && cmd_str[i] != '>');
			char file_name[len + 1];
			memcpy(file_name, &cmd_str[start_idx], len);
			file_name[len] = '\0';
			if (c == '<')
			{
				if ((command->in_stream = open(file_name, O_RDONLY)) < 0)
				{
					perror(file_name);
					// exit(EXIT_FAILURE);
					return -1;
				}
				retval += 2;
			}
			else
			{
				if ((command->out_stream = open(file_name, O_WRONLY | O_CREAT, 0644)) < 0)
				{
					perror(file_name);
					// exit(EXIT_FAILURE);
					return -1;
				}
				redir_output = 1;
				retval += 1;
			}
		}
	}
	cmd_str[bracket_idx] = '\0';
	return retval;
}

Command *tokenize(char *string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_command;
	char *this_token;
	char **tokens;

	// Intialize structure for Command
	Command *command = NULL;

	// Split the command string using the Filter / Pipe "|" operator
	// This will give us different commands, which we can split using whitespaces to obtain tokens
	while ((this_command = strsep(&string, "|")) != NULL &&
		   !(do_exit = (strcmp(trim_whitespace(this_command), EXIT_STR) == 0)))
	{

		if (*(trim_whitespace(this_command)) == '\0')
			continue;

		++num_commands;
		if (mysh_debug)
			printf("Command [%d]\n", num_commands);
		command = (Command *)malloc(sizeof(Command));
		command->__command_str = this_command;
		int redir_io = redirect_io(command, this_command);
		if (redir_io == -1) {
			cmd_valid = 0;
			return NULL;
		} else if (redir_io >= 2 && num_commands > 1) {
			printf("Syntax error: Input file is only allowed in first command");
			cmd_valid = 0;
			return NULL;
		} else if (redir_io >= 1 && redir_output >= 2) {
			printf("Syntax error: Output file is only allowed in last command");
			cmd_valid = 0;
			return NULL;
		}
		token_count = 0;

		// allocate space for individual tokens
		assert((tokens = malloc(sizeof(char *) * MAX_TOKENS)) != NULL);

		// Split the command using whitespaces, to obtain tokens
		while (((this_token = strsep(&this_command, " \t\v\f\n\r")) != NULL))
		{

			if (*this_token == '\0')
				continue;
			else if (*this_token == '|')
			{
				tokens[token_count] = NULL;
				command->tokens = tokens;
				command->next = tokenize(string);
				return command;
			}

			tokens[token_count] = this_token;
			if (mysh_debug)
				printf("Token %d: %s\n", token_count, tokens[token_count]);


			if (token_count == 0)
			{
				// Get absolute path of command's binary file
				if (!(command->command_file = get_command_file(this_token)))
				{
					printf("Command [%s] not found\n", this_token);
					cmd_valid = 0;
					return NULL;
				}
			}
			
			token_count++;


			// if there are more tokens than space ,reallocate more space
			if (token_count >= size)
			{
				size *= 2;

				assert((tokens = realloc(tokens, sizeof(char *) * size)) != NULL);
			}
		}
		tokens[token_count] = NULL;
		command->tokens = tokens;
		command->next = tokenize(string);
		return command;
	}
	return command;
}

void read_command()
{

	// getline will reallocate if input exceeds max length
	assert(getline(&line, &MAX_LINE_LEN, fp) > -1);

	if (mysh_debug)
		printf("Shell read this line: %s", line);

	num_commands = 0;
	redir_output = 0;
	cmd_valid = 1;
	command = tokenize(line);
}

int run_command()
{
	return command == NULL || !cmd_valid ? 0 : run(command, num_commands);
}

void gc(Command *cmd)
{
	if (cmd != NULL)
	{
		gc(cmd->next);
		if (cmd->tokens != NULL)
			free(cmd->tokens);
		if (cmd->command_file != NULL)
			free(cmd->command_file);
		free(cmd);
	}
}

int main(int argc, char **argv)
{
	initialize(argc, argv);

	do
	{
		printf("\nmysh> ");
		read_command();
		if (mysh_debug)
			printf("\n******************* OUTPUT **********************\n");
		run_command();
		gc(command);
	} while (!do_exit);

	return 0;
}
