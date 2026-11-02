#ifndef MINI_OS_FDT_UART_H
#define MINI_OS_FDT_UART_H

#include <stdint.h>

/* Generic parser */
int fdt_path_offset(const void* fdt, const char* path);
const void* fdt_getprop(const void* fdt, int nodeoffset,
                        const char* name, int* lenp);

/* UART MMIO base from /soc/uart or /soc/serial, 0 if none */
uintptr_t fdt_uart_base(const void *fdt);

/* /chosen/linux,initrd-{start,end}. Return 0 on success */
int fdt_get_initrd_range(const void *fdt,
                         const void **start, const void **end);

uint64_t read_cells(const uint32_t *p, uint32_t n);
uint32_t root_cells(const void *fdt, const char *name, uint32_t dflt);
uintptr_t fdt_total_size(const void *fdt);

#endif
