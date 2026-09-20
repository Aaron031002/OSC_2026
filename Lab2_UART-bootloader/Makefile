RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy	# elf -> objcopy -> raw binary
CFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany -ffreestanding -nostdlib -g -Wall
QEMU ?= qemu-system-riscv64

# build depends on these files
build: clean bootloader kernel	

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

kernel:
	$(CC) $(CFLAGS) -c start.S -o kernel_start.o
	$(CC) $(CFLAGS) -c uart.c -o kernel_uart.o
	$(CC) $(CFLAGS) -c kernel.c -o kernel_main.o

	$(LD) -T kernel_link.ld \
		-o kernel.elf \
		kernel_start.o kernel_uart.o kernel_main.o

	$(OBJCOPY) -O binary \
		kernel.elf \
		kernel

run: bootloader
	$(QEMU) -M virt -m 128M -kernel bootloader -display none -serial pty

clean:
	rm -f *.o \
		bootloader bootloader.elf \
		kernel kernel.elf
