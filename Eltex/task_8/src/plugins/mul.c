#include "plugin.h"

static int op_func(int a, int b) {
    return a * b;
}

const operation_t operation = {
  .name = "mul",
  .description = "Multiply A by B",
  .func = op_func
};
