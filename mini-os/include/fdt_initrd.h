#ifndef MINI_OS_FDT_INITRD_H
#define MINI_OS_FDT_INITRD_H

/* Resolve /chosen/linux,initrd-{start,end} dynamically, without constants. */
int fdt_get_initrd_range(const void *fdt,
                         const void **start, const void **end);

#endif
