#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "helper.h"

char* trim_spaces(char* s) {
  while (isspace((unsigned char)*s)) s++;
  if (*s) {
    char* p = s;
    while (*p) p++;
    while (isspace((unsigned char)*(--p)));
    p[1] = '\0';
  }
  return s;
}

int split_str(char* input, char* subs[], const char* delim) {
  int s = 0;
  char* tkn = strtok(input, delim);

  while (tkn != NULL && s < MAX_ARGS - 1) {
    subs[s] = trim_spaces(tkn);
    tkn = strtok(NULL, delim);
    s++;
  }
  subs[s] = NULL;

  return s;
}

bool process_command(char* argv[]) {
  if (strncmp("exit", argv[0], 4) == 0) {
    exit(EXIT_SUCCESS);
  }
  return false;
}