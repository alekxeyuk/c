#include "buyer.h"

#include "color.h"

void* buyer_thread(void* arg) {
  (void)arg;
  pthread_t buyer_id = pthread_self();
  unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)buyer_id;
  int need = rand_r(&seed) % MAX_NEED_CAP;

  while (need > 0) {
    int managed_to_eat = 0;

    for (int i = 0; i < MAX_SHOPS_CNT; i++) {
      SHOP* shop = &shops[i];
      if (pthread_mutex_trylock(&shop->mutex) == 0) {
        if (shop->stock == 0) {
          pthread_mutex_unlock(&shop->mutex);
          continue;
        }
        printf(CYN "<BUYER>[%jd] with need [%d] at shop [%d] with stock [%d]\n" RESET,
               buyer_id, need, i, shop->stock);
        if (shop->stock <= need) {
          need -= shop->stock;
          shop->stock = 0;
        } else {
          shop->stock -= need;
          need = 0;
        }
        pthread_mutex_unlock(&shop->mutex);
        managed_to_eat = 1;
        break;
      }
    }

    if (need <= 0) break;

    if (managed_to_eat) {
      printf(CYN "<BUYER>[%jd] sleeping with need [%d]\n" RESET, buyer_id, need);
      usleep(BYER_TO_SLEEP);
    }
  }

  printf(CYN "<BUYER>[%jd] fulfilled\n" RESET, buyer_id);

  pthread_exit(EXIT_SUCCESS);
}