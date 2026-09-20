extern char uart_getc(void);
extern void uart_putc(char c);
extern void uart_puts(const char* s);
extern void uart_hex(unsigned long h);
extern unsigned char uart_getb(void);

#define SBI_EXT_SET_TIMER 0x0
#define SBI_EXT_SHUTDOWN  0x8
#define SBI_EXT_BASE      0x10
#define BOOT_MAGIC        0x544F4F42
#define KERNEL_LOAD_ADDR  0x82000000UL

enum sbi_ext_base_fid {
    SBI_EXT_BASE_GET_SPEC_VERSION,
    SBI_EXT_BASE_GET_IMP_ID,
    SBI_EXT_BASE_GET_IMP_VERSION,
    SBI_EXT_BASE_PROBE_EXT,
    SBI_EXT_BASE_GET_MVENDORID,
    SBI_EXT_BASE_GET_MARCHID,
    SBI_EXT_BASE_GET_MIMPID,
};

struct sbiret {
    long error;
    long value;
};

struct sbiret sbi_ecall(int ext,
                        int fid,
                        unsigned long arg0,
                        unsigned long arg1,
                        unsigned long arg2,
                        unsigned long arg3,
                        unsigned long arg4,
                        unsigned long arg5) {
    struct sbiret ret;
    register unsigned long a0 asm("a0") = (unsigned long)arg0;
    register unsigned long a1 asm("a1") = (unsigned long)arg1;
    register unsigned long a2 asm("a2") = (unsigned long)arg2;
    register unsigned long a3 asm("a3") = (unsigned long)arg3;
    register unsigned long a4 asm("a4") = (unsigned long)arg4;
    register unsigned long a5 asm("a5") = (unsigned long)arg5;
    register unsigned long a6 asm("a6") = (unsigned long)fid;
    register unsigned long a7 asm("a7") = (unsigned long)ext;
    asm volatile("ecall"
                 : "+r"(a0), "+r"(a1)
                 : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                 : "memory");
    ret.error = a0;
    ret.value = a1;
    return ret;
}

struct boot_header {
    unsigned int magic;
    unsigned int size;
};

/**
 * sbi_get_spec_version() - Get the SBI specification version.
 *
 * Return: The current SBI specification version.
 * The minor number of the SBI specification is encoded in the low 24 bits,
 * with the major number encoded in the next 7 bits. Bit 31 must be 0.
 */
long sbi_get_spec_version(void) {
    // TODO: Implement this function
    struct sbiret ret;

    ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_SPEC_VERSION, 0, 0, 0, 0, 0, 0);

    return ret.value;
}

/**
 * sbi_probe_extension() - Check if an SBI extension ID is supported or not.
 * @extid: The extension ID to be probed.
 *
 * Return: 1 or an extension specific nonzero value if yes, 0 otherwise.
 */
long sbi_probe_extension(int extid) {
    // TODO: Implement this function
    struct sbiret ret;

    ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_PROBE_EXT, extid, 0, 0, 0, 0, 0);

    return ret.value;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 != '\0' && *s2 != '\0'){
        if (*s1 != *s2){
            return 1;
        }
        s1++;
        s2++;
    }

    return !(*s1 == '\0' && *s2 == '\0');
}

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

    /* (void (*)(void)) cast the type into a function pointer with no parameter and void output type */
    void (*kernel_entry)(void) = (void (*)(void))KERNEL_LOAD_ADDR;  // function pointer: dynamically decide which function to execute

    /* jump to execute the new kernel */
    kernel_entry(); // execute kernel_entry at KERNEL_LOAD_ADDR (PC points to KERNEL_LOAD_ADDR)

    while(1);   // do not jump back to the initial addr (in case of finishing executing kernel_entry)
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
                uart_puts("Hello World!\n");
            }
            else if (!strcmp(command, "help")){
                uart_puts("Available commands:\
                            \n  help - show all commands.\
                            \n  hello - print Hello World.\n");
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

    uart_puts("SBI specification version: ");
    uart_hex(sbi_get_spec_version());
    uart_puts("\n");

    uart_puts("Probe Set Timer: ");
    uart_hex(sbi_probe_extension(SBI_EXT_SET_TIMER));
    uart_puts("\n");

    uart_puts("Probe Shutdown:  ");
    uart_hex(sbi_probe_extension(SBI_EXT_SHUTDOWN));
    uart_puts("\n");

    while (1) {
        shell();
    }
}
