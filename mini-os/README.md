# Demo commands
1. `make`
2. `make run`
3. run `screen /dev/pts/x` at another terminal
4. type `cont` at the first terminal (after "(qemu)")
5. write command in the stage-1 bootloader
6. type `load` to load kernel in
7. type `sudo python3 tools/send_kernel.py /dev/pts/3 build/kernel.bin` on another terminal
8. write command on in kernel

# 前置設定
```
qemu-system-riscv64 \
    -M virt \
    -m 128M \
    -kernel bootloader \
    -display none \
    -serial pty
```
QEMU 建立一台虛擬的 RISCV 電腦(模擬硬體環境) with 128MB RAM，並將 compile 好的 bootloader image 載入 (booloader 包含所需的 assembly & `.c` file)

預設韌體 OpenSBI 會開始從 `0x80000000` 以 Machine mode (M-mode) 執行，初始化韌體環境，並提供出 OpenSBI 服務。
初始化完成之後，進入 Supervisor mode (S-mode)，開始執行 `0x80200000` 之後的程式

:::info
透過 `linker.ld` 讓 QEMU 得知要把程式放到記憶體的哪個位址
:::

# Workflow
## Bootloader
1. 執行 `start.S`，初始化程式環境 (例如: 清空 `.bss`、初始化 stack pointer)

2. 跳轉到 `main.c`，執行 `shell()` (`shell()` 包含多個功能)，當輸入 `load`，會執行 `load_kernel()`，等待host computer 透過 UART 傳送 kernel image 給開發板 (此載入的 kernel 會被放在 `0x82000000`)

    從 `0x82000000` 開始執行新的 kernel code
    :::info
    透過 pty，讓 host 能夠傳送資料到 UART 接收，提供資料給開發板 (而不需接一條實體線到開發板)
    :::

3. 經過 `start.S` 執行後，跳轉到 `main.c` 開始執行 `start_kernel()`，並執行 `shell()`

![啟動記憶體配置圖](https://hackmd.io/_uploads/B10NrWz3zx.png)
