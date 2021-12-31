#include "mm.h"
#include "sysregs.h"
#include "mmu.h"

.section ".text.boot"

.globl _start
_start:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    b proc_hang

master:
    ldr x0, =SCTLR_VALUE_MMU_DISABLED
    msr sctlr_el1, x0

    ldr x0, =HCR_VALUE
    msr hcr_el2, x0

    ldr x0, =SCR_VALUE
    msr scr_el3, x0

    ldr x0, =SPSR_VALUE
    msr spsr_el3, x0

#if INIT_MMU == 1
    ldr x0, =CPACR_EL1_VAL
    msr CPACR_EL1, x0

    ldr x0, =TCR_EL1_VAL
    msr TCR_EL1, x0

    ldr x0, =MAIR_VALUE
    msr MAIR_EL1, x0
#endif

    adr x0, el1_entry
    msr elr_el3, x0

    eret

el1_entry:
    mov sp, #LOW_MEMORY

    adr x0, bss_begin
    adr x1, bss_end
    sub x1, x1, x0
    bl memzero

#if INIT_MMU == 1
    bl init_mmu

    adrp x0, id_pgd
    msr ttbr0_el1, x0

    mrs x0, sctlr_el1
    mov x1, #SCTLR_MMU_ENABLED
    orr x0, x0, x1
    msr sctlr_el1, x0
#endif

    bl kernel_main
    b  proc_hang

proc_hang:
    wfe
    b proc_hang

.globl id_pgd_addr
id_pgd_addr:
    adrp x0, id_pgd
    ret