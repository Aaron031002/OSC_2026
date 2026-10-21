#include <string.h>
#include "uart.h"
#include "sbi.h"
#include "fdt.h"

#define BOOT_MAGIC        0x544F4F42
#define KERNEL_LOAD_ADDR  0x82000000UL

static unsigned long boot_hartid;   // CPU hart ID currently activated
static const void *boot_fdt;    // the address DTB located in

struct boot_header {
    unsigned int magic;
    unsigned int size;
};

void load_kernel()
{
    struct boot_header header;

    uart_puts("Waiting for kernel...\n");

    unsigned char* header_ptr = (unsigned char*)&header;

    /* load the values of magic and size into header_ptr */
    for (unsigned long i=0; i<sizeof(header); i++){
        header_ptr[i] = uart_getb();    // wait magic & size from python
    }

    /* check if it is the correct image */
    if (header.magic != BOOT_MAGIC){
        uart_puts("\nInvalid boot image\n");
        return;
    }

    uart_puts("\nHeader OK\n");
    uart_puts("Kernel size: ");
    uart_hex(header.size);
    uart_puts("\n");

    uart_puts("\nReceiving kernel...\n");

    /* kernel point to the kernel start point */
    unsigned char* kernel = (unsigned char*)KERNEL_LOAD_ADDR;

    /* load kernel image into kernel start point */
    for (unsigned long i=0; i<header.size; i++){
        kernel[i] = uart_getb();    // wait python to transmit image
    }

    uart_puts("Kernel received\n");

    asm volatile("fence.i" ::: "memory"); // see the data that just updated as instruction to fetch, and make a memory barrier to stop compiler from reordering

/*--------------------------------------------*/
    /* (void (*)(void)) cast the type into a function pointer with no parameter and void output type */
    //void (*kernel_entry)(void) = (void (*)(void))KERNEL_LOAD_ADDR;  // function pointer: dynamically decide which function to execute

    /* jump to execute the new kernel */
    //kernel_entry(); // execute kernel_entry at KERNEL_LOAD_ADDR (PC points to KERNEL_LOAD_ADDR)
/*-----------------------------------------------*/
    
    /* define a function pointer points to functions like:
        void start_kernel(unsigned long hartid, const void *fdt){} 
    */
    typedef void (*kernel_entry_t)(
        unsigned long  hartid,
        const void *fdt
    );

    /* the pointer points to KERNEL_LOAD_ADDR and execute the function at that location */
    kernel_entry_t kernel_entry = (kernel_entry_t)KERNEL_LOAD_ADDR;

    kernel_entry(boot_hartid, boot_fdt);

    while(1);   // do not jump back to the initial addr (in case of finishing executing kernel_entry)
}

void shell() 
{
    char command[128];
    int index = 0;

    uart_puts("stage_1-boot> ");

    while (1){
        char c_in = uart_getc();
        uart_putc(c_in);

        if (c_in == '\n' || c_in == '\r'){
            command[index] = '\0';

            if (!strcmp(command, "hello")){
                uart_puts("Hello World!\n");
            }
            else if (!strcmp(command, "help")){
                uart_puts("Available commands:\
                            \n  help - show all commands.\
                            \n  hello - print Hello World.\
                            \n  load - load kernel.\n");
            }
            else if (!strcmp(command, "load")){
                load_kernel();
            }
            else {
                uart_puts("Unknown command: ");
                uart_puts(command);
                uart_puts("\nuse help to get commands.\n");
            }
            
            index = 0;
            uart_puts("stage_1-boot> ");
        }
        else {
            if (index < 127){
                command[index++] = c_in;
            }
        }
        
    }
}

void start_kernel(unsigned long hartid, const void* fdt) 
{
    /* preserve the info openSBI sent */
    boot_hartid = hartid;
    boot_fdt = fdt;    
    
    /* Get the UART base address from the DTB */
    uintptr_t uart_mmio = fdt_uart_base(fdt);

    if (!uart_mmio) {
        while (1);
    }

    uart_init(uart_mmio);   // store it as variable

    uart_puts("UART base from DTB: ");
    uart_hex(uart_mmio);
    uart_puts("\n");


    uart_puts("\nStarting kernel ...\n");


    uart_puts("SBI specification version: ");
    uart_hex(sbi_get_spec_version());
    uart_puts("\n");

    uart_puts("Probe Set Timer: ");
    uart_hex(sbi_probe_extension(SBI_EXT_SET_TIMER));
    uart_puts("\n");

    uart_puts("Probe Shutdown:  ");
    uart_hex(sbi_probe_extension(SBI_EXT_SHUTDOWN));
    uart_puts("\n");

    shell();
}
