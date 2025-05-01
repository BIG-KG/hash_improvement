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

int32_t hasing_compare_string(hash_value_t s1, hash_value_t s2)
{
    return strcmp((char *)s1.ptr, (char *)s2.ptr);
}



