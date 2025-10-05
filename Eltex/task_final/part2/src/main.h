#ifndef MAIN_H_
#define MAIN_H_

#include "driver.h"
#include "utils.h"

#define EVENTS_SIZE 100

static command_handler_t cmd_table[] = {
    {CMD_CREATE_DRIVER, 0, handle_create_driver},
    {CMD_SEND_TASK, 2, handle_send_task},
    {CMD_GET_STATUS, 1, handle_get_status},
    {CMD_GET_DRIVERS, 0, handle_get_drivers},
    {CMD_UNKNOWN, 0, NULL}};

#define CMD_TABLE_SIZE (sizeof(cmd_table) / sizeof(cmd_table[0]) - 1)

#endif  // MAIN_H_