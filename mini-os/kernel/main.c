#include <string.h>
#include "fdt.h"
#include "uart.h"
#include "fdt_uart.h"
#include "cpio.h"
#include "fdt_initrd.h"

static const void *initrd_start;
static int initrd_ready;

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
                            \n  hello - print Hello World.\
                            \n  ls - list initramfs files.\
                            \n  cat <file> - display a file\n");
            }
            else if (!strcmp(command, "ls")) {
                uart_puts("\n");

                if (initrd_ready)
                    initrd_list(initrd_start);
                else
                    uart_puts("Initramfs not available\n");
            }
            else if (!strncmp(command, "cat", 3) &&
                    (command[3] == '\0' ||
                    command[3] == ' ' ||
                    command[3] == '\t')) {

                const char *name = command + 3;     // point to ' ' space char (ex:cat hello.txt)

                while (*name == ' ' || *name == '\t')
                    name++;

                uart_puts("\n");
                
                if (!*name)
                    uart_puts("Usage: cat <filename>\n");
                else if (!initrd_ready)
                    uart_puts("Initramfs not available\n");
                else
                    initrd_cat(initrd_start, name);
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
    

    /* UART DTB part */
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
    
    /* initrd DTB part */
    const void *initrd_end = NULL;

    if (fdt_get_initrd_range(fdt, &initrd_start, &initrd_end) == 0 &&
        initrd_set_range(initrd_start, initrd_end) == 0) 
    {
        initrd_ready = 1;
        uart_puts("Initramfs range found in DTB\n");
    }
    else {
        initrd_start = NULL;
        initrd_ready = 0;
        uart_puts("Initramfs missing in DTB\n");
    }


    while (1) {
        shell();
    }
}
