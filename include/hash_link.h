#ifndef HASH_TYPES_H
#define HASH_TYPES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <settings.h>

union hash_value_t
{
    int32_t int32 = 0;
    float   float32;
    void    *ptr;

};

struct lst_hash_node_t
{
    lst_hash_node_t *next;
    hash_value_t     value;
    uint32_t         isActive = false;
    lst_hash_node_t *prev;
};

struct lst_hash_table_t
{
    uint32_t         tableSize         = 0;
    uint32_t         hashingConst1     = 0;
    uint32_t         hashingConst2     = 0;
    uint32_t         (*hashfunction)(hash_value_t, lst_hash_table_t *);
    lst_hash_node_t  **table           = NULL;
    int32_t          (*cmpFunction) (hash_value_t, hash_value_t);
    lst_hash_node_t *(*findFunction)(hash_value_t targetValue, lst_hash_table_t *hashTable);
    uint32_t         checkIfValueInTable = true;
    lst_hash_node_t  *allocatedSegment = NULL;
    uint32_t         numberOfElements  = 0;
    lst_hash_node_t  *deleteList       = NULL;
    uint32_t         allocatingSize    = START_ALLOCATING_SIZE;
    uint32_t         allUsed           = 0;
    bool             rehashing         = false;
}; 

error_t reinit_list_table           (lst_hash_table_t *table, uint32_t size);
error_t check_coefficient_list_table(lst_hash_table_t *table);
error_t delete_from_list_table      (lst_hash_table_t *table, hash_value_t value);
error_t add_to_list_table           (lst_hash_table_t *table, hash_value_t value);
lst_hash_node_t *find_list_table               (hash_value_t targetValue, lst_hash_table_t *hashTable);
lst_hash_node_t *find_list_table_nasm          (hash_value_t targetValue, lst_hash_table_t *hashTable);

error_t destroy_list_table(lst_hash_table_t *table);
error_t init_list_table(uint32_t         (*hashfunction)(hash_value_t, lst_hash_table_t *),
                        int32_t          (*cmpFunction) (hash_value_t, hash_value_t),
                        lst_hash_node_t *(*findFunction)(hash_value_t targetValue, lst_hash_table_t *hashTable),
                        bool rehashing, lst_hash_table_t *table, uint32_t size, uint32_t startNumOfNodes);
#endif