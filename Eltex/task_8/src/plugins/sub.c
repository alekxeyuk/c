#include "plugin.h"

static int op_func(int a, int b) {
    return a - b;
}
const operation_t operation = {
  .name = "sub",
  .description = "Subtract B from A",
  .func = op_func
};
