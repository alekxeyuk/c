#ifndef DRIVER_H_
#define DRIVER_H_

#include <stdbool.h>

bool handle_create_driver(int argc, int argv[]);
bool handle_send_task(int argc, int argv[]);
bool handle_get_status(int argc, int argv[]);
bool handle_get_drivers(int argc, int argv[]);

#endif  // DRIVER_H_