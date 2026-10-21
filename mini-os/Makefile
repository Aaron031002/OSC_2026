# ---- Toolchain ---------------------------------------------------------
CROSS   ?= riscv64-unknown-elf-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
QEMU    ?= qemu-system-riscv64
HOST_CC ?= gcc

# ---- Memory layout (single source of truth) ----------------------------
BOOTLOADER_ADDR := 0x80200000
KERNEL_ADDR     := 0x82000000

BUILD := build

CFLAGS := -march=rv64gc -mabi=lp64d -mcmodel=medany \
          -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -g -Wall -Iinclude -MMD -MP \
          -DKERNEL_LOAD_ADDR=$(KERNEL_ADDR)UL
LDLIBS   ?= -lgcc

# ---- Sources -----------------------------------------------------------
COMMON_SRCS := drivers/uart.c \
               lib/string.c lib/malloc.c \
               fdt/fdt.c fdt/fdt_uart.c

BOOT_SRCS   := bootloader/start.S $(COMMON_SRCS) arch/riscv/sbi.c bootloader/main.c

KERNEL_SRCS := kernel/start.S $(COMMON_SRCS) lib/stdio.c fs/cpio.c fdt/fdt_initrd.c \
               kernel/main.c

obj = $(patsubst %,$(BUILD)/obj/%.o,$(1))
BOOT_OBJS   := $(call obj,$(BOOT_SRCS))
KERNEL_OBJS := $(call obj,$(KERNEL_SRCS))

# ---- Targets -----------------------------------------------------------
.PHONY: all bootloader kernel initrd run test clean
.DELETE_ON_ERROR:

all: bootloader kernel initrd

bootloader: $(BUILD)/bootloader.bin
kernel:     $(BUILD)/kernel.bin
initrd:     $(BUILD)/initramfs.cpio

# Every .c / .S compiles the same way; objects are shared by both images.
$(BUILD)/obj/%.o: %
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/bootloader.elf: $(BOOT_OBJS) bootloader/linker.ld
	$(CC) $(CFLAGS) -T bootloader/linker.ld -Wl,--defsym=LOAD_ADDR=$(BOOTLOADER_ADDR) \
		$(BOOT_OBJS) $(LDLIBS) -o $@

$(BUILD)/kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(CC) $(CFLAGS) -T kernel/linker.ld -Wl,--defsym=LOAD_ADDR=$(KERNEL_ADDR) \
		$(KERNEL_OBJS) $(LDLIBS) -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

ROOTFS_FILES := $(shell find rootfs -type f)
$(BUILD)/initramfs.cpio: $(ROOTFS_FILES)
	@mkdir -p $(@D)
	cd rootfs && find . -mindepth 1 -printf '%P\n' | cpio -o -H newc > ../$@

run: all
	$(QEMU) -M virt -m 128M -display none -serial pty -monitor stdio -S \
		-kernel $(BUILD)/bootloader.bin -initrd $(BUILD)/initramfs.cpio

# ---- Host unit tests (x86-64 Linux) ------------------------------------
HOST_CFLAGS := -std=gnu11 -ffreestanding -nostdlib -fno-builtin \
               -fno-stack-protector -fno-pie -no-pie -static \
               -Wall -Wextra -Werror -Iinclude
TESTS := $(BUILD)/tests/test-lib $(BUILD)/tests/test-fdt $(BUILD)/tests/test-initrd

$(BUILD)/tests/test-lib:    tests/test-lib.c lib/string.c lib/malloc.c lib/stdio.c
$(BUILD)/tests/test-fdt:    tests/test-fdt.c lib/string.c lib/malloc.c fdt/fdt.c
$(BUILD)/tests/test-initrd: tests/test-initrd.c lib/string.c lib/malloc.c lib/stdio.c \
                            fdt/fdt.c fdt/fdt_initrd.c fs/cpio.c
$(TESTS):
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) $^ -o $@

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || { echo "FAIL: $$t (exit $$?)"; exit 1; }; done
	@echo "All host tests: PASS"

clean:
	rm -rf $(BUILD)

-include $(BOOT_OBJS:.o=.d) $(KERNEL_OBJS:.o=.d)