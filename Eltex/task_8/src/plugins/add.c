#include "plugin.h"

static int op_func(int a, int b) {
    return a + b;
}

const operation_t operation = {
  .name = "add",
  .description = "Sum of A and B",
  .func = op_func
};
