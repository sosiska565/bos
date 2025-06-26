bits 32
section .text
    align 4
    dd 0x1BADB002
    dd 0x00
    dd - (0x1BADB002 + 0x00)
global start
extern _OSmain

start:
    cli
    mov esp, stack_space
    push eax
    push ebx
    call _OSmain
    hlt

section .bss
    resb 16384
stack_space: