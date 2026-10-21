#include "uart.h"

static uintptr_t uart_base; 

// volatile: this state register could be change, tell compiler do not caching it 
#define UART_RBR ((volatile unsigned char*)(uart_base + 0x0))   // Receive Buffer Register (buffer that receive data)
#define UART_THR ((volatile unsigned char*)(uart_base + 0x0))   // Transmit Holding Register (holds the data to be transmitted)
#define UART_LSR ((volatile unsigned char*)(uart_base + 0x5))
#define LSR_DR    (1 << 0)  // if data is available in Receive Buffer Register
#define LSR_TDRQ  (1 << 5)  // if UART is ready to accept a new char for transmission

/* store the UART base address just found to uart_base */
void uart_init(uintptr_t base)
{
    uart_base = base;
}

char uart_getc() 
{
    // TODO: Implement this function
    while ((*UART_LSR & LSR_DR) == 0)
        ;
    char c = (char)*UART_RBR;
    return c == '\r' ? '\n' : c;
}

unsigned char uart_getb()   // for binary transmission (if .img contain 0x0D(ie.\r) would be change to 0x0A(\n))
{
    while ((*UART_LSR & LSR_DR) == 0);

    return *UART_RBR;
}

void uart_putc(char c) 
{
    // TODO: Implement this function
    if (c == '\n')
        uart_putc('\r');

    while ((*UART_LSR & LSR_TDRQ) == 0)
        ;
    *UART_THR = c;
}

void uart_puts(const char* s) 
{
    // TODO: Implement this function
    while (*s)
        uart_putc(*s++);
}

void uart_hex(unsigned long h) {
    uart_puts("0x");
    unsigned long n;
    for (int c = 60; c >= 0; c -= 4) {
        n = (h >> c) & 0xf;
        n += n > 9 ? 0x57 : '0';
        uart_putc(n);
    }
}
