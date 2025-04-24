#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <error_hash.h>
#include <hash_link.h>
#include <settings.h>
#include <common.h>

error_t realloc_list_table(lst_hash_table_t *table);

lst_hash_node_t *find_list_table(hash_value_t targetValue, lst_hash_table_t *hashTable)
{
    uint32_t index = hashTable->hashfunction(targetValue, hashTable) % hashTable->tableSize;
    lst_hash_node_t *head = hashTable->table[index];

    if (head == NULL)
    {
        return head;
    }

    while (!hashTable->cmpfunction(head->value, targetValue) && head->next != NULL)
    {
        head = head->next;
    }

    if (!hashTable->cmpfunction(head->value, targetValue))
    {
        return NULL;
    }

    return head;
}

lst_hash_node_t *find_with_prev_list_table(hash_value_t targetValue, lst_hash_table_t *hashTable, lst_hash_node_t **previousNode)
{
    uint32_t index = hashTable->hashfunction(targetValue, hashTable) % hashTable->tableSize;
    lst_hash_node_t *head = hashTable->table[index];
    lst_hash_node_t *prevNodeLocal = NULL;

    if (head == NULL)
    {
        return head;
    }

    while (head->next != NULL && hashTable->cmpfunction(head->value, targetValue))
    {
        prevNodeLocal = head;
        head = head->next;
    }

    if (hashTable->cmpfunction(head->value, targetValue))
    {
        *previousNode = NULL;
        return NULL;
    }

    *previousNode = prevNodeLocal;

    return head;
}

lst_hash_node_t *give_next_node_pointer(lst_hash_table_t *table)
{
    if(table->deleteList != NULL)
    {
        lst_hash_node_t *free = table->deleteList;
        table->deleteList = table->deleteList->next;
        return free;
    }

    if(table->numberOfElements >= table->allocatingSize)
    {
        if(realloc_list_table(table) != ERR_SUCCESS) return NULL;
    }

    return table->allocatedSegment + table->numberOfElements++;
}

error_t realloc_list_table(lst_hash_table_t *table)
{
    lst_hash_node_t *oldNode = table->allocatedSegment;
    uint32_t newSize = next_prime (table->allocatingSize * 2);
    table->allocatedSegment = (lst_hash_node_t *) realloc(table->allocatedSegment, 
                                                             newSize * sizeof(lst_hash_node_t));
    if(table->allocatedSegment == NULL)
    {
        table->allocatedSegment = oldNode;
        return ERR_NO_MEMORY;
    }

    for (lst_hash_node_t *i = table->allocatedSegment; i < table->allocatedSegment + table->allocatingSize; i++)
    {
        if(i->next != NULL) i->next += (table->allocatedSegment - oldNode) / sizeof(lst_hash_node_t);
    }
    
    for (uint32_t i = 0; i < table->tableSize; i++)
    {
        if(table->table[i] != NULL) table->table[i] += (table->allocatedSegment - oldNode) / sizeof(lst_hash_node_t *);
    }
    
    table->allocatingSize = newSize; 

    return ERR_SUCCESS;
}

error_t add_to_list_table(lst_hash_table_t *table, hash_value_t value)
{
    uint32_t index = table->hashfunction(value, table) % table->tableSize;
    lst_hash_node_t *previousNode = NULL;

    if(find_list_table(value, table) != NULL) return ERR_SUCCESS;
    
    lst_hash_node_t *newNode = give_next_node_pointer(table);
    newNode->value = value;
    newNode->next = table->table[index];
    newNode->isActive = true;
    table->table[index] = newNode;
    table->listCounter[index]++;

    if(table->rehashing) check_coefficient_list_table(table);

    return ERR_SUCCESS;
}

error_t delete_from_list_table(lst_hash_table_t *table, hash_value_t value)
{
    lst_hash_node_t *previousNode = NULL, *targetNode = NULL;
    targetNode = find_with_prev_list_table(value, table, &previousNode);

    if (targetNode == NULL)
    {
        return ERR_SUCCESS;
    }

    targetNode->next = table->deleteList;
    table->deleteList = targetNode;
    targetNode->isActive = false;
    table->numberOfElements--;

    table->listCounter[table->hashfunction(value, table)]--;

    if (previousNode == NULL)
    {
        table->table[table->hashfunction(value, table) % table->tableSize] = targetNode->next;
        return ERR_SUCCESS;
    }

    

    previousNode->next = targetNode->next;
    return ERR_SUCCESS;   
}

error_t check_coefficient_list_table(lst_hash_table_t *table)
{
    if ((float)table->numberOfElements / (float)table->tableSize >= MAX_LOAD_FACTOR)
    {
        error_t error = ERR_SUCCESS;
        if(error = reinit_list_table(table, table->tableSize * 2)) return error;

        table->numberOfElements = 0;

        for (size_t i = 0; i < table->allocatingSize; i++)
        {
            if(table->allocatedSegment[i].isActive) add_to_list_table(table, table->allocatedSegment[i].value);
        }
    }

    return ERR_SUCCESS;
}

error_t reinit_list_table(lst_hash_table_t *table, uint32_t size)
{
    if(table == NULL) return ERR_NULL_POINTER;
    
    if(size == 0 )
    {
        table->tableSize = 0;
        if (table->table != NULL)
        {
            free(table->table);
            table->table = NULL;
        }        
        if (table->listCounter != NULL)
        {
            free(table->listCounter);
            table->listCounter = NULL;
        }
        return ERR_SUCCESS;
    }       


    lst_hash_node_t **newTable = NULL;
    if(table->table != NULL) newTable = (lst_hash_node_t **)realloc(table->table, size * sizeof(lst_hash_node_t *));
    else                     newTable = (lst_hash_node_t **)calloc (size, sizeof(lst_hash_node_t *));
    if(newTable == NULL) return ERR_NO_MEMORY;
    table->table = newTable;

    uint32_t *newCounter = NULL;
    newCounter = (uint32_t *)calloc(size, sizeof(uint32_t));
    if(newCounter == NULL) return ERR_NO_MEMORY;
    free(table->listCounter);
    table->listCounter = newCounter;

    table->hashingConst1 = next_prime(rand() % 1024);
    table->hashingConst2 = next_prime(rand() % 1024);

    table->tableSize = size;
    return ERR_SUCCESS;
}
