#ifndef COMMAND_H_
#define COMMAND_H_

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define T_DELIMS " \t\n"

#define T_CREATE_DRIVER "create_driver"
#define T_SEND_TASK "send_task"
#define T_GET_STATUS "get_status"
#define T_GET_DRIVERS "get_drivers"

typedef enum {
  CMD_CREATE_DRIVER,
  CMD_SEND_TASK,
  CMD_GET_STATUS,
  CMD_GET_DRIVERS,
  CMD_UNKNOWN
} command_t;

typedef bool (*cmd_handler_fn)(int argc, int argv[]);

typedef struct {
  command_t cmd;
  int arg_count;
  cmd_handler_fn handler;
} command_handler_t;

command_t parse_cmd(char* input, char** args);
void handle_input(char* input, command_handler_t handlers[], ssize_t hc);

#endif  // COMMAND_H_