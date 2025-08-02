#include "worker.h"

#include "color.h"

void* worker_thread(void* arg) {
  (void)arg;
  while (1) {
    int managed_to_add = 0;

    for (int i = 0; i < MAX_SHOPS_CNT; i++) {
      SHOP* shop = &shops[i];
      if (pthread_mutex_trylock(&shop->mutex) == 0) {
        if (shop->stock >= MAX_STOCK_CAP) {
          pthread_mutex_unlock(&shop->mutex);
          continue;
        }
        printf(RED
               "<WORKER> at shop [%d] with stock [%d] will add [%d]\n" RESET,
               i, shop->stock, CAN_ADD);
        shop->stock += CAN_ADD;
        pthread_mutex_unlock(&shop->mutex);
        managed_to_add = 1;
        break;
      }
    }

    if (managed_to_add) {
      printf(RED "<WORKER> sleeping for %d seconds\n" RESET,
             WRK_TO_SLEEP / SECONDS);
      usleep(WRK_TO_SLEEP);
    }
  }

  pthread_exit(EXIT_SUCCESS);
}