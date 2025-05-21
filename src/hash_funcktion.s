section .text


global hashing_crc32_string_asm

; RDI — строка (выровненная по 16 байт)
; RSI — ptr на table.size
; RAX — вернёт хеш (crc32 % size)

hashing_crc32_string_asm:

    push rbx

    xor     eax, eax
    mov     ebx, 0xFFFFFFFF
    mov     rdx, rdi          ; rdx = ptr

.loop64:
    mov     rax, [rdx]

    test   al, al
    jz     .tail_loop

    crc32   rbx, rax
    add     rdx, 8
    jmp     .loop64

.tail_loop:
    movzx   ecx, byte [rdx]
    test    cl, cl
    jz      .done
    crc32   ebx, cl
    inc     rdx
    jmp     .tail_loop

.done:
    ; делим на table.size
    mov     ecx, dword [rsi]
    xor     edx, edx
    mov     eax, ebx
    xor     eax, -1
    div     ecx
    mov     eax, edx

    pop rbx 
    ret

    
section .data