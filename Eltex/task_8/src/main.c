#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "plugins/plugin.h"
#include "helper.h"

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "C.utf8");

  const char* plug_path = (argc > 1) ? argv[1] : "./plugins";

  plugin_entry_t plugins[MAX_OPERATIONS];
  size_t n_plugins = load_plugins(plug_path, plugins, MAX_OPERATIONS);

  if (n_plugins == 0) {
    fprintf(stderr, "No plugins found in %s\n", plug_path);
    exit(EXIT_FAILURE);
  }
  
  bool work = true;
  int a, b;
  do {
    print_menu(plugins, n_plugins);
    int user_input = select_operation(n_plugins);

    switch (user_input) {
    case -2:
      printf("Invalid choice\n");
      break;
    case -1:
      work = false;
      break;
    default:
      if (!read_ab(&a, &b)) {
        fprintf(stderr, "Bad input\n");
        flush_stdint();
        continue;
      }

      const operation_t* op = plugins[user_input].op;
      int result = op->func(a, b);
      printf("%s(%d, %d) = %d\n", op->name, a, b, result);
    }
  } while (work);

  unload_plugins(plugins, n_plugins);
  exit(EXIT_SUCCESS);
}
