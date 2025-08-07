#include "plugin.h"

static int op_func(int a, int b) {
    if (b != 0) {
        return a / b;
    }
    return 0;
}

const operation_t operation = {
  .name = "div",
  .description = "Divide A by B",
  .func = op_func
};
