global perform_task_switch

perform_task_switch:
    pusha
    mov eax, [esp+36]
    mov cr3, eax
    popa
    mov ebp, [esp+8]
    mov esp, [esp+12]
    mov eax, [esp+4]
    jmp eax
