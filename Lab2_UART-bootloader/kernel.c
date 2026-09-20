extern char uart_getc(void);
extern void uart_putc(char c);
extern void uart_puts(const char* s);

void start_kernel()
{
    uart_puts("\nStage-2 bootloader is running...\n");

    while (1){
        uart_putc(uart_getc());
    }
}