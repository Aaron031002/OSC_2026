#include <string.h>
#include "fdt.h"
#include "uart.h"
#include "fdt_uart.h"

static uintptr_t uart_base;

#define UART_RBR ((volatile unsigned char*)(uart_base + 0x0))   // Receive Buffer Register (buffer that receive data)
#define UART_THR ((volatile unsigned char*)(uart_base + 0x0))   // Transmit Holding Register (holds the data to be transmitted)
#define UART_LSR ((volatile unsigned char*)(uart_base + 0x5))
#define LSR_DR    (1 << 0)  // if data is available in Receive Buffer Register
#define LSR_TDRQ  (1 << 5)  // if UART is ready to accept a new char for transmission

void uart_init(uintptr_t base)
{
    uart_base = base;
}

char uart_getc() {
    // TODO: Implement this function
    while (((*UART_LSR) & LSR_DR) == 0);

    return *UART_RBR;
}

void uart_putc(char c) {
    // TODO: Implement this function
    if (c == '\n'){
        uart_putc('\r');
    }
    
    while (((*UART_LSR) & LSR_TDRQ) == 0);

    *UART_THR = c;
}

void uart_puts(const char* s) {
    // TODO: Implement this function
    while (*s != '\0'){
        uart_putc(*s);
        s++;
    }
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

            if (!strcmp(command, "hello")){
                uart_puts("\nHello World!\n");
            }
            else if (!strcmp(command, "help")){
                uart_puts("\nAvailable commands:\
                            \n  help - show all commands.\
                            \n  hello - print Hello World.\n");
            }
            else {
                uart_puts("\nUnknown command: ");
                uart_puts(command);
                uart_puts("\nuse help to get commands.\n");
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

void start_kernel(unsigned long hartid, const void *fdt) 
{
    (void)hartid;   // tell the compiler 'hartid' won't be used

    uintptr_t uart_mmio = fdt_uart_base(fdt);

    if (!uart_mmio) {
        while (1);
    }

    uart_init(uart_mmio);

    uart_puts("\nStarting kernel ...\n");
    
    if (!fdt){
        uart_puts("Error: DTB pointer is NULL\n");
    }
    else{
        const unsigned char *dtb = (const unsigned char*)fdt;

        /* prevent it from endian problem */
        if (dtb[0] == 0xd0 && dtb[1] == 0x0d && dtb[2] == 0xfe && dtb[3] == 0xed){
            uart_puts("DTB pointer received; magic OK!\n");

            /* find UART node for QEMU */
            int offset = fdt_path_offset(fdt, "/soc/serial");

            if (offset < 0)
                offset = fdt_path_offset(fdt, "/soc/uart");

            if (offset >= 0) {
                int len;
                const void *reg =
                    fdt_getprop(fdt, offset, "reg", &len);

                if (reg && len >= 4)
                    uart_puts("FDT: UART reg found\n");
                else
                    uart_puts("FDT: UART reg missing\n");
            }
            else {
                uart_puts("FDT: /soc/uart not found\n");
            }
        }
        else{
            uart_puts("Error: Invalid DTB magic\n");
        }
    }
    
    while (1) {
        shell();
    }
}
