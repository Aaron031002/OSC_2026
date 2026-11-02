/*
 *  OpenSBI load bootloader to 0x80200000
 *  linker expect bootloader to exec. at 0x86000000
*/
.section ".text.boot"
.global _start                  /* linker think here is 0x86000000 */
_start:
    /* s0 ~ s7: callee-saved reg. */
    mv   s0, a0                 /* s0 = hartid */
    mv   s1, a1                 /* s1 = DTB 位址 */

    lla   s2, _start             /* s2 = 現在在哪（來源）_start represent the address of _start(not its value) */
    ld   s3, link_start         /* s3 = 應該在哪（目的地） */
    beq  s2, s3, relocated      /* 已經在目的地就不用搬 */

    /* (1) create a temp C environment at current location, since reloc_check() will be exec. later */
    lla   t0, __bss_start
    lla   t1, __bss_stop

    /* clear .bss */
1:  bgeu t0, t1, 2f     /* if t0 >= t1, jump to the nearest forward 2: label */
    sd   zero, 0(t0)    /* set the next 8 bytes to zero starting from 0(t0) */
    addi t0, t0, 8
    j    1b             /* b: backward */
2:  lla   sp, _end       /* set stack pointer to _end */

    /* (2) reloc_check(fdt, dst)：目的地不安全就停下 */
    mv   a0, s1     /* the first argument for reloc_check() */
    mv   a1, s3     /* the second argument for reloc_check() */
    call reloc_check
    bnez a0, halt   /* a0 contains the return value */

    /* (3) 複製 [_start, __reloc_image_end) 到目的地 */
    mv   t0, s2                 /* 來源 (0x80200000)*/
    lla   t1, __reloc_image_end  /* 來源結尾 (0x80200000 + .text + .rodata + .data)*/
    mv   t2, s3                 /* 目的地 (0x86000000) */
3:  ld   t3, 0(t0)
    sd   t3, 0(t2)      /* move contents of old location(0x80200000) to the new location(0x86000000) */
    addi t0, t0, 8
    addi t2, t2, 8
    bltu t0, t1, 3b

    fence.i                     /* 讓 I-cache 看到剛寫進去的指令 */
    ld   t0, link_relocated     /* jump to 新那份 relocated Label 的絕對位址 */
    jr   t0

relocated:
    /* (4) 已在新位置：重新清 .bss、設 stack */
    lla   t0, __bss_start
    lla   t1, __bss_stop
4:  bgeu t0, t1, 5f
    sd   zero, 0(t0)
    addi t0, t0, 8
    j    4b
5:  lla   sp, _end

    mv   a0, s0                 /* start_kernel(hartid, fdt) */
    mv   a1, s1
    tail start_kernel

halt:
    wfi
    j    halt

    .balign 8
link_start:     .dword _start       /* = RELOC_ADDR, _start represent the value of _start(not its address) */
link_relocated: .dword relocated