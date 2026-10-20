#ifndef MINI_OS_FDT_UART_H
#define MINI_OS_FDT_UART_H

#include <stdint.h>

/* Get the UART MMIO base via ex21's two unchanged parser APIs.
 * Returns 0 if no supported UART node/property is found.
 */
uintptr_t fdt_uart_base(const void *fdt);

#endif
