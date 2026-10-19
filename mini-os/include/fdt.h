#ifndef MINI_OS_FDT_H
#define MINI_OS_FDT_H

/* These two interfaces are kept unchanged from ex21. */
int fdt_path_offset(const void *fdt, const char *path);
const void *fdt_getprop(const void *fdt, int nodeoffset,
                        const char *name, int *lenp);

#endif
