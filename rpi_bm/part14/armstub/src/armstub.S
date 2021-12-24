.globl _start
_start:
    ldr w4, kernel_entry32
    br x4

    //mrs x6, MPIDR_EL1
    //and x6, x6, #0x3
    //cbz x6, primary_cpu
    //ldr w0, kernel_entry32
    //br x0
    //bl primary_cpu

secondary_spin:
    //wfe
    //ldr x4, [x5, x6, lsl #3]
    //cbz x4, secondary_spin
    //mov x0, #0
    //b boot_kernel

.ltorg

.org 0xf0
.globl stub_magic
stub_magic:
    .word 0x5afe570b

.org 0xf4
.globl stub_version
stub_version:
    .word 0

.org 0xfc
.globl kernel_entry32
kernel_entry32:
    .word 0x0

.org 0x100

primary_cpu:
    ldr w4, kernel_entry32
    //ldr w0, dtb_ptr32

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    br x4