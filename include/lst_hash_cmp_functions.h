#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <stdint.h>
uint32_t hashing_length_string(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_sum_string(hash_value_t inputKey, lst_hash_table_t *table);
uint32_t hashing_polynomial_string(hash_value_t inputKey, lst_hash_table_t *table);
extern "C" uint32_t fnv1a_hash_asm(hash_value_t inputKey, lst_hash_table_t* table);
uint32_t fnv1a_hash(hash_value_t inputKey, lst_hash_table_t* table);


int32_t hasing_compare_string(hash_value_t firstKey, hash_value_t secondKey);

#endif //HASH_FUNCTIONS_H