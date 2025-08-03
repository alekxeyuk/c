#ifndef HELPER_H_
#define HELPER_H_

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

char* trim_spaces(char*);
int split_str(char*, char**, const char*);
bool process_command(char**);

#endif  // HELPER_H_
