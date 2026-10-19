#include "string.h"

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n--) {
        unsigned char x = (unsigned char)*a++;
        unsigned char y = (unsigned char)*b++;
        if (x != y) return (int)x - (int)y;
        if (x == '\0') return 0;
    }
    return 0;
}

char *strchr(const char *s, int c)
{
    for (;;) {
        if (*s == (char)c) return (char *)s;
        if (*s == '\0') return NULL;
        s++;
    }
}

char *strcpy(char *dest, const char *src)
{
    char *start = dest;
    while ((*dest++ = *src++) != '\0') {}
    return start;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a;
    const unsigned char *y = b;
    for (size_t i = 0; i < n; i++) {
        if (x[i] != y[i]) return (int)x[i] - (int)y[i];
    }
    return 0;
}
