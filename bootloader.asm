bits 16
org 0x7C00

; Main procedure
start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg
    call print_string

    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0
    mov bx, 0x7E00
    int 0x13
    jc error

    jmp 0x0000:0x7E00

error:
    mov si, error_msg
    call print_string

print_string:
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

msg:
    db "Booting...", 0x0D, 0x0A, 0
error_msg:
    db "Load error", 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55