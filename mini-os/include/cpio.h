#ifndef MINI_OS_CPIO_H
#define MINI_OS_CPIO_H

void initrd_list(const void *rd);
void initrd_cat(const void *rd, const char *filename);

/* New: bound every CPIO read by the DTB-provided start/end addresses. */
int initrd_set_range(const void *start, const void *end);

#endif
