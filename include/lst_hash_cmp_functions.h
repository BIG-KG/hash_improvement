#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <stdint.h>
#include <hash_link.h>

uint32_t hashing_remainder_division_float(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_binary_float            (hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_mantissa_float          (hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_exponent_float          (hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_combined_float          (hash_value_t inputKey, lst_hash_table_t *table);

uint32_t hashing_remainder_division_int  (hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_binary_int              (hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_Thomas_Cormen_int       (hash_value_t inputKey, lst_hash_table_t *table);

uint32_t hashing_length_string(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_sum_string(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_polynomial_string(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_crc32_string(hash_value_t inputKey, lst_hash_table_t *table);

int32_t hasing_compare_float(hash_value_t firstKey, hash_value_t secondKey);
int32_t hasing_compare_int(hash_value_t firstKey, hash_value_t secondKey);
int32_t hasing_compare_string(hash_value_t firstKey, hash_value_t secondKey);

#endif //HASH_FUNCTIONS_H