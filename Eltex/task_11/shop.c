#include "shop.h"

#include <stdlib.h>

void shop_init(SHOP* shp) {
  shp->stock = rand() % MAX_STOCK_CAP;
  pthread_mutex_init(&shp->mutex, NULL);
}

void shop_destroy(SHOP* shp) { pthread_mutex_destroy(&shp->mutex); }