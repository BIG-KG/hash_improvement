#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>

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

uint32_t fnv1a_hash(hash_value_t inputKey, lst_hash_table_t *table) {
    const uint32_t FNV_PRIME = 16777619u;
    const uint32_t FNV_OFFSET = 2166136261u;
    uint32_t hash = FNV_OFFSET;
    char *str = (char *)inputKey.ptr;
    while (*str != '\0') {
        hash ^= (uint32_t)(*str++);
        hash *= FNV_PRIME;
    }
    return hash % table->tableSize;
}

