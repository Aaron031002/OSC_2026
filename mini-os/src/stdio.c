#include "stdio.h"
#include <stdarg.h>
#include <stdint.h>

/* Provided by the existing UART driver; do not duplicate it here. */
extern void uart_putc(char c);

struct mini_file { int is_uart; };
static FILE console = {1};
FILE *stdout = &console;

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!size || !nmemb) return 0;
    if (!ptr || stream != stdout || nmemb > SIZE_MAX / size) return 0;

    const unsigned char *p = ptr;
    size_t bytes = size * nmemb;
    for (size_t i = 0; i < bytes; i++) uart_putc((char)p[i]);
    return nmemb;
}

static void print_unsigned(unsigned int value, int *count)
{
    char digits[10];
    unsigned int n = 0;
    do {
        digits[n++] = (char)('0' + value % 10);
        value /= 10;
    } while (value);
    while (n) { uart_putc(digits[--n]); (*count)++; }
}

int printf(const char *format, ...)
{
    va_list args;
    int count = 0;
    va_start(args, format);

    for (const char *p = format; *p; p++) {
        if (*p != '%') {
            uart_putc(*p);
            count++;
            continue;
        }
        p++;
        if (*p == '%') {
            uart_putc('%');
            count++;
        } else if (*p == 's') {
            const char *s = va_arg(args, const char *);
            if (!s) s = "(null)";
            while (*s) { uart_putc(*s++); count++; }
        } else if (*p == 'i') {
            int number = va_arg(args, int);
            if (number < 0) {
                uart_putc('-');
                count++;
                print_unsigned((unsigned int)(-(long long)number), &count);
            } else {
                print_unsigned((unsigned int)number, &count);
            }
        } else {
            va_end(args);
            return -1; /* Unsupported conversion; add it only when needed. */
        }
    }
    va_end(args);
    return count;
}
