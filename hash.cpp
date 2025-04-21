#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define TABLE_SIZE 2000003
const int C1_COEFF = 1;

enum error_t
{
    SUCCESS_ERR,
    NO_MEMORY_ERR,
    NOT_FOUND_ERR,
    NULL_POINTER_ERR,
};

enum command_type
{
    ADD,
    DELETE,
    TEST
};

typedef char *  hashingElement;

struct hash_node_t
{
    hashingElement value;
    hash_node_t   *next;
};

#define START_ALLOCATING_SIZE 100
#define START_AMOUNT_ALLOCATED_SEGMENTS 100

struct hash_table_t
{
    uint32_t size             = 0;
    uint32_t numberOfElements = 0;
    hash_node_t **table       = NULL;
    int32_t allocatingSize    = START_ALLOCATING_SIZE;
    int32_t usedSize          = 0;
    hash_node_t *currentAllocatedSegment  = (hash_node_t  *)calloc(allocatingSize,       sizeof(hash_node_t  ));
};

unsigned long djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
} 

hash_node_t *find_last(hash_node_t *head)
{
    while (head->next != NULL)
    {
        head = head->next;
    }

    return head;  
}

//!! not check input arguments to 
int hash_cmp_strings(hashingElement v1, hashingElement v2)
{
    return (  strcmp((char *)v1, (char *)v1)  );     
}

hash_node_t *find(hashingElement targetValue, hash_table_t *hashTable)
{
    uint32_t index = djb2(targetValue) % hashTable->size;
    hash_node_t *head = hashTable->table[index];

    if (head == NULL)
    {
        return head;
    }

    while ( hash_cmp_strings(head->value, targetValue) && head->next != NULL)
    {
        head = head->next;
    }

    if (head->value != targetValue)
    {
        return NULL;
    }

    return head;
}

hash_node_t *find_with_prev(hashingElement targetValue, hash_table_t *hashTable, hash_node_t **previousNode)
{
    uint32_t index = djb2(targetValue) % hashTable->size;
    hash_node_t *head = hashTable->table[index];
    hash_node_t *prevNodeLocal = NULL;

    if (head == NULL)
    {
        return head;
    }

    while ( hash_cmp_strings(head->value, targetValue) && head->next != NULL)
    {
        prevNodeLocal = head;
        head = head->next;
    }

    if (head->value != targetValue)
    {
        *previousNode = NULL;
        return NULL;
    }

    *previousNode = prevNodeLocal;

    return head;
}

hash_node_t *new_node(hash_table_t *table)
{
    if(table->usedSize >= table->allocatingSize)
    {
        if(table->amountAllocatedSegments >= table->maxAmountAllocatedSegments)
        {
            hash_node_t **newAllocatedMemorySegments = (hash_node_t **)realloc(table->allocatedMemorySegments, sizeof(hash_node_t *) * table->maxAmountAllocatedSegments * 2);
            if (newAllocatedMemorySegments == NULL) return NULL;
            table->allocatedMemorySegments = newAllocatedMemorySegments;
            table->maxAmountAllocatedSegments *= 2;
        }

        hash_node_t *newAllocatedSegment = (hash_node_t *)calloc(table->allocatingSize * 2, sizeof(hash_node_t));
        if (newAllocatedSegment == NULL) return NULL;
        table->currentAllocatedSegment = newAllocatedSegment;
        table->allocatedMemorySegments[table->amountAllocatedSegments++] = newAllocatedSegment;

        table->allocatingSize *= 2;
        table->usedSize = 0;
    }

    return table->currentAllocatedSegment + table->usedSize++;
}

error_t add_to_table(hash_table_t *table, char *value)
{
    int32_t index = djb2(value) % table->size;
    hash_node_t *previousNode = NULL;

    if(find(value, table) != NULL) return SUCCESS_ERR;
    
    hash_node_t *newNode = new_node(table);
    newNode->value = value;
    newNode->next = table->table[index];
    table->table[index] = newNode;

    return SUCCESS_ERR;
}

error_t delete_from_table(hash_table_t *table, char *value)
{
    hash_node_t *previousNode = NULL, *targetNode = NULL;
    targetNode = find_with_prev(value, table, &previousNode);

    if (targetNode == NULL)
    {
        return SUCCESS_ERR;
    }

    if (previousNode == NULL)
    {
        table->table[djb2(value) % table->size] = targetNode->next;
        return SUCCESS_ERR;
    }

    previousNode->next = targetNode->next;
    return SUCCESS_ERR;   
}

error_t check_coefficient_table(hash_table_t *table)
{
    if (table->size)
    {
        /* code */
    }
    
}

error_t init_reinit_table(hash_table_t *table, uint32_t size)
{
    if(table == NULL) return NULL_POINTER_ERR;
    
    if(size == 0 )
    {
        table->size = 0;
        if (table->table != NULL)
        {
            free(table->table);
            table->table = NULL;
        }        
        return SUCCESS_ERR;
    }       


    hash_node_t **newTable = NULL;
    if(table->table != NULL) newTable = (hash_node_t **)realloc(table->table, size * sizeof(hash_node_t *));
    else                     newTable = (hash_node_t **)calloc (size, sizeof(hash_node_t *));

    if(newTable == NULL) return NO_MEMORY_ERR;

    table->table = newTable;
    return SUCCESS_ERR;
}