#ifndef UTILS_H_
#define UTILS_H_
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>

void cleanup(void);
bool prefix(const char* pre, const char* str);
// Move selection cursos up
void sel_dec(size_t* sel);
// Move selection cursos down
void sel_inc(size_t* sel, size_t max);
// Static Getter/Setter for curent user's PWD
const char* stored_user_pwd(void* ptr);

#endif  // UTILS_H_