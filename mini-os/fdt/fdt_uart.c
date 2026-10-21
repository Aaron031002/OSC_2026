#include "fdt.h"
#include "endian.h"

#include <stdint.h>

/* find the base address of UART */
uintptr_t fdt_uart_base(const void *fdt)    // fdt points to the start point of DTB in memory
{
    if (!fdt)   
        return 0;

    /* find the UART node offset */
    int node = fdt_path_offset(fdt, "/soc/uart");      /* QEMU virt */
    if (node < 0)
        node = fdt_path_offset(fdt, "/soc/serial");    /* OrangePi RV2 */
    if (node < 0)
        return 0;

    /* 
    / {
        soc {
            #address-cells = <1>;
            #size-cells = <1>;

            uart@10000000 {
                reg = <0x10000000 0x100>;
            };
        };
    };
    The start point of UART register is 0x10000000
    The size of the register area is 0x100 bytes
    */

    /* #address-cells belongs to the parent bus, /soc, not the UART child. */
    int parent = fdt_path_offset(fdt, "/soc");
    if (parent < 0)
        return 0;

    uint32_t address_cells = 2; /* Devicetree spec's default when absent. */
    int len = 0;    // length of 32-bit address_cell
    const void *cells = fdt_getprop(fdt, parent, "#address-cells", &len);
    if (cells) {    // 'cells' points to the start point of property value
        if (len != 4)   // 4 bytes (32-bit)
            return 0;
        address_cells = bswap32(*(const uint32_t *)cells);  // change to the real value of address_cells
    }

    /* check the reg property of UART */
    const void *reg = fdt_getprop(fdt, node, "reg", &len);
    if (!reg)
        return 0;

    if (address_cells == 1) {
        if (len < 4)
            return 0;
        return (uintptr_t)bswap32(*(const uint32_t *)reg);
    }
    if (address_cells == 2) {   // to represent a 64-bit address_cells
        if (len < 8)
            return 0;
        const uint32_t *addr = (const uint32_t *)reg;

        uint64_t high = bswap32(addr[0]);
        uint64_t low = bswap32(addr[1]);
        return (uintptr_t)((high << 32) | low);     // concate to the 64-bit address_cells
    }

    /* This lab uses 1- or 2-cell UART addresses; don't guess otherwise. */
    return 0;
}
