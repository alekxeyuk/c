#include <locale.h>
#include <stdio.h>

#include "abonent.h"
#include "list.h"
#include "utils.h"

typedef enum { ADD = 1, DELETE, SEARCH, PRINT, EXIT } state;

static void menu_print(void);

int main(void) {
  #ifdef _WIN32
  setlocale(LC_ALL, "Russian");
  #endif

  List db = {.head = NULL, .tail = NULL};

  state user_input = 0;

  do {
    menu_print();
    if (MY_SCANF("%d", (int*)&user_input) == 0) {
      flush_stdint();
      printf("Некорректный ввод\n");
      continue;
    }
    flush_stdint();

    switch (user_input) {
      case ADD:
        add_abonent(&db);
        break;
      case DELETE:
        del_abonent(&db);
        break;
      case SEARCH:
        search_db(&db);
        break;
      case PRINT:
        print_db(&db);
        break;
      case EXIT:
        freeList(&db);
        return 0;
      default:
        printf("Некорректный ввод\n");
        break;
    }
  } while (1);
}

static void menu_print(void) {
  printf(
      "\
1) Добавить абонента\n\
2) Удалить абонента\n\
3) Поиск абонентов по имени\n\
4) Вывод всех записей\n\
5) Выход\n\
");
}
