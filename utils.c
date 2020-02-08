#include "utils.h"

#include <ctype.h>
#include <string.h>

char *trim_whitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

int count_occurences(char *str, char ch) {
  int count;
  char *s;
  *s = *str;
  for (count=0; s[count]; s[count]=='.' ? count++ : *s++);
  return  count;
}

char* strtoks(char* str, char* delimeters) {
  char *s, *sbk;
  s = strtok(str, delimeters);
  *sbk = *s;
  while (s != NULL) {
    *sbk = *s;
    s = strtok(NULL, delimeters);
  }
  return sbk;
}