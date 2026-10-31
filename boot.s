
.section .multiboot
.align 4

.long 0x1BADB002
.long 0x00000003
.long -(0x1BADB002 + 0x00000003)

.section .text
.global _start
.type _start, @function

_start:
    cli
    cld

    mov $stack_top, %esp
    xor %ebp, %ebp

    push %ebx
    push %eax

    call kernel_main

halt_loop:
    cli
    hlt
    jmp halt_loop

.section .bss
.align 16

stack_bottom:
.skip 16384
stack_top:
