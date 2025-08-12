#ifndef LOG_H_
#define LOG_G_

#include "common.h"

typedef struct {
  chatmsg messages[MAX_MESSAGES];
  int head;
  int count;
} chatlog_t;

void init_chatlog(chatlog_t *log);
void add_message(chatlog_t *log, const char *user, const char *msg, msgtype type);
const chatmsg *get_message(const chatlog_t *log, int index);

#endif  // LOG_H_