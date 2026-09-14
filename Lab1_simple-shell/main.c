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

int strcmp(const char* s1, const char* s2)
{
    while (*s1 != '\0' && *s2 != '\0'){
        if (*s1 != *s2){
            return 0;
        }
        s1++;
        s2++;
    }

    return (*s1 == '\0' && *s2 == '\0');
}

void shell() 
{
    char command[128];
    int index = 0;

    uart_puts("opi - rv2> ");

    while (1){
        char c_in = uart_getc();
        uart_putc(c_in);

        if (c_in == '\n' || c_in == '\r'){
            command[index] = '\0';

            if (strcmp(command, "hello")){
                uart_puts("\nHello World!\r\n");
            }
            else if (strcmp(command, "help")){
                uart_puts("\nAvailable commands:\
                            \n  help - show all commands.\
                            \n  hello - print Hello World.\r\n");
            }
            else {
                uart_puts("\nUnknown command: ");
                uart_puts(command);
                uart_puts("\nuse help to get commands.\r\n");
            }
            
            index = 0;
            uart_puts("opi - rv2> ");
        }
        else {
            if (index < 127){
                command[index++] = c_in;
            }
        }
        
    }
}

void start_kernel() {
    uart_puts("\nStarting kernel ...\n");
    while (1) {
        shell();
    }
}
