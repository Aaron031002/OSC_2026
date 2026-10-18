RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy

CFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany \
         -ffreestanding -nostdlib -g -Wall

TARGET = kernel

all: $(TARGET)

$(TARGET): start.S main.c link.ld
	$(CC) $(CFLAGS) -c start.S -o start.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(LD) -T link.ld -o kernel.elf start.o main.o
	$(OBJCOPY) -O binary kernel.elf $(TARGET)

clean:
	rm -f *.o kernel kernel.elf

.PHONY: all clean
