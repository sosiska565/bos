; --- _kernelASM.asm ---

bits 16         ; Начинаем в 16-битном режиме
org 0x7E00      ; Нас загрузили по этому адресу

start:
    cli             ; 1. Отключаем прерывания

    lgdt [gdt_descriptor] ; 2. Загружаем GDT

    ; 3. Включаем защищенный режим
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; 4. Дальний переход в 32-битный код
    jmp CODE_SEG:init_pm

; --- Таблица GDT ---
gdt_start:
    ; Нулевой дескриптор
    dd 0x0
    dd 0x0

    ; Дескриптор кода. base=0, limit=0xFFFFF, флаги: 32-bit, 4KB гранулярность, исполняемый
gdt_code:
    dw 0xFFFF       ; Limit (0-15)
    dw 0x0000       ; Base (0-15)
    db 0x00         ; Base (16-23)
    db 0b10011010   ; Флаги доступа: Present, Ring 0, Code, Executable/Readable
    db 0b11001111   ; Флаги гранулярности: Granularity (4KB), 32-bit, Limit (16-19)
    db 0x00         ; Base (24-31)

    ; Дескриптор данных. base=0, limit=0xFFFFF, флаги: 32-bit, 4KB гранулярность, R/W
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0b10010010   ; Флаги доступа: Present, Ring 0, Data, Readable/Writable
    db 0b11001111
    db 0x00

gdt_end:

; Дескриптор для команды LGDT
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Размер GDT
    dd gdt_start               ; Адрес начала GDT

; Селекторы для наших сегментов (смещение от начала GDT)
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


; --- 32-битный код ---
bits 32
init_pm:
    ; 5. Настраиваем сегменты данных
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 6. Настраиваем стек
    mov esp, 0x90000 ; Устанавливаем стек в безопасное место (например, 90000h)

    ; 7. Передаем управление C-коду
    ; Предполагаем, что у нас есть C-функция main()
    extern _main
    call _main

    ; Если main вернет управление, зависаем
    hlt