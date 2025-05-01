#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <string_hash_table.h>
#include <SDL2/SDL.h>


uint32_t hashing_length_string(hash_value_t inputKey, lst_hash_table_t *table)
{
    return strlen((char*)inputKey.ptr) % table->tableSize;
}

uint32_t hashing_sum_string(hash_value_t inputKey, lst_hash_table_t *table)
{
    char *key = (char *)inputKey.ptr;
    int32_t  i = 0;
    uint32_t hash = 0;
    while (key[i] != '\0')
    {
        hash += key[i];
    }

    return hash % table->tableSize;
}

uint32_t hashing_polynomial_string(hash_value_t inputKey, lst_hash_table_t *table)
{
    char *key = (char *)inputKey.ptr;
    int32_t  i = 0;
    uint32_t hash = 0;
    uint32_t pow = 1;

    while (key[i] != '\0')
    {
        hash += key[i] * pow;
        pow *= table->hashingConst1;
    }

    return hash % table->tableSize;    
}

uint32_t hashing_crc32_string1(hash_value_t inputKey, lst_hash_table_t *table)
{
    unsigned int crc = 0xFFFFFFFF;
    char *string = (char *)inputKey.ptr;

    while (*string != 0)
        {
            crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *string) & 255];
            string++;
            //len--;
        }

    return crc % table->tableSize;
}

uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t *table) {
    uint32_t crc = 0xFFFFFFFF;
    char *string = (char *)inputKey.ptr;
    uint32_t result;

    __asm__ __volatile__ (
        // Инициализация
        ".intel_syntax noprefix\n\t"
        "mov eax, %[crc]\n\t"          // crc в eax
        "mov ebx, %[string]\n\t"       // string в rbx
        "lea r8, %[crc32_table]\n\t"      // r8 = адрес crc32_table
        "jmp 2f\n\t"

        "1:\n\t"
        // crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *string) & 255]
        "mov esi, eax\n\t"                // esi = crc
        "shr esi, 24\n\t"                 // esi = crc >> 24
        "xor esi, edx\n\t"                // esi = (crc >> 24) ^ *string
        "and esi, 0xFF\n\t"               // esi = индекс (0..255)
        "shl esi, 2\n\t"                  // esi = индекс * 4 (размер uint32_t)
        //^
        //|calculate current session ((crc >> 24) ^ *string) & 255


        "mov edx, [r8 + rsi]\n\t"         // edx = crc32_table[индекс]
        "shl eax, 8\n\t"                  // eax = crc << 8
        "xor eax, edx\n\t"                // eax = (crc << 8) ^ crc32_table[...]

        "2:\n\t"
        "mov  edx, byte ptr [ebx]\n\t"
        "test dl, dl\n\t"                // Проверяем *string == 0
        "jne 1b\n\t"             // Если конец строки, выходим

        "mov %[result], eax\n\t"
        
        ".att_syntax prefix\n\t"
        : [result] "=r" (result)          // Выходной параметр
        : [crc] "r" (crc),                // Входные параметры
          [string] "r" (string),
          [crc32_table] "m" (crc32_table)
        : "eax", "ebx", "edx", "esi", "r8", "cc", "memory" // Используемые регистры
    );

    return result % table->tableSize;
}

int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}



