RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy

CFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany \
         -ffreestanding -nostdlib -fno-builtin \
         -fno-stack-protector -g -Wall \
         -I../include

TARGET = kernel

all: $(TARGET)

$(TARGET): start.S main.c link.ld \
           ../src/string.c ../src/malloc.c \
           ../src/fdt.c ../src/fdt_uart.c \
		   ../src/cpio.c ../src/fdt_initrd.c \
		   ../src/stdio.c
	$(CC) $(CFLAGS) -c start.S -o start.o
	$(CC) $(CFLAGS) -c main.c -o main.o

	$(CC) $(CFLAGS) -c ../src/string.c -o string.o
	$(CC) $(CFLAGS) -c ../src/malloc.c -o malloc.o
	$(CC) $(CFLAGS) -c ../src/fdt.c -o fdt.o
	$(CC) $(CFLAGS) -c ../src/fdt_uart.c -o fdt_uart.o
	$(CC) $(CFLAGS) -c ../src/fdt_initrd.c -o fdt_initrd.o
	$(CC) $(CFLAGS) -c ../src/cpio.c -o cpio.o
	$(CC) $(CFLAGS) -c ../src/stdio.c -o stdio.o

	$(CC) $(CFLAGS) -T link.ld -o kernel.elf \
    	start.o main.o string.o malloc.o fdt.o fdt_uart.o fdt_initrd.o cpio.o stdio.o \
		-lgcc

	$(OBJCOPY) -O binary kernel.elf $(TARGET)

clean:
	rm -f *.o kernel kernel.elf

.PHONY: all clean
