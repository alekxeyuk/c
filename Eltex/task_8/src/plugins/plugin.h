#ifndef PLUGIN_H_
#define PLUGIN_H_

typedef int (*op_func_t)(int a, int b);

typedef struct {
  const char* name;
  const char* description;
  op_func_t func;
} operation_t;

extern const operation_t operation;

#endif  // PLUGIN_H_
