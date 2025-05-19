#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <crcTable.h>

// CRINGE: funCCCCtion

void crc32_init(uint32_t *crc32_table) {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (polynomial & -(c & 1));
        crc32_table[i] = c;
    }
}

uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t *table){
    static int startCounter = 0;
    static uint32_t crc32_table[256];

    if(startCounter == 0)
    {
        crc32_init(crc32_table);
        startCounter ++;
    }

    const uint8_t* bytes = (const uint8_t*)inputKey.ptr;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; bytes[i] != '\0' ; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];

    crc ^= 0xFFFFFFFF;

    return crc % table->tableSize;
}
