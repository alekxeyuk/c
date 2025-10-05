#ifndef DRIVER_H_
#define DRIVER_H_

#include <stdbool.h>
#include <sys/types.h>

typedef enum { DRIVER_AVAILABLE, DRIVER_BUSY } driver_state_t;

typedef struct {
  pid_t pid;
  driver_state_t state;
  int timer;
  int pipe_to;
  int pipe_from;
} driver_t;

#define PIPE_BUF_SIZE 64
#define DEVENTS_SIZE 64
#define MAX_DRIVERS 64

#define ERR_DRIVER_NOT_FOUND "Driver not found"
#define ERR_INVALID_ARGS "Invalid arguments"
#define ERR_TOO_MANY_DRIVERS "Maximum drivers exceeded"
#define ERR_PIPE_CREATE "Failed to create pipe"
#define ERR_FORK_FAILED "Failed to fork driver process"

#define MSG_TASK "TASK"
#define MSG_STATUS "STATUS"
#define MSG_BUSY "BUSY"
#define MSG_AVAILABLE "AVAILABLE"

bool register_pipe(int pipe);
bool handle_create_driver(int argc, int argv[]);
bool handle_send_task(int argc, int argv[]);
bool handle_get_status(int argc, int argv[]);
bool handle_get_drivers(int argc, int argv[]);
void kill_drivers(void);
void update_driver_state(pid_t pid, driver_state_t state,
                         int remaining_seconds);

#endif  // DRIVER_H_