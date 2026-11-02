#ifndef MINI_OS_RELOC_H
#define MINI_OS_RELOC_H

#include <stdint.h>

int region_conflict(const void *fdt, uintptr_t start, uintptr_t end);
int reloc_check(const void *fdt, uintptr_t dst);

#endif