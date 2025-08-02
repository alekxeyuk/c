#ifndef BUYER_H_
#define BUYER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shop.h"

#define MAX_NEED_CAP 100000
#define SECONDS 1000000
#define BYER_TO_SLEEP 2 * SECONDS

void* buyer_thread(void* arg);

extern SHOP shops[];

#endif  // BUYER_H_
