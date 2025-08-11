#ifndef UI_H_
#define UI_H_

#include <pthread.h>

#include "common.h"

typedef enum {
  NOOP = -1,
  RESIZE = 1,
  LOG,
  USER,
  INPUT,
  STOP,
} update_type_t;

int start_ui_thread(pthread_t *ui_thread);
void update_ui(update_type_t type);
#endif  // UI_H_