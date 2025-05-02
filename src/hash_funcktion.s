section .text

global printf__my

section .text
global fnv1a_hash_asm

;INPUT  : RDI - start of string, RSI - ptr to table structure
;OUTPUT : RAX - hash
;DST    : RAX, RCX, RDX


fnv1a_hash_asm:
    ; Сохраняем регистры, если нужно (по System V ABI, callee-saved регистры не трогаем)
    ; Инициализация
    mov eax, [rsi+4]         ; eax = FNV_OFFSET (2166136261)
                             ; rdi = inputKey.ptr (указатель на строку)
    mov ecx, [rsi+8]         ; ecx = FNV_PRIME (предполагаем tableSize по смещению 0)

    jmp .check

.loop:
    ; Загружаем очередной байт строки
    xor eax, edx                ; hash ^= *str
    mul ecx                     ; hash *= FNV_PRIME(ecx) (eax *= edx, результат в eax)
    inc rdi                     ; str++
.check:
    ; Проверка конца строки
    movzx edx, byte [rdi]       ; edx = *str (zero-extend byte to 32-bit)
    cmp byte edx, 0           ; *str == '\0'?
    jne .loop                   ; Если не конец, продолжить цикл

.done:
    ; Вычисляем hash % tableSize
    mov rcx, [rsi]              ; rcx = table->tableSize (предполагаем tableSize по смещению 0)
    xor edx, edx                ; Обнуляем edx для div
    div rcx                     ; eax / rcx, остаток в edx
    mov eax, edx                ; eax = hash % tableSize

    ; Восстанавливаем регистры и возврат
    ret