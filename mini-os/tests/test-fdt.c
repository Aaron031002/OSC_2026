#include "fdt.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Small in-memory DTB: no fopen/fread needed in the bare-metal parser test. */
static uint8_t blob[2048];
static unsigned int pos;

static void be32_at(unsigned int at, uint32_t value)
{
    blob[at] = (uint8_t)(value >> 24);
    blob[at + 1] = (uint8_t)(value >> 16);
    blob[at + 2] = (uint8_t)(value >> 8);
    blob[at + 3] = (uint8_t)value;
}

static void push32(uint32_t value) { be32_at(pos, value); pos += 4; }
static void push_name(const char *name)
{
    do { blob[pos++] = (uint8_t)*name; } while (*name++);
    while (pos % 4) blob[pos++] = 0;
}

static void prop32(uint32_t nameoff, uint32_t value)
{
    push32(3); /* FDT_PROP */
    push32(4);
    push32(nameoff);
    push32(value);
}

static void prop_reg(void)
{
    push32(3); /* FDT_PROP */
    push32(16);
    push32(0); /* "reg" offset in strings */
    push32(0); push32(0x10000000); /* 64-bit address */
    push32(0); push32(0x100);      /* 64-bit size */
}

static int make_blob(void)
{
    const unsigned int off_struct = 56;
    pos = off_struct;
    push32(1); push_name(""); /* root */

    push32(1); push_name("soc");
    prop32(4, 2); /* #address-cells */
    push32(1); push_name("uart@10000000");
    prop_reg();
    push32(2); /* end uart */
    push32(2); /* end soc */

    push32(1); push_name("memory@80000000");
    push32(2);

    push32(1); push_name("chosen");
    prop32(19, 0x83000000); /* linux,initrd-start, 4-byte sample */
    push32(2);

    push32(2); /* end root */
    push32(9); /* FDT_END */

    unsigned int size_struct = pos - off_struct;
    unsigned int off_strings = pos;
    const char strings[] = "reg\0#address-cells\0linux,initrd-start\0";
    /* Explicitly copy strings (including trailing NUL). */
    for (unsigned int i = 0; i < sizeof(strings); i++)
        blob[pos++] = (uint8_t)strings[i];

    be32_at(0, 0xd00dfeed);
    be32_at(4, pos);
    be32_at(8, off_struct);
    be32_at(12, off_strings);
    be32_at(16, 40); /* reserved-memory map */
    be32_at(20, 17); /* FDT version */
    be32_at(24, 16); /* compatible version */
    be32_at(28, 0);
    be32_at(32, sizeof(strings));
    be32_at(36, size_struct);
    return 0;
}

int main(void)
{
    make_blob();
    if (fdt_path_offset(blob, "/") != 0) return 1;
    int uart = fdt_path_offset(blob, "/soc/uart");
    if (uart < 0) return 2;
    if (fdt_path_offset(blob, "/soc/uart@10000000") != uart) return 3;
    if (fdt_path_offset(blob, "/soc/not-a-device") != -1) return 4;
    if (fdt_path_offset(blob, "/memory") < 0) return 5;
    if (fdt_path_offset(blob, "/chosen") < 0) return 6;

    int len = 0;
    const uint8_t *reg = fdt_getprop(blob, uart, "reg", &len);
    if (!reg || len != 16 || reg[7] != 0x00 || reg[6] != 0x00 || reg[5] != 0x00 || reg[4] != 0x10) return 7;
    if (fdt_getprop(blob, uart, "absent", &len) != NULL) return 8;

    /* Confirm repeated malloc/free calls do not exhaust the 16 KiB heap. */
    for (int i = 0; i < 500; ++i)
        if (fdt_path_offset(blob, "/soc/uart") != uart) return 9;

    uint8_t first = blob[0];
    blob[0] = 0;
    if (fdt_path_offset(blob, "/soc/uart") != -1) return 10;
    blob[0] = first;
    return 0;
}

/* Exit to Linux via syscall to avoid relying on host libc. */
__attribute__((noreturn)) void _start(void)
{
    int status = main();
    __asm__ volatile("syscall" : : "a"(60L), "D"((long)status) : "rcx", "r11", "memory");
    __builtin_unreachable();
}
