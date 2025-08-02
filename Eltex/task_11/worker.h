#ifndef WORKER_H_
#define WORKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shop.h"

#define CAN_ADD 5000
#define SECONDS 1000000
#define WRK_TO_SLEEP 1 * SECONDS

void* worker_thread(void* arg);

extern SHOP shops[];

#endif  // WORKER_H_
