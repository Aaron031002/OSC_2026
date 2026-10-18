RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy	# elf -> objcopy -> raw binary
CFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany -ffreestanding -nostdlib -g -Wall
QEMU ?= qemu-system-riscv64

# build depends on these files
build: clean bootloader	

bootloader:
	$(CC) $(CFLAGS) -c start.S -o boot_start.o
	$(CC) $(CFLAGS) -c uart.c -o boot_uart.o
	$(CC) $(CFLAGS) -c main.c -o boot_main.o

	$(LD) -T link.ld \
		-o bootloader.elf \
		boot_start.o boot_uart.o boot_main.o

	# make elf to raw binary
	$(OBJCOPY) -O binary \
		bootloader.elf \
		bootloader

run: bootloader
	$(QEMU) -M virt -m 64M -kernel bootloader -display none -serial pty

clean:
	rm -f *.o \
		bootloader bootloader.elf \
		kernel kernel.elf
