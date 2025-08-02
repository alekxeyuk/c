#ifndef SHOP_H_
#define SHOP_H_

#include <pthread.h>

#define MAX_STOCK_CAP 10000
#define MAX_SHOPS_CNT 5

typedef struct {
  int stock;
  pthread_mutex_t mutex;
} SHOP;

void shop_init(SHOP*);
void shop_destroy(SHOP*);

#endif  // SHOP_H_
