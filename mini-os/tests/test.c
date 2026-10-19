#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>

static char logbuf[200];
static size_t loglen;
void uart_putc(char c) {
    if (loglen < sizeof(logbuf)) logbuf[loglen++] = c;
}

int main(void) {
    if (strlen("abcd") != 4) return 1;
    if (strcmp("a", "a") != 0 || strcmp("a", "b") >= 0) return 2;
    if (strncmp("hello", "help", 3) != 0) return 3;
    if (strchr("ab@123", '@') != (char *)("ab@123" + 2)) return 4;
    char buffer[8];
    if (strcpy(buffer, "test") != buffer || strcmp(buffer, "test")) return 5;
    if (memcmp("abc", "abd", 3) >= 0) return 6;
    void *a = malloc(32), *b = malloc(32);
    if (!a || !b || (uintptr_t)a % 16 || (uintptr_t)b % 16) return 7;
    free(a); free(b);
    void *c = malloc(8000);
    if (!c) return 8;
    free(c);
    if (printf("file %i %s %%\n", -42, "ok") != 14) return 9;
    if (fwrite("A\0B", 1, 3, stdout) != 3) return 10;
    if (loglen != 17 || memcmp(logbuf, "file -42 ok %\nA\0B", 17) != 0) return 11;
    return 0;
}

/* Bare x86-64 Linux test entry point; no host C runtime is used. */
__attribute__((noreturn)) void _start(void) {
    int status = main();
    __asm__ volatile("syscall" : : "a"(60L), "D"((long)status) : "rcx", "r11", "memory");
    __builtin_unreachable();
}
