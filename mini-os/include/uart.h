#ifndef MINI_OS_UART_H
#define MINI_OS_UART_H

#include <stdint.h>

void uart_init(uintptr_t base);
char uart_getc(void);
unsigned char uart_getb(void); /* Only bootloader needs this. */
void uart_putc(char c);
void uart_puts(const char *s);
void uart_hex(unsigned long value); /* Currently implemented by bootloader. */

#endif
