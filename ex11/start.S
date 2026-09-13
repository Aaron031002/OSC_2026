.section ".text.boot"
.global _start
_start:
    // TODO: Clear the .bss section
    la x5, __bss_start
    la x6, __bss_stop

    clear_bss:
        bgeu x5, x6, clear_bss_done

        sd x0, 0(x5)
        addi x5, x5, 8

        j clear_bss

    clear_bss_done:

    // TODO: Set up the stack pointer
    la sp, _end 

    tail start_kernel
