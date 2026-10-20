#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
gcc -std=gnu11 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
  -fno-pie -no-pie -static -Wall -Wextra -Werror -Iinclude \
  src/string.c src/malloc.c src/stdio.c src/fdt.c \
  src/fdt_initrd.c src/cpio.c tests/test-initrd.c -o tests/test-initrd
./tests/test-initrd
printf 'Host initrd/CPIO/FDT tests: PASS\n'
