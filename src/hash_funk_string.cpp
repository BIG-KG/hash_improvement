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

uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t *table)
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

uint32_t hashing_crc32_string_asm(hash_value_t inputKey, lst_hash_table_t *table) {
    uint32_t crc = 0xFFFFFFFF;
    char *string = (char *)inputKey.ptr;
    uint32_t result;

    __asm__ __volatile__ (
        // Инициализация
        "movl %[crc],         %%eax\n\t"          // crc в eax (32 бита)
        "movq %[string],      %%rbx\n\t"       // string в rbx (64 бита)
        "leaq %[crc32_table], %%r8\n\t"        // r8 = адрес crc32_table
        "jmp 1f\n\t"

        // crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *string) & 255]
        "2:\n\t"
        "movl %%eax,        %%esi\n\t"           // esi = crc
        "shrl $24,          %%esi\n\t"           // esi = crc >> 24
        "xorl %%edx,        %%esi\n\t"           // esi = (crc >> 24) ^ *string
        "andl $0xFF,        %%esi\n\t"           // esi = индекс (0..255)
        "shll $2,           %%esi\n\t"           // esi = индекс * 4 (размер uint32_t)
        "movl (%%r8,%%rsi), %%edx\n\t"           // edx = crc32_table[индекс]
        "shll $8,           %%eax\n\t"           // eax = crc << 8
        "xorl %%edx,        %%eax\n\t"           // eax = (crc << 8) ^ crc32_table[]

        "incq %%rbx\n\t"                  // string++

        // Цикл обработки строки
        "1:\n\t"
        "movzbl (%%rbx), %%edx\n\t"       // edx = *string (нуль-расширение)
        "testb %%dl,     %%dl\n\t"            // Проверяем *string == 0
        "jnz 2b\n\t"                       // Если не конец строки, повторяем

        "movl %%eax,    %[result]\n\t"       // result = edx

        : [result] "=r" (result)          // Выходной параметр
        : [crc]    "r"  (crc),                // Входные параметры
          [string] "r"  (string),
          [crc32_table] "m" (crc32_table)
        : "eax", "ebx", "edx", "esi", "r8", "cc", "memory" // Используемые регистры
    );

    return result % table->tableSize;
}


int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}



