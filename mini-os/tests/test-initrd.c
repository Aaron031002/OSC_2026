#include "fdt_initrd.h"
#include "cpio.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Test that our own stdio.c really routes output to UART. */
static char output[4096];
static size_t written;
void uart_putc(char c)
{
    if (written < sizeof(output) - 1)
        output[written++] = c;
    output[written] = '\0';
}
static void reset_output(void) { written = 0; output[0] = '\0'; }
static int includes(const char *s, const char *needle)
{
    size_t n = strlen(needle);
    while (*s) {
        if (strncmp(s, needle, n) == 0) return 1;
        s++;
    }
    return 0;
}

static unsigned char archive[2048] __attribute__((aligned(16)));
static size_t apos;
static void put_bytes(const char *src, size_t n)
{
    for (size_t i=0; i<n; i++) archive[apos++] = (unsigned char)src[i];
}
static void put_hex8(size_t off, unsigned int value)
{
    const char hex[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        archive[off + (size_t)i] = (unsigned char)hex[value & 15];
        value >>= 4;
    }
}
static void put_file(const char *name, const char *contents)
{
    size_t start = apos;
    /* Header: magic(6) plus 13 8-byte hex fields. */
    put_bytes("070701", 6);
    for (int i = 0; i < 104; ++i) archive[apos++] = '0';
    size_t n = strlen(name) + 1;
    size_t len = strlen(contents);
    put_hex8(start + 54, (unsigned int)len);  /* filesize */
    put_hex8(start + 94, (unsigned int)n);    /* namesize */
    put_bytes(name, n);
    while (apos % 4) archive[apos++] = 0;
    put_bytes(contents, len);
    while (apos % 4) archive[apos++] = 0;
}

static unsigned char fdt[512] __attribute__((aligned(8)));
static size_t dpos;
static void be32(size_t at, uint32_t v)
{
    fdt[at] = (unsigned char)(v >> 24);
    fdt[at+1] = (unsigned char)(v >> 16);
    fdt[at+2] = (unsigned char)(v >> 8);
    fdt[at+3] = (unsigned char)v;
}
static void push32(uint32_t v) { be32(dpos, v); dpos += 4; }
static void push_name(const char *s)
{
    do { fdt[dpos++] = (unsigned char)*s; } while (*s++);
    while (dpos % 4) fdt[dpos++] = 0;
}
static void prop_addr(uint32_t off, uintptr_t val, int is64)
{
    push32(3); push32(is64 ? 8 : 4); push32(off);
    if (is64)
        push32((uint32_t)((uint64_t)val >> 32));
    push32((uint32_t)val);
}
static void build_dtb(uintptr_t start, uintptr_t end, int is64)
{
    dpos = 56;
    push32(1); push_name("");
    push32(1); push_name("chosen");
    prop_addr(0, start, is64);
    prop_addr(19, end, is64);
    push32(2); push32(2); push32(9);
    uint32_t s_size = (uint32_t)(dpos - 56);
    uint32_t strings_off = (uint32_t)dpos;
    const char strings[] = "linux,initrd-start\0linux,initrd-end\0";
    for (size_t i = 0; i < sizeof(strings); ++i)
        fdt[dpos++] = (unsigned char)strings[i];
    be32(0, 0xd00dfeed);
    be32(4, (uint32_t)dpos);
    be32(8, 56);
    be32(12, strings_off);
    be32(16, 40);
    be32(20, 17);
    be32(24, 16);
    be32(28, 0);
    be32(32, (uint32_t)sizeof(strings));
    be32(36, s_size);
}

static int tests(void)
{
    put_file(".", "");
    put_file("./hello.txt", "hello from test\n");
    put_file("test.txt", "abc\n");
    put_file("TRAILER!!!", "");
    build_dtb((uintptr_t)archive, (uintptr_t)archive + apos, 1);

    const void *start = NULL, *end = NULL;
    if (fdt_get_initrd_range(fdt, &start, &end) != 0) return 1;
    if (start != archive || (uintptr_t)end != (uintptr_t)archive + apos) return 2;
    if (initrd_set_range(start, end) != 0) return 3;

    reset_output(); initrd_list(start);
    if (!includes(output, "0 .\n") ||
        !includes(output, "16 ./hello.txt\n") ||
        !includes(output, "4 test.txt\n")) return 4;

    reset_output(); initrd_cat(start, "hello.txt");
    if (strcmp(output, "hello from test\n") != 0) return 5;
    reset_output(); initrd_cat(start, "./test.txt");
    if (strcmp(output, "abc\n") != 0) return 6;
    reset_output(); initrd_cat(start, "missing");
    if (!includes(output, "No such file")) return 7;

    archive[0] = 'X';
    reset_output(); initrd_list(start);
    if (!includes(output, "Invalid CPIO")) return 8;
    archive[0] = '0';

    fdt[0] = 0;
    if (fdt_get_initrd_range(fdt, &start, &end) == 0) return 9;

    /* Some firmwares encode chosen initrd addresses in 32 bits. */
    build_dtb(0x83000000U, 0x83001000U, 0);
    if (fdt_get_initrd_range(fdt, &start, &end) != 0) return 10;
    if ((uintptr_t)start != 0x83000000U ||
        (uintptr_t)end != 0x83001000U) return 11;
    return 0;
}

__attribute__((noreturn)) void _start(void)
{
    long status = tests();
    __asm__ volatile("syscall" : : "a"(60L), "D"(status) : "rcx", "r11", "memory");
    __builtin_unreachable();
}
