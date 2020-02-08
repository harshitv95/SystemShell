#ifndef _MYSH_UTILS
#define _MYSH_UTILS

char *trim_whitespace(char *str);

int count_occurences(char *str, char ch);

// Just like strtok, but with multiple delimiters instead of 1
char* strtoks(char *str, char* delimeters);

#endif