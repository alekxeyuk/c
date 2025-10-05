#include "driver.h"

#include <stdio.h>

bool handle_create_driver(int argc, int argv[]) {
  (void)argc, (void)argv;
  printf("TODO: create_driver\n");
  return true;
}

bool handle_send_task(int argc, int argv[]) {
  if (argc != 2) {
    return false;
  }
  int pid = argv[0];
  int timer = argv[1];
  printf("TODO: send_task [%d][%d]\n", pid, timer);
  return true;
}

bool handle_get_status(int argc, int argv[]) {
  if (argc != 1) {
    return false;
  }
  int pid = argv[0];
  printf("TODO: get_status [%d]\n", pid);
  return true;
}

bool handle_get_drivers(int argc, int argv[]) {
  (void)argc, (void)argv;
  printf("TODO: get_drivers\n");
  return true;
}