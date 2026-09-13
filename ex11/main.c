#define UART_BASE 0x10000000UL
#define UART_RBR  (unsigned char*)(UART_BASE + 0x0) // Receive Buffer Register (buffer that receive data)
#define UART_THR  (unsigned char*)(UART_BASE + 0x0) // Transmit Holding Register (holds the data to be transmitted)
#define UART_LSR  (unsigned char*)(UART_BASE + 0x5)
#define LSR_DR    (1 << 0)  // if data is available in Receive Buffer Register
#define LSR_TDRQ  (1 << 5)  // if UART is ready to accept a new char for transmission

char uart_getc() {
    // TODO: Implement this function
    while ((*UART_LSR) & LSR_DR == 0);

    return *UART_RBR;
}

void uart_putc(char c) {
    // TODO: Implement this function
    while ((*UART_LSR) & LSR_TDRQ == 0);

    *UART_THR = c;
}

void uart_puts(const char* s) {
    // TODO: Implement this function
    while (*s != '\0'){
        uart_putc(*s);
        s++;
    }
}

void start_kernel() {
    uart_puts("\nStarting kernel ...\n");
    while (1) {
        uart_putc(uart_getc());
    }
}
