#include "fdt_initrd.h"
#include "fdt.h"
#include "endian.h"
#include <stdint.h>
#include <stddef.h>

/* Linux /chosen properties may use a single 32-bit cell or a 64-bit value. */
/* Read out the value in the selected property */
static int read_address(const void *prop, int len, uintptr_t *result)
{
    
    /*
    chosen {
        linux,initrd-start = <0x00000000 0x83xxxxxx>;
        linux,initrd-end   = <0x00000000 0x83yyyyyy>;
    };
    */
    
    if (!prop || !result)
        return -1;

    /* 32-bit format (4 bytes) */
    if (len == 4) {
        *result = (uintptr_t)bswap32(*(const uint32_t *)prop);  // read out the property value
        return 0;
    }

    /* 64-bit format (8 bytes) */
    if (len == 8) {
        const uint32_t *p = (const uint32_t *)prop;

        // concate to 64-bit format
        uint64_t value = ((uint64_t)bswap32(p[0]) << 32) | bswap32(p[1]);

        *result = (uintptr_t)value;
        return 0;
    }

    return -1;
}

/* find the start address & end address for initramfs.cpio in RAM */
int fdt_get_initrd_range(const void *fdt,
                         const void **start, const void **end)
{
    if (!start || !end)
        return -1;

    *start = NULL;
    *end = NULL;

    /* find chosen node */
    int chosen = fdt_path_offset(fdt, "/chosen");
    if (chosen < 0)
        return -1;

    int start_len = 0, end_len = 0;     // length of linux,initrd-start & linux,initrd-end (byte format)
    const void *start_prop = fdt_getprop(fdt, chosen,
                                        "linux,initrd-start", &start_len);
    const void *end_prop = fdt_getprop(fdt, chosen,
                                      "linux,initrd-end", &end_len);

    uintptr_t first, last;
    if (read_address(start_prop, start_len, &first) < 0 ||
        read_address(end_prop, end_len, &last) < 0 ||
        first == 0 || last <= first)
        return -1;

    *start = (const void *)first;   // start address of initramfs
    *end = (const void *)last;      // end address of initramfs
    return 0;
}
