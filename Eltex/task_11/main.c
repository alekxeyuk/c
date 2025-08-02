#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "buyer.h"
#include "color.h"
#include "shop.h"
#include "worker.h"

#define MAX_BUYERS 3

SHOP shops[MAX_SHOPS_CNT];

int main(void) {
  setlocale(LC_ALL, "C.utf8");
  srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

  for (int i = 0; i < MAX_SHOPS_CNT; i++) {
    shop_init(&shops[i]);
    printf(GRN "SHOP[%d] starts with stock [%d]\n" RESET, i, shops[i].stock);
  }

  pthread_t buyers_t[MAX_BUYERS];
  for (int i = 0; i < MAX_BUYERS; i++) {
    int st = pthread_create(&buyers_t[i], NULL, buyer_thread, NULL);
    if (st != 0) {
      perror("pthread_create failed");
    }
  }

  pthread_t worker_t;
  int st = pthread_create(&worker_t, NULL, worker_thread, NULL);
  if (st != 0) {
    perror("pthread_create failed");
  }

  for (int i = 0; i < MAX_BUYERS; i++) {
    st = pthread_join(buyers_t[i], NULL);
    if (st != 0) {
      perror("pthread_join failed");
    }
  }

  st = pthread_cancel(worker_t);
  if (st != 0) {
    perror("pthread_cancel failed");
  }

  for (int i = 0; i < MAX_SHOPS_CNT; i++) {
    printf(GRN "SHOP[%d] left with stock [%d]\n" RESET, i, shops[i].stock);
  }

  for (int i = 0; i < MAX_SHOPS_CNT; i++) {
    shop_destroy(&shops[i]);
  }

  exit(EXIT_SUCCESS);
}
