#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
gcc -std=gnu11 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pie -no-pie -static -Wall -Wextra -Werror -Iinclude src/string.c src/malloc.c src/stdio.c tests/test.c -o tests/test
./tests/test
printf 'Host library tests: PASS\n'
