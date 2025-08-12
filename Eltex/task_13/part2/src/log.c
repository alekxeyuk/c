#include "log.h"

#include <string.h>

void init_chatlog(chatlog_t *log) {
  log->head = -1;
  log->count = 0;
}

void add_message(chatlog_t *log, const char *user, const char *msg, msgtype type) {
  log->head = (log->head + 1) % MAX_MESSAGES;
  strncpy(log->messages[log->head].msgtext, msg, MAX_MSG_SIZE - 1);
  strncpy(log->messages[log->head].username, user, MAX_USERNAME_SIZE - 1);
  log->messages[log->head].msgtext[MAX_MSG_SIZE - 1] = '\0';
  log->messages[log->head].username[MAX_USERNAME_SIZE - 1] = '\0';
  log->messages[log->head].type = type;
  if (log->count < MAX_MESSAGES) log->count++;
}

const chatmsg *get_message(const chatlog_t *log, int index) {
  // index: 0 = oldest, count-1 = newest
  int real_index = (log->head - log->count + 1 + index + MAX_MESSAGES) % MAX_MESSAGES;
  return &log->messages[real_index];
}