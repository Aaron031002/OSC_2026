.section ".text.boot"
.global _start
_start:
    // TODO: Clear the .bss section
    la x5, __bss_start  // load address
    la x6, __bss_stop

    clear_bss:
        bgeu x5, x6, clear_bss_done // branch if greater than or equal to (unsigned)

        sd x0, 0(x5)    // store double
        addi x5, x5, 8

        j clear_bss

    clear_bss_done:

    // TODO: Set up the stack pointer
    la sp, _end 

    tail start_kernel   // jump and not keeping the return address
