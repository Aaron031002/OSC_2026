#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build

gcc -std=gnu11 -ffreestanding -nostdlib -fno-builtin \
    -fno-stack-protector -fno-pie -no-pie -static -Wall -Wextra -Werror \
    -Iinclude src/string.c src/malloc.c src/fdt.c tests/test-fdt.c \
    -o build/test-fdt
./build/test-fdt
printf 'Host FDT parser tests: PASS\n'
