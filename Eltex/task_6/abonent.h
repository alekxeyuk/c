#ifndef ABONENT_H_
#define ABONENT_H_

#include "list.h"

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__, (size_t)sizeof(__VA_ARGS__))
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

void add_abonent(List*);
void del_abonent(List*);
void search_db(List*);
void print_db(List*);

void flush_stdint(void);
#endif  // ABONENT_H_
