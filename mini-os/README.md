# Demo commands
1. `make`
2. `make run`
3. run `screen /dev/pts/x` at another terminal
4. type `cont` at the first terminal (after "(qemu)")
5. write command in the stage-1 bootloader
6. type `load` to load kernel in
7. type `sudo python3 tools/send_kernel.py /dev/pts/3 build/kernel.bin` on another terminal
8. write command on in kernel