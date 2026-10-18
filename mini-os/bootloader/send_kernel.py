import struct
import sys

BOOT_MAGIC = 0x544F4F42

if len(sys.argv) != 3:
    print("Usage:")
    print("python3 send_kernel.py <tty> <kernel>")  # tty: /dev/pts/3, kernel: kernel (raw binary)
    sys.exit(1) # end the program

tty_path = sys.argv[1]
kernel_path = sys.argv[2]

# prepare kernel image content
with open(kernel_path, "rb") as f:  # binary read mode
    kernel_data = f.read()

# prepare header content
header = struct.pack("<II", BOOT_MAGIC, len(kernel_data))

# write /dev/pts/3 will help user send data to UART
with open(tty_path, "wb", buffering=0) as tty:  # don't write data to buffer
    tty.write(header)
    tty.write(kernel_data)

print("Kernel sent")
print("Size: ", len(kernel_data))