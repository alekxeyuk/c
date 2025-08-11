#ifndef CLIENT_H_
#define CLIENT_H_

#include <pthread.h>

#include "common.h"

int init_message_queue(const char *path, char key_char);
int start_consumer_thread(pthread_t *ui_thread, pthread_mutex_t *l_mut, pthread_mutex_t *u_mut);
int start_publisher_thread(pthread_t *ui_thread);
void send_message(msgtype type);
void close_message_queue(void);

#endif  // CLIENT_H_