#ifndef MINI_OS_STRING_H
#define MINI_OS_STRING_H

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(const char *s, int c);
char *strcpy(char *dest, const char *src);
int memcmp(const void *a, const void *b, size_t n);

#endif
