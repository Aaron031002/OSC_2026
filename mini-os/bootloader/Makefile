RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy	# elf -> objcopy -> raw binary
CFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany \
         -ffreestanding -nostdlib -fno-builtin \
         -fno-stack-protector -g -Wall -I../include
QEMU ?= qemu-system-riscv64

# build depends on these files
build: clean bootloader	

bootloader:
	$(CC) $(CFLAGS) -c start.S -o boot_start.o
	$(CC) $(CFLAGS) -c uart.c -o boot_uart.o
	$(CC) $(CFLAGS) -c main.c -o boot_main.o
	$(CC) $(CFLAGS) -c ../src/string.c -o boot_string.o
	$(CC) $(CFLAGS) -c ../src/malloc.c -o boot_malloc.o
	$(CC) $(CFLAGS) -c ../src/fdt.c -o boot_fdt.o
	$(CC) $(CFLAGS) -c ../src/fdt_uart.c -o boot_fdt_uart.o

	
	$(CC) $(CFLAGS) -T link.ld \
		-o bootloader.elf \
		boot_start.o boot_uart.o boot_main.o \
		boot_string.o boot_malloc.o boot_fdt.o boot_fdt_uart.o \
		-lgcc


	# make elf to raw binary
	$(OBJCOPY) -O binary \
		bootloader.elf \
		bootloader

run: bootloader
	$(QEMU) -M virt -m 128M -kernel bootloader -display none -serial pty -monitor stdio -S -initrd ../rootfs/initramfs.cpio

clean:
	rm -f *.o \
		bootloader bootloader.elf \
		kernel kernel.elf
