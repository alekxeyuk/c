#ifndef UTILS_H_
#define UTILS_H_

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__, (size_t)sizeof(__VA_ARGS__))
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

int strncmp(const char*, const char*, size_t n);
void flush_stdint(void);

#endif  // UTILS_H_