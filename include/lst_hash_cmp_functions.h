#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <stdint.h>
extern "C" uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t* table);

extern "C" int32_t hasing_compare_string(hash_value_t firstKey, hash_value_t secondKey);
extern "C" int32_t hasing_compare_string_simd(hash_value_t s1Struct, hash_value_t s2Struct);

#endif //HASH_FUNCTIONS_H