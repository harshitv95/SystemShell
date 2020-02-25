#include "utils.h"

#include <ctype.h>
#include <string.h>

char *trim_whitespace(char *str)
{
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*str))
    str++;

  if (*str == 0) // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// int count_occurences(char *str, char ch) {
//   int count;
//   char *s;
//   // *s = *str;
//   strcpy(s, str);
//   for (count=0; s[count]; s[count]==ch ? count++ : *s++);
//   return  count;
// }

// char* strtoks(char* str, char* delimeters) {
//   char *s, *sbk;
//   s = strtok(str, delimeters);
//   *sbk = *s;
//   while (s != NULL) {
//     *sbk = *s;
//     s = strtok(NULL, delimeters);
//   }
//   return sbk;
// }

char *get_input_output_files(char *command)
{
  char *io[2];
  const char *cmd = command;
  // Input file
  char *inp = strstr(cmd, "<");
  if (inp == NULL)
    io[0] = NULL;
  else
  {
    char filename[strlen(inp)];
    strcpy(filename, inp);
    strcpy(filename, trim_whitespace(filename));
    io[0] = strtok(filename, " >\0");
  }
  char *out = strstr(cmd, ">");
  if (inp == NULL)
    io[1] = NULL;
  else
  {
    char filename[strlen(out)];
    strcpy(filename, out);
    strcpy(filename, trim_whitespace(filename));
    io[0] = strtok(filename, " <\0");
  }

  return *io;
}