#include "utils.h"

void cleanup(void) { endwin(); }

bool prefix(const char* pre, const char* str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

void sel_dec(size_t* sel) {
    if (*sel > 0) (*sel)--;
}

void sel_inc(size_t* sel, size_t max) {
    if (*sel < max) (*sel)++;
}

const char* stored_user_pwd(void* ptr) {
    static const char* pwd;
    if (ptr != NULL) {
        pwd = (const char*)ptr;
    }
    return pwd;
}