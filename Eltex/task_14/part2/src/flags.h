#ifndef FLAGS_H_
#define FLAGS_H_

typedef unsigned int flags_t;

#define LOG_BIT 0
#define USER_BIT 1
#define INPUT_BIT 2
#define RESIZE_BIT 3
#define STOP_BIT 4
#define MODIFIED_BIT 5

#define IS_BIT_SET(flags, bit) (flags) & (1 << bit)
#define SET_BIT(flags, bit) (flags) |= (1 << bit)
#define CLEAR_BIT(flags, bit) (flags) &= ~(1 << bit)

#endif  // FLAGS_H_