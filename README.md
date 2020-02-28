# Command Shell

A shell console, that supports commands with arguments, input as well output file redirection, and multiple commands separated by pipes, and all of these combined.

## Installation and Usage

Use the command ```make``` to compile the shell and link the object files. This produces an executable name 'mysh'. Run this executable on the command line:
```bash
$ make
$ ./mysh
```
Use the following command to start the shell in debug mode (this prints all the commands, and tokens in each command, and also the exit status of each child in their order of execution):
```bash
$ ./mysh debug
```

You should see the following prompt, here you can type in the command(s):
```bash
mysh> _
```

## Approach

### Data Structure
The key data structure used in this shell is a Linked List of a struct 'Command':
```
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
```

### Logic
#### 1. Parser
 1. Different Commands are obtained by separating the entire typed string on basis of pipe ('|') character - *strsep(&string, "|")*
 2. Different tokens in each command are obtained by separating each command on the basis of space
 3. For each command, various syntax validations, and file existence validations are done, and the fields ***command_file***, ***tokens***, and ***__command_str*** of the ***Command*** struct are populated
 4. Input and Output files, if specified, are opened and their file descriptors are assigned to ***in_stream*** and ***out_stream*** respectively
 5. The commands are then forwarded  to the **Executor**
 
#### 2. Executor
- The executor first looks at the number of commands; if there's just one command, **fork**s a single child process, **exec**utes it and returns, while taking care of Input and Ouput file redirection, if any
- If there are more than 1 command, i.e. ***n*** commands, then:
  1. Creates an array of ***n-1*** pipes, each containing 2 fields, **Read** and **Write**
  2. The first command reads either from Input file or ```STDIN```, so the **Read** end of the first pipe is closed
  3. Analogous to this, the last command writes either to the Output file or ```STDOUT```, so the **Write** end of the last pipe is closed too
  4. Each process (command) makes the ```pipe()``` system call, to create a pipe between itself and the next process
  5. Each process then writes to the **Write** end of the pipe it just created, and reads from the **Read** end of the pipe created by the previous process
  6. The shell (parent process) of course **fork**s child process after calling ```pipe()``` and **exec**utes the child