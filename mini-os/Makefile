# Convenience entrypoint for the existing separate bootloader/kernel Makefiles.
QEMU ?= qemu-system-riscv64
ROOTFS_ITEMS := $(shell find rootfs -mindepth 1 -print)

.PHONY: all initrd kernel bootloader run clean

all: initrd kernel bootloader

initrd: rootfs/initramfs.cpio

kernel:
	$(MAKE) -C kernel

bootloader:
	$(MAKE) -C bootloader bootloader

run: all
	cd bootloader && $(QEMU) -M virt -m 128M -kernel bootloader -display none -serial pty -monitor stdio -S -initrd ../rootfs/initramfs.cpio

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C bootloader clean
