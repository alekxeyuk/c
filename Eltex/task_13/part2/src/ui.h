#ifndef UI_H_
#define UI_H_

#include <pthread.h>

#include "common.h"

typedef enum {
  UNOOP = -1,
  ULOG,
  UUSER,
  UINPUT,
  URESIZE,
  USTOP,
} update_type_t;

int start_ui_thread(pthread_t *ui_thread, pthread_mutex_t *l_mut, pthread_mutex_t *u_mut);
void update_ui(update_type_t type);
#endif  // UI_H_