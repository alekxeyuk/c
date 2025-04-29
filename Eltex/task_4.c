#include <locale.h>
#include <stdio.h>

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__)
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

#define N 100

typedef enum { ADD = 1, DELETE, SEARCH, PRINT, EXIT } state;

struct abonent {
  char name[10];
  char second_name[10];
  char tel[10];
};

static void flush_stdint(void);
static void menu_print(void);
static void add_abonent(struct abonent db[]);
static void del_abonent(struct abonent db[]);
static void search_db(struct abonent db[]);
static void print_db(struct abonent db[]);
static void wipe_db(struct abonent db[]);
static void wipe_entry(struct abonent db[], size_t n);

int main(void) {
  setlocale(LC_ALL, "Russian");

  struct abonent db[N];
  wipe_db(db);

  state user_input = 0;

  do {
    menu_print();
    if (MY_SCANF("%d", &user_input) == 0) {
      flush_stdint();
      printf("Некорректный ввод\n");
      continue;
    }
    flush_stdint();

    switch (user_input) {
      case ADD:
        add_abonent(db);
        break;
      case DELETE:
        del_abonent(db);
        break;
      case SEARCH:
        search_db(db);
        break;
      case PRINT:
        print_db(db);
        break;
      case EXIT:
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

static void flush_stdint(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

static int strncmp(const char* ptr1, const char* ptr2, size_t n) {
  char* f = (char*)ptr1;
  char* s = (char*)ptr2;
  for (size_t i = 0; i < n; i++) {
    if (f[i] == '\0' && s[i] == '\0') return 0;
    if (f[i] > s[i]) return 1;
    if (f[i] < s[i]) return -1;
  }
  return 0;
}

static void add_abonent(struct abonent db[]) {
  size_t i;
  for (i = 0; i < N; i++) {
    if (db[i].name[0] == '\0') break;
  }
  if (i == N) {
    printf("В справочнике нет свободного места.\n");
    return;
  }

  int status = 1;

  printf("Введите имя: ");
  status &= MY_SCANF(" %10[^\n]", db[i].name, 10);
  flush_stdint();

  printf("Введите фамилию: ");
  status &= MY_SCANF(" %10[^\n]", db[i].second_name, 10);
  flush_stdint();

  printf("Введите номер телефона: ");
  status &= MY_SCANF(" %10[^\n]", db[i].tel, 10);
  flush_stdint();

  if (status == 0) {
    printf("Введенные данные некорректные.\n");
    wipe_entry(db, i);
  }
}

void del_abonent(struct abonent db[]) {
  int i;
  printf("Введите index записи: ");
  if (MY_SCANF("%d", &i) != 1) {
    flush_stdint();
    printf("Некорректный ввод\n");
    return;
  }
  flush_stdint();
  if (i < 0 || i >= N) {
    printf("Неправильный index\n");
    return;
  }
  if (db[i].name[0] == '\0') {
    printf("Запись с таким index отсутствует\n");
    return;
  }
  wipe_entry(db, i);
  printf("Запись удалена\n");
}

void search_db(struct abonent db[]) {
  char search_buff[10];
  printf("Введите имя для поиска: ");
  if (MY_SCANF(" %10[^\n]", search_buff, 10) != 1) {
    flush_stdint();
    printf("Некорректный ввод\n");
    return;
  }
  flush_stdint();

  int found = 0;
  for (size_t i = 0; i < N; i++) {
    if (strncmp(search_buff, db[i].name, 10) == 0) {
      found++;
      printf("[%zu] - ( %s )( %s )( %s )\n", i, db[i].name, db[i].second_name,
             db[i].tel);
    }
  }
  if (!found) {
    printf("Записей с таким именем нет\n");
  }
}

void print_db(struct abonent db[]) {
  for (size_t i = 0; i < N; i++) {
    if (db[i].name[0] == '\0') continue;
    printf("[%zu] - ( %s )( %s )( %s )\n", i, db[i].name, db[i].second_name,
           db[i].tel);
  }
}

void wipe_db(struct abonent db[]) {
  for (size_t i = 0; i < N; i++) {
    wipe_entry(db, i);
  }
}

void wipe_entry(struct abonent db[], size_t n) {
  for (char* ptr = (char*)&(db[n]); ptr < (char*)&(db[n + 1]); ptr++) {
    *ptr = '\0';
  }
}
