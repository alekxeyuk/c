#include "command.h"

#include "utils.h"

command_t parse_cmd(char* input, char** args) {
  char* token = strtok(input, T_DELIMS);
  if (!token) return CMD_UNKNOWN;

  if (strcmp(token, T_CREATE_DRIVER) == 0) {
    return CMD_CREATE_DRIVER;
  } else if (strcmp(token, T_SEND_TASK) == 0) {
    args[0] = strtok(NULL, T_DELIMS);
    args[1] = strtok(NULL, T_DELIMS);
    return CMD_SEND_TASK;
  } else if (strcmp(token, T_GET_STATUS) == 0) {
    args[0] = strtok(NULL, T_DELIMS);
    return CMD_GET_STATUS;
  } else if (strcmp(token, T_GET_DRIVERS) == 0) {
    return CMD_GET_DRIVERS;
  }

  return CMD_UNKNOWN;
}

static const command_handler_t* find_cmd(const ssize_t hc,
                                         const command_handler_t handlers[],
                                         const command_t cmd) {
  for (ssize_t i = 0; i < hc; i++) {
    if (handlers[i].cmd == cmd) {
      return &handlers[i];
    }
  }
  return NULL;
}

void handle_input(char* input, command_handler_t handlers[], ssize_t hc) {
  char* args[8];
  command_t cmd = parse_cmd(input, args);
  int argv[2];
  const command_handler_t* handler = find_cmd(hc, handlers, cmd);
  if (!handler) {
    printf("Unknown command.\n");
    return;
  }

  switch (cmd) {
    case CMD_CREATE_DRIVER:
      handler->handler(0, NULL);
      break;
    case CMD_SEND_TASK:
      for (int i = 0; i < handler->arg_count; i++) {
        if (!args[i]) {
          printf("send_task <pid> <task_timer>\n");
          return;
        }
        if (!satou(args[i], &argv[i])) {
          printf("bad args\n");
          return;
        }
      }

      handler->handler(2, argv);
      break;
    case CMD_GET_STATUS:
      if (!args[0]) {
        printf("get_status <pid>\n");
        return;
      }
      if (!satou(args[0], &argv[0])) {
        printf("bad pid\n");
        return;
      }

      handler->handler(1, argv);
      break;
    case CMD_GET_DRIVERS:
      handler->handler(0, NULL);
      break;
    default:
      printf("Unknown command.\n");
      break;
  }
}
