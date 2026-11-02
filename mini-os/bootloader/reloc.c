#include <stdint.h>
#include "fdt.h"
#include "uart.h"
#include "reloc.h"

extern  char _start[], _end[];  // get the addr. of _start & _end

/* if [a0, a1), [b0, b1) overlap */
static int overlaps(uintptr_t a0, uintptr_t a1, uintptr_t b0, uintptr_t b1)
{
    return a0 < b1 && b0 < a1;  // not overlap: a1 <= b0 or a0 >= b1 (use DeMorgan's law)
}

/* Print out the location of devices */
static void print_range(const char *tag, uintptr_t start, uintptr_t end)
{
    uart_puts(tag);
    uart_hex(start);
    uart_puts(" - ");
    uart_hex(end);
    uart_puts("\n");
}

/* Check if (start, end] (the destination) is completed inside the memory */
static int inside_memory(const void *fdt, uintptr_t start, uintptr_t end)
{
    int node = fdt_path_offset(fdt, "/memory");
    if (node < 0)
        return 0;

    int len = 0;
    const uint32_t *reg = fdt_getprop(fdt, node, "reg", &len);
    if (!reg)
        return 0;

    uint32_t ac = root_cells(fdt, "#address-cells", 2);
    uint32_t sc = root_cells(fdt, "#size-cells", 1);
    if (ac < 1 || ac > 2 || sc < 1 || sc > 2)
        return 0;

    int entry = (ac + sc) * 4;      // size of <base, size> for a pair
    for (int off = 0; off + entry <= len; off += entry){    // loop the info of memory (memory may not be fully contiguous)
        uint64_t base = read_cells(reg, ac);
        uint64_t size = read_cells(reg + ac, sc);
        reg += ac + sc;     // move to the end of reg info for this round
        
        if (start >= base && end <= base + size)
            return 1;
    }

    return 0;
}

/* Check if no one is using this region now */
int region_conflict(const void *fdt, uintptr_t start, uintptr_t end)
{
    uintptr_t dtb = (uintptr_t)fdt;
    const void *rd_start, *rd_end;

    if (!inside_memory(fdt, start, end)){
        uart_puts("  -> not inside /memory\n");
        return 1;
    }
    if (overlaps(start, end, dtb, dtb + fdt_total_size(fdt))){
        uart_puts("  -> overlaps the devicetree\n");
        return 1;
    }
    if (fdt_get_initrd_range(fdt, &rd_start, &rd_end) == 0 && overlaps(start, end, (uintptr_t)rd_start, (uintptr_t)rd_end)){
        uart_puts("  -> overlaps the initrd\n");
        return 1;
    }

    return 0;
}

int reloc_check(const void *fdt, uintptr_t dst)
{
    uintptr_t uart = fdt_uart_base(fdt);
    if (!uart)
        return -1;
    uart_init(uart);    // .bss will be cleared after reloc, so reinitialize it

    uintptr_t src = (uintptr_t)_start;  // PC-relative -> current address
    uintptr_t size = (uintptr_t)(_end - _start);    // code + data + .bss + stack
    const void *rd_start, *rd_end;

    uart_puts("\n[reloc] memory map\n");
    print_range("  running at : ", src, src + size);
    print_range("  relocate to: ", dst, dst + size);
    print_range("  devicetree : ", (uintptr_t)fdt, (uintptr_t)fdt + fdt_total_size(fdt));
    if (fdt_get_initrd_range(fdt, &rd_start, &rd_end) == 0)
        print_range("  initrd     : ", (uintptr_t)rd_start, (uintptr_t)rd_end);

    if (overlaps(dst, dst + size, src, src + size)) {
        uart_puts("[reloc] ERROR: destination overlaps the running image\n");
        return -1;
    }
    if (region_conflict(fdt, dst, dst + size)) {
        uart_puts("[reloc] ERROR: unsafe relocation address, halt\n");
        return -1;
    }
    uart_puts("[reloc] OK, relocating...\n");
    return 0;
}