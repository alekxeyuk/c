#include "abonent.h"
#include "utils.h"

void appendAbonent(List* lst, Abonent d) {
  Node* ptr = createNode(d);
  if (ptr == NULL) {
    perror("Fatal error when appending abonent");
    exit(-1);
  }
  appendNode(lst, ptr);
}

void freeAbonent(List* lst, Node* n) {
  if (n == NULL) {
    return;
  }
  if (n == lst->head) {
    lst->head = n->next;
  }
  if (n == lst->tail) {
    lst->tail = n->prev;
  }
  freeNode(n);
}

void printAbonent(Abonent* d) {
  printf("( %s )( %s )( %s )\n", d->name, d->second_name, d->tel);
}

void add_abonent(List* lst) {
  Abonent temp = {0};
  int status = 1;

  printf("Введите имя: ");
  status &= MY_SCANF(" %10[^\n]", temp.name);
  flush_stdint();

  printf("Введите фамилию: ");
  status &= MY_SCANF(" %10[^\n]", temp.second_name);
  flush_stdint();

  printf("Введите номер телефона: ");
  status &= MY_SCANF(" %9[^\n]", temp.tel);
  flush_stdint();

  if (status == 0) {
    printf("Введенные данные некорректные.\n");
    return;
  }

  appendAbonent(lst, temp);
}

void del_abonent(List* lst) {
  char search_buff[11];
  printf("Введите имя для поиска: ");
  if (MY_SCANF(" %10[^\n]", search_buff) != 1) {
    flush_stdint();
    printf("Некорректный ввод\n");
    return;
  }
  flush_stdint();

  int found = 0;
  Node* searchResult = searchList(lst, search_buff);
  while (searchResult != NULL) {
    found++;
    printAbonent(&searchResult->data);

    printf("Удаляем? Y/N\n");
    int choice = getchar();
    if (choice == 'Y') {
      freeAbonent(lst, searchResult);
      printf("Запись удалена\n");
      break;
    } else {
      flush_stdint();
    }

    searchResult = findNext(searchResult, search_buff);
  }

  if (!found) {
    printf("Записей с таким именем нет\n");
  }
}

void search_db(List* lst) {
  char search_buff[11];
  printf("Введите имя для поиска: ");
  if (MY_SCANF(" %10[^\n]", search_buff) != 1) {
    flush_stdint();
    printf("Некорректный ввод\n");
    return;
  }
  flush_stdint();

  int found = 0;
  Node* searchResult = searchList(lst, search_buff);
  while (searchResult != NULL) {
    found++;
    printAbonent(&searchResult->data);
    searchResult = findNext(searchResult, search_buff);
  }

  if (!found) {
    printf("Записей с таким именем нет\n");
  }
}

void print_db(List* lst) { printList(lst); }
