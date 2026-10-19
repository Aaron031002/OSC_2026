#ifndef MINI_OS_STDIO_H
#define MINI_OS_STDIO_H

#include <stddef.h>

typedef struct mini_file FILE;
extern FILE *stdout;

int printf(const char *format, ...);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif
